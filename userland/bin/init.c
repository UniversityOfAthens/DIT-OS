// DIT-OS init: runs the kernel self-test, then the shell.
//
// The self-test covers the process model from user space, so a broken
// page-fault handler, copy-on-write fork or ELF loader shows up at boot
// instead of at the first shell command that happens to need it.

#include "syscall.h"
#include "stdio.h"
#include "sys/stat.h"

void userland_shell_main(void);

static int failures;

static void check(int ok, const char* what)
{
    if (ok)
        return;
    printf("[selftest] FAIL: %s\n", what);
    failures++;
}

/* Same, but reports the value so a failing syscall is identifiable. */
static void check_value(int ok, int value, const char* what)
{
    if (ok)
        return;
    printf("[selftest] FAIL: %s (got %d)\n", what, value);
    failures++;
}

/* brk() and mmap() hand out memory lazily, so writing to a fresh page is what
 * exercises the demand-paging path. */
static void test_memory(void)
{
    char* start = (char*)brk(0);
    check(start != 0, "brk(0) returns the break");
    if (!start)
        return;

    char* grown = (char*)brk(start + 0x2000);
    check(grown == start + 0x2000, "brk() grows");

    /* First touch of two new heap pages: both should fault in zeroed. The
     * break is the first address *past* the heap, so stay below it. */
    grown[-1] = 'A';
    grown[-0x2000] = 'B';
    check(grown[-1] == 'A' && grown[-0x2000] == 'B', "brk memory is writable");

    char* block = (char*)mmap(0x1000, PROT_READ | PROT_WRITE,
                              MAP_PRIVATE | MAP_ANONYMOUS);
    check(block > (char*)0x1000, "mmap returns a usable address");
    if (block <= (char*)0x1000)
        return;
    for (int i = 0; i < 0x1000; i++)
        block[i] = (char)(i & 0xFF);
    int sum = 0;
    for (int i = 0; i < 0x1000; i++)
        sum += (unsigned char)block[i];
    /* 0x1000 bytes hold 16 repeats of the pattern 0..255. */
    check_value(sum == 16 * (255 * 256) / 2, sum,
                "mmap memory reads back what was written");
    check(munmap(block, 0x1000) == 0, "munmap succeeds");

    /* Shrinking the break keeps the page that straddles the new value. */
    char* back = (char*)brk(start);
    check(back == start, "brk() shrinks back");
}

/* The child changes a value the parent wrote before the fork; the parent must
 * not see it, which is what copy-on-write guarantees. */
static void test_fork(void)
{
    volatile char marker = 0x12;
    char* address = (char*)&marker;

    int pid = fork();
    check_value(pid >= 0, pid, "fork succeeds");
    if (pid == 0)
    {
        marker = 0x34;
        write(1, "[selftest] child ran\n", 22);
        exit(7);
    }
    if (pid < 0)
        return;

    check(marker == 0x12, "parent memory survives the child");
    check(*address == 0x12, "parent copy of the shared page is intact");

    int status = -1;
    int reaped = waitpid(pid, &status, 0);
    check_value(reaped == pid, reaped, "waitpid returns the child pid");
    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    check_value(exit_code == 7, exit_code, "waitpid returns the exit code");

    int none = waitpid(-1, 0, 0);
    check_value(none == -ECHILD, none, "waitpid without children is -ECHILD");
}

/* execve replaces the child with another program from the filesystem. */
static void test_execve(void)
{
    char* argv[] = { (char*)"greeter", 0 };
    char* envp[] = { 0 };

    int pid = fork();
    check_value(pid >= 0, pid, "fork for execve succeeds");
    if (pid == 0)
    {
        /* Only reached when execve failed. */
        printf("[selftest] execve returned %d\n",
               execve("/greeter", argv, envp));
        exit(1);
    }
    if (pid < 0)
        return;

    int status = -1;
    int reaped = waitpid(pid, &status, 0);
    check_value(reaped == pid, reaped, "waitpid after execve");
    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    check_value(exit_code == 3, exit_code,
                "the exec'd program ran and exited with 3");
}

/* stat() and fstat() read file metadata back from ramfs. The size is printed
 * rather than compared: scripts/qemu-test.sh derives it from the seeded text,
 * so the test does not duplicate the number. */
static void test_stat(void)
{
    struct stat by_path = { 0 };
    int r = stat("/hello.txt", &by_path);
    check_value(r == 0, r, "stat /hello.txt succeeds");
    const int have_path = (r == 0);
    if (have_path)
    {
        check(S_ISREG(by_path.st_mode), "/hello.txt is a regular file");
        printf("[selftest] stat: /hello.txt %d bytes\n", (int)by_path.st_size);
    }

    struct stat root = { 0 };
    r = stat("/", &root);
    check_value(r == 0, r, "stat of the root directory succeeds");
    if (r == 0)
        check(S_ISDIR(root.st_mode), "/ is a directory");

    struct stat missing = { 0 };
    r = stat("/no-such-file", &missing);
    check_value(r == -ENOENT, r, "stat of a missing file is -ENOENT");

    int fd = open("/hello.txt", 0);
    check_value(fd >= 0, fd, "open /hello.txt for fstat");
    if (fd < 0)
        return;

    struct stat by_fd = { 0 };
    r = fstat(fd, &by_fd);
    check_value(r == 0, r, "fstat succeeds");
    /* Comparing against by_path is only meaningful if that call succeeded. */
    if (r == 0 && have_path)
        check(by_fd.st_size == by_path.st_size &&
                  by_fd.st_ino == by_path.st_ino,
              "fstat matches stat");

    r = close(fd);
    check_value(r == 0, r, "close after fstat succeeds");

    /* The slot must be gone from the table, not just the file released. */
    r = fstat(fd, &by_fd);
    check_value(r == -EBADF, r, "fstat of a closed fd is -EBADF");
}

/* kill() delivers default actions only: SIGKILL/SIGTERM terminate, ignored
 * signals do nothing, and stop signals need job control. */
static void test_kill(void)
{
    int self = getpid();

    int r = kill(self, 0);
    check_value(r == 0, r, "kill(self, 0) succeeds");

    r = kill(self, SIGCHLD);
    check_value(r == 0, r, "SIGCHLD is ignored by default");

    r = kill(self, SIGSTOP);
    check_value(r == -ENOSYS, r, "SIGSTOP is -ENOSYS (no job control)");

    /* Self-delivery happens at syscall exit, so this must run in a child. */
    int pid = fork();
    check_value(pid >= 0, pid, "fork for SIGKILL succeeds");
    if (pid == 0)
    {
        kill(getpid(), SIGKILL);
        /* Reached only if delivery failed. */
        exit(1);
    }
    if (pid < 0)
        return;

    int status = -1;
    int reaped = waitpid(pid, &status, 0);
    check_value(reaped == pid, reaped, "waitpid after SIGKILL");
    check(WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL,
          "SIGKILL terminates the child");
    printf("[selftest] kill: SIGKILL status %d\n", status);

    /* A blocked task must be woken so the pending signal is delivered. */
    pid = fork();
    check_value(pid >= 0, pid, "fork for blocked SIGTERM succeeds");
    if (pid == 0)
    {
        char c;
        read(0, &c, 1); /* blocks until the parent signals us */
        exit(1);
    }
    if (pid < 0)
        return;

    r = kill(pid, SIGTERM);
    check_value(r == 0, r, "parent can signal its child");
    status = -1;
    reaped = waitpid(pid, &status, 0);
    check_value(reaped == pid, reaped, "waitpid after SIGTERM");
    check(WIFSIGNALED(status) && WTERMSIG(status) == SIGTERM,
          "SIGTERM terminates a blocked child");
}

/* A user fault is reported through the wait status as SIGSEGV. */
static void test_fault_signal(void)
{
    int pid = fork();
    check_value(pid >= 0, pid, "fork for fault status succeeds");
    if (pid == 0)
    {
        /* Deliberate fault: it is the only way to test that a user page fault
         * is reported to the parent as SIGSEGV. */
        // cppcheck-suppress nullPointer
        *(volatile int*)0 = 1;
        exit(1);
    }
    if (pid < 0)
        return;

    int status = -1;
    int reaped = waitpid(pid, &status, 0);
    check_value(reaped == pid, reaped, "waitpid after user fault");
    check(WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV,
          "a user fault is reported as SIGSEGV");
}

/* copy_to_user() must break fork()'s COW instead of failing (M39): a global
 * struct filled before fork() is present but read-only in the child, so a
 * syscall writing it exercises the COW path. The parent's copy must stay
 * untouched, proving the child got a private page. */
static struct stat g_cow_stat;

static void test_cow_copy_to_user(void)
{
    for (unsigned int i = 0; i < sizeof(g_cow_stat); i++)
        ((unsigned char*)&g_cow_stat)[i] = 0xAA;

    int pid = fork();
    check_value(pid >= 0, pid, "fork for COW copy_to_user succeeds");
    if (pid == 0)
    {
        int fd = open("/hello.txt", 0);
        if (fd < 0)
            exit(2);
        /* Before the fix this returned -EFAULT: the COW page is not yet
         * writable, so the write=true range check rejected it. */
        int r = fstat(fd, &g_cow_stat);
        if (r != 0)
            exit(3);
        struct stat by_fd = { 0 };
        r = fstat(fd, &by_fd);
        close(fd);
        if (r != 0)
            exit(4);
        if (g_cow_stat.st_mode != by_fd.st_mode ||
            g_cow_stat.st_size != by_fd.st_size ||
            g_cow_stat.st_ino != by_fd.st_ino)
            exit(5);
        exit(0);
    }
    if (pid < 0)
        return;

    int status = -1;
    int reaped = waitpid(pid, &status, 0);
    check_value(reaped == pid, reaped, "waitpid after COW copy_to_user");
    int code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    check_value(code == 0, code, "fstat into a COW page succeeds (M39)");

    int untouched = 1;
    for (unsigned int i = 0; i < sizeof(g_cow_stat); i++)
    {
        if (((const unsigned char*)&g_cow_stat)[i] != 0xAA)
        {
            untouched = 0;
            break;
        }
    }
    check(untouched, "fork() isolates the child's COW copy");
}

/* fork() must take a reference for every page it shares, including a code
 * image page this process already made private: vmm_clone_user_directory()
 * pre-maps the image in the new space and skipped frame_ref() whenever the
 * destination PTE was present. The child then owned the only reference, so
 * its exit freed a frame the parent still mapped (M42). A neighbouring
 * global on the same page detects a COW break that zeroes the page. */
static volatile unsigned int g_cow_probe;
static volatile unsigned int g_cow_guard;

static void test_cow_frame_refcount(void)
{
    g_cow_guard = 0xC0FFEE00u;
    g_cow_probe = 0x11111111u;

    int pid = fork();
    check_value(pid >= 0, pid, "fork for COW frame refcount succeeds");
    if (pid == 0)
    {
        g_cow_probe = 0x33333333u;
        exit(0);
    }
    if (pid < 0)
        return;

    int status = -1;
    int reaped = waitpid(pid, &status, 0);
    check_value(reaped == pid, reaped, "waitpid after COW frame refcount");

    g_cow_probe = 0x44444444u;
    check_value(g_cow_probe == 0x44444444u, (int)g_cow_probe,
                "parent still writes its COW page after the child exits");
    check_value(g_cow_guard == 0xC0FFEE00u, (int)g_cow_guard,
                "COW break preserves the rest of the page (M42)");
}

/* A dead parent's children are reparented to PID 1 (init), so their zombies
 * stay reapable instead of leaking forever (M40). */
static void test_orphan_reparent(void)
{
    int parent = fork();
    check_value(parent >= 0, parent, "fork for orphan reparenting succeeds");
    if (parent == 0)
    {
        int child = fork();
        if (child == 0)
            exit(5);
        if (child < 0)
            exit(2);
        exit(0);
    }
    if (parent < 0)
        return;

    int status = -1;
    int reaped = waitpid(parent, &status, 0);
    check_value(reaped == parent, reaped,
                "waitpid reaps the exiting middle parent");
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
    {
        check(0, "the middle parent exits normally");
        return;
    }

    /* The grandchild is now init's child and must be reapable. */
    reaped = waitpid(-1, &status, 0);
    check_value(reaped > 0, reaped, "init reaps a reparented orphan");
    int code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    check_value(code == 5, code, "the reparented orphan keeps its exit code");
}

/* Security probes: every pointer and descriptor below is untrusted, so the
 * kernel must return an error instead of dereferencing it. None of these is
 * expected to fault, so they run in init itself rather than a child. */
static void test_sec_stat_bad_pointers(void)
{
    struct stat st = { 0 };

    /* Kernel memory (the start of the kernel image) as the path. */
    int r = stat((const char*)0x00100000, &st);
    check_value(r == -EFAULT, r,
                "security: stat path in kernel memory is -EFAULT");

    /* Kernel memory as the result buffer. */
    r = stat("/hello.txt", (struct stat*)0x00100000);
    check_value(r == -EFAULT, r,
                "security: stat result in kernel memory is -EFAULT");

    r = stat(0, &st);
    check_value(r == -EFAULT, r, "security: stat NULL path is -EFAULT");

    r = stat("/hello.txt", 0);
    check_value(r == -EFAULT, r, "security: stat NULL result is -EFAULT");

    /* addr + sizeof(struct stat) wraps past the end of the address space. */
    r = stat("/hello.txt", (struct stat*)0xFFFFFFF0u);
    check_value(r == -EFAULT, r, "security: stat wrapping result is -EFAULT");
}

static void test_sec_fstat_bad_fd(void)
{
    /* Mirrors FD_TABLE_SIZE in kernel/include/syscall/fd.hpp. */
    enum
    {
        TEST_FD_TABLE_SIZE = 32
    };
    struct stat st = { 0 };

    int r = fstat(-1, &st);
    check_value(r == -EBADF, r, "security: fstat(-1) is -EBADF");

    /* fd 0 is the console, which is not a VFS file yet. */
    r = fstat(0, &st);
    check_value(r == -EBADF, r, "security: fstat(0) is -EBADF");

    r = fstat(TEST_FD_TABLE_SIZE, &st);
    check_value(r == -EBADF, r, "security: fstat past the table is -EBADF");
}

/* The result struct is copied to user space, so the kernel must write every
 * byte of it: a field (or padding) left at the caller's fill pattern would
 * leak kernel stack contents (CVE-2018-11508 class). */
static void test_sec_stat_uninitialised(void)
{
    struct stat st;
    for (unsigned i = 0; i < sizeof(st); i++)
        ((unsigned char*)&st)[i] = 0xAA;

    int r = stat("/hello.txt", &st);
    check_value(r == 0, r, "security: stat into a 0xAA-filled struct");
    if (r != 0)
        return;

    check(st.st_dev == 0 && st.st_rdev == 0 && st.st_blksize == 0,
          "security: stat zero-fills fields it does not report");

    int leftover = 0;
    for (unsigned i = 0; i < sizeof(st); i++)
    {
        if (((const unsigned char*)&st)[i] == 0xAA)
            leftover++;
    }
    check_value(leftover == 0, leftover,
                "security: stat writes every byte of struct stat");
}

/* pid 0/-1 would mean a process group; neither is implemented. Signal
 * numbers are bounds-checked before use. */
static void test_sec_kill_invalid(void)
{
    int self = getpid();

    int r = kill(0, SIGKILL);
    check_value(r == -EINVAL, r, "security: kill(0) is -EINVAL");
    r = kill(-1, SIGKILL);
    check_value(r == -EINVAL, r, "security: kill(-1) is -EINVAL");
    r = kill(self, -1);
    check_value(r == -EINVAL, r, "security: kill with signal -1 is -EINVAL");
    r = kill(self, NSIG);
    check_value(r == -EINVAL, r, "security: kill with signal NSIG is -EINVAL");

    /* A pid far above any allocated one must not be confused with a task. */
    r = kill(999999, 0);
    check_value(r == -ESRCH, r, "security: kill of a missing pid is -ESRCH");
}

/* Only self and direct children may be signalled: a child must not be able
 * to kill init (PID 1). Signal 0 checks the permission without delivering a
 * fatal signal, so a regression fails the assertion instead of taking the
 * self-test down with it. */
static void test_sec_kill_parent(void)
{
    int pid = fork();
    check_value(pid >= 0, pid, "fork for kill permission probe succeeds");
    if (pid == 0)
    {
        int r = kill(1, 0);
        exit(r == -EPERM ? 0 : 1);
    }
    if (pid < 0)
        return;

    int status = -1;
    int reaped = waitpid(pid, &status, 0);
    check_value(reaped == pid, reaped, "waitpid after kill permission probe");
    int code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    check_value(code == 0, code, "security: a child cannot kill its parent");
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    test_memory();
    test_fork();
    test_execve();
    test_stat();
    test_kill();
    test_fault_signal();
    test_orphan_reparent();
    test_cow_copy_to_user();
    test_cow_frame_refcount();
    test_sec_stat_bad_pointers();
    test_sec_fstat_bad_fd();
    test_sec_stat_uninitialised();
    test_sec_kill_invalid();
    test_sec_kill_parent();

    if (failures == 0)
        printf("[selftest] ok\n");
    else
        printf("[selftest] %d failure(s)\n", failures);

    userland_shell_main();
    return 0;
}
