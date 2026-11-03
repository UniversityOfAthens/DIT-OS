// DIT-OS init: runs the kernel self-test, then the shell.
//
// The self-test covers the process model from user space, so a broken
// page-fault handler, copy-on-write fork or ELF loader shows up at boot
// instead of at the first shell command that happens to need it.

#include "syscall.h"
#include "signal.h"
#include "stdio.h"
#include "stdlib.h"
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

/* Default actions: SIGKILL/SIGTERM terminate and ignored signals do
 * nothing. init (PID 1) cannot be stopped or killed. */
static void test_kill(void)
{
    int self = getpid();

    int r = kill(self, 0);
    check_value(r == 0, r, "kill(self, 0) succeeds");

    r = kill(self, SIGCHLD);
    check_value(r == 0, r, "SIGCHLD is ignored by default");

    r = kill(self, SIGSTOP);
    check_value(r == 0, r, "SIGSTOP to init is accepted and discarded");

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

    /* fd 0 is the console: a character device with no inode. */
    r = fstat(0, &st);
    check_value(r == 0 && S_ISCHR(st.st_mode), r, "fstat(0) is the console");

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

/* Broadcast (-1) is not supported and signal numbers are bounds-checked
 * before use. pid 0 is the caller's group; signal 0 only probes it. */
static void test_sec_kill_invalid(void)
{
    int self = getpid();

    int r = kill(0, 0);
    check_value(r == 0, r, "kill(0, 0) probes the caller's group");
    r = kill(-1, SIGKILL);
    check_value(r == -EINVAL, r, "security: kill(-1) is -EINVAL");
    r = kill(self, -1);
    check_value(r == -EINVAL, r, "security: kill with signal -1 is -EINVAL");
    r = kill(self, NSIG);
    check_value(r == -EINVAL, r, "security: kill with signal NSIG is -EINVAL");

    /* A pid far above any allocated one must not be confused with a task. */
    r = kill(999999, 0);
    check_value(r == -ESRCH, r, "security: kill of a missing pid is -ESRCH");
    r = kill(-999999, 0);
    check_value(r == -ESRCH, r, "security: kill of a missing group is -ESRCH");
    /* -INT_MIN overflows; it must not alias any group. */
    r = kill((int)0x80000000u, 0);
    check_value(r == -EINVAL, r, "security: kill(INT_MIN) is -EINVAL");
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

/* ---- Signals ---------------------------------------------------------- */

static volatile int g_sig_count;
static volatile int g_sig_last;

static void count_signal(int sig)
{
    g_sig_count++;
    g_sig_last = sig;
}

static int install(int sig, sighandler_t handler, unsigned int flags)
{
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_mask = 0;
    act.sa_flags = flags;
    act.sa_restorer = 0;
    return sigaction(sig, &act, 0);
}

/* Wait for @pid and return its wait status (-1 if waitpid failed). */
static int wait_status(int pid, int options)
{
    int status = -1;
    int reaped = waitpid(pid, &status, options);
    return reaped == pid ? status : -1;
}

/* A caught signal runs its handler on the way out of the raising syscall
 * and returns to the interrupted code through sigreturn. */
static void test_signal_handler(void)
{
    g_sig_count = 0;
    int r = install(SIGUSR1, count_signal, 0);
    check_value(r == 0, r, "sigaction installs a handler");

    struct sigaction old;
    r = sigaction(SIGUSR1, 0, &old);
    check_value(r == 0, r, "sigaction queries the current action");
    check(old.sa_handler == count_signal, "sigaction reports the handler");

    /* Live values across the handler prove sigreturn restored the
     * registers, not just EIP. */
    volatile int before = 1234;
    r = raise(SIGUSR1);
    check_value(r == 0, r, "raise(SIGUSR1) succeeds");
    check_value(g_sig_count == 1, g_sig_count, "the handler ran once");
    check_value(g_sig_last == SIGUSR1, g_sig_last,
                "the handler received the signal number");
    check(before == 1234, "context survives the handler");
    printf("[selftest] signal: handler ran for %d\n", g_sig_last);

    r = install(SIGUSR1, SIG_DFL, 0);
    check_value(r == 0, r, "sigaction restores SIG_DFL");
}

/* A blocked signal stays pending and is delivered once unblocked. */
static void test_signal_mask(void)
{
    g_sig_count = 0;
    install(SIGUSR2, count_signal, 0);

    sigset_t block;
    sigemptyset(&block);
    sigaddset(&block, SIGUSR2);
    sigset_t old;
    int r = sigprocmask(SIG_BLOCK, &block, &old);
    check_value(r == 0, r, "sigprocmask blocks SIGUSR2");

    raise(SIGUSR2);
    check_value(g_sig_count == 0, g_sig_count, "a blocked signal waits");

    sigset_t pending = 0;
    r = sigpending(&pending);
    check_value(r == 0, r, "sigpending succeeds");
    check(sigismember(&pending, SIGUSR2) == 1, "sigpending reports SIGUSR2");

    /* SIGKILL and SIGSTOP cannot be blocked: they are dropped silently. */
    sigset_t all;
    sigfillset(&all);
    sigset_t during = 0;
    sigprocmask(SIG_BLOCK, &all, &during);
    sigset_t now = 0;
    sigprocmask(SIG_BLOCK, 0, &now);
    check(!sigismember(&now, SIGKILL) && !sigismember(&now, SIGSTOP),
          "SIGKILL and SIGSTOP cannot be blocked");
    sigprocmask(SIG_SETMASK, &during, 0);

    r = sigprocmask(SIG_SETMASK, &old, 0);
    check_value(r == 0, r, "sigprocmask unblocks");
    check_value(g_sig_count == 1, g_sig_count,
                "unblocking delivers the pending signal");
    install(SIGUSR2, SIG_DFL, 0);
}

/* sigsuspend() swaps the mask, sleeps until a handler runs, and puts the
 * old mask back afterwards. */
static void test_sigsuspend(void)
{
    g_sig_count = 0;
    install(SIGUSR1, count_signal, 0);
    sigset_t block;
    sigemptyset(&block);
    sigaddset(&block, SIGUSR1);
    sigset_t old;
    sigprocmask(SIG_BLOCK, &block, &old);
    raise(SIGUSR1);

    sigset_t wait_mask;
    sigemptyset(&wait_mask);
    int r = sigsuspend(&wait_mask);
    check_value(r == -EINTR, r, "sigsuspend returns -EINTR");
    check_value(g_sig_count == 1, g_sig_count, "sigsuspend ran the handler");

    sigset_t now = 0;
    sigprocmask(SIG_BLOCK, 0, &now);
    check(sigismember(&now, SIGUSR1) == 1, "sigsuspend restores the mask");
    sigprocmask(SIG_SETMASK, &old, 0);
    install(SIGUSR1, SIG_DFL, 0);
}

/* The parent learns that the child is about to block: the child writes a
 * byte, then (no preemption) keeps the CPU until it blocks itself. */
static void sync_wait(int fd)
{
    char c;
    (void)read(fd, &c, 1);
}

/* A caught signal interrupts a blocking read with -EINTR. */
static void test_signal_eintr(void)
{
    int sync[2];
    int r = pipe(sync);
    check_value(r == 0, r, "pipe for EINTR sync");
    if (r != 0)
        return;
    install(SIGUSR1, count_signal, 0);
    int pid = fork();
    if (pid == 0)
    {
        close(sync[0]);
        g_sig_count = 0;
        write(sync[1], "r", 1);
        char c;
        r = read(0, &c, 1);
        exit(r == -EINTR && g_sig_count == 1 ? 0 : 1);
    }
    install(SIGUSR1, SIG_DFL, 0);
    close(sync[1]);
    if (pid < 0)
    {
        close(sync[0]);
        return;
    }
    sync_wait(sync[0]);
    kill(pid, SIGUSR1);
    int status = wait_status(pid, 0);
    close(sync[0]);
    int code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    check_value(code == 0, code, "a handler interrupts read() with -EINTR");
}

static int g_restart_fd = -1;

static void restart_handler(int sig)
{
    g_sig_count++;
    g_sig_last = sig;
    write(g_restart_fd, "h", 1);
}

/* With SA_RESTART the interrupted read() runs again after the handler and
 * returns the data that arrives later. */
static void test_sa_restart(void)
{
    int sync[2];
    int data[2];
    if (pipe(sync) != 0 || pipe(data) != 0)
    {
        check(0, "pipes for SA_RESTART");
        return;
    }
    int pid = fork();
    if (pid == 0)
    {
        close(sync[0]);
        close(data[1]);
        g_restart_fd = sync[1];
        g_sig_count = 0;
        install(SIGUSR1, restart_handler, SA_RESTART);
        write(sync[1], "r", 1);
        char c = 0;
        int r = read(data[0], &c, 1);
        exit(r == 1 && c == 'x' && g_sig_count == 1 ? 0 : 1);
    }
    close(sync[1]);
    close(data[0]);
    if (pid < 0)
        return;
    sync_wait(sync[0]); /* child is in read() */
    kill(pid, SIGUSR1);
    sync_wait(sync[0]); /* handler ran; read() restarted and blocked */
    write(data[1], "x", 1);
    int status = wait_status(pid, 0);
    close(sync[0]);
    close(data[1]);
    int code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    check_value(code == 0, code, "SA_RESTART restarts an interrupted read");
}

/* SIGCHLD reaches the parent when a child exits. */
static void test_sigchld(void)
{
    g_sig_count = 0;
    install(SIGCHLD, count_signal, 0);
    int pid = fork();
    if (pid == 0)
        exit(0);
    int status = wait_status(pid, 0);
    check(WIFEXITED(status), "SIGCHLD child exits");
    check_value(g_sig_count == 1 && g_sig_last == SIGCHLD, g_sig_count,
                "the parent gets SIGCHLD");
    install(SIGCHLD, SIG_DFL, 0);
}

static void exit_42(int sig)
{
    (void)sig;
    exit(42);
}

/* Faults raise real signals: a handler can catch SIGSEGV, and other
 * exceptions report their own signal. */
static void test_fault_signals(void)
{
    int pid = fork();
    if (pid == 0)
    {
        install(SIGSEGV, exit_42, 0);
        /* Deliberate fault: the handler must run instead of the default
         * termination. */
        // cppcheck-suppress nullPointer
        *(volatile int*)0 = 1;
        exit(1);
    }
    int status = wait_status(pid, 0);
    int code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    check_value(code == 42, code, "a SIGSEGV handler catches a fault");

    pid = fork();
    if (pid == 0)
    {
        __asm__ volatile("ud2");
        exit(1);
    }
    status = wait_status(pid, 0);
    check_value(WIFSIGNALED(status) && WTERMSIG(status) == SIGILL, status,
                "an invalid opcode is SIGILL");

    pid = fork();
    if (pid == 0)
    {
        /* An ignored fault signal cannot stop the fault from killing. */
        install(SIGFPE, SIG_IGN, 0);
        /* A real div instruction: the compiler turns 1 / x into a compare. */
        unsigned int quotient = 1;
        __asm__ volatile("xor %%edx, %%edx\n\t"
                         "div %1"
                         : "+a"(quotient)
                         : "r"(0u)
                         : "edx");
        exit((int)quotient);
    }
    status = wait_status(pid, 0);
    check_value(WIFSIGNALED(status) && WTERMSIG(status) == SIGFPE, status,
                "a divide error is SIGFPE even when ignored");
}

/* There is no preemption: a task spinning in user mode only sees a signal
 * when an IRQ (the timer) returns to it. */
static void test_signal_spinning_child(void)
{
    int pid = fork();
    if (pid == 0)
    {
        for (;;)
        {
        }
    }
    if (pid < 0)
        return;
    kill(pid, SIGTERM);
    int status = wait_status(pid, 0);
    check_value(WIFSIGNALED(status) && WTERMSIG(status) == SIGTERM, status,
                "a signal reaches a task spinning in user mode");
}

/* Job control: stop, report with WUNTRACED, continue, report with
 * WCONTINUED. */
static void test_stop_continue(void)
{
    int pid = fork();
    if (pid == 0)
    {
        kill(getpid(), SIGSTOP);
        exit(4);
    }
    if (pid < 0)
        return;
    int status = wait_status(pid, WUNTRACED);
    check_value(WIFSTOPPED(status) && WSTOPSIG(status) == SIGSTOP, status,
                "waitpid(WUNTRACED) reports a stopped child");

    int r = waitpid(pid, &status, WNOHANG | WUNTRACED);
    check_value(r == 0, r, "a stop is reported only once");

    r = kill(pid, SIGCONT);
    check_value(r == 0, r, "SIGCONT is sent");
    status = wait_status(pid, WCONTINUED);
    check_value(WIFCONTINUED(status), status,
                "waitpid(WCONTINUED) reports the continue");

    status = wait_status(pid, 0);
    int code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    check_value(code == 4, code, "a continued child runs to its exit");
    printf("[selftest] job control: stop/continue ok\n");

    /* SIGTSTP stops by default too, and SIGKILL ends a stopped child. */
    pid = fork();
    if (pid == 0)
    {
        kill(getpid(), SIGTSTP);
        exit(1);
    }
    status = wait_status(pid, WUNTRACED);
    check_value(WIFSTOPPED(status) && WSTOPSIG(status) == SIGTSTP, status,
                "SIGTSTP stops by default");
    kill(pid, SIGKILL);
    status = wait_status(pid, 0);
    check_value(WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL, status,
                "SIGKILL ends a stopped child");
}

/* Process groups: the parent moves two children into a new group, signals
 * the group and reaps it with waitpid(-pgid). */
static void test_process_groups(void)
{
    check_value(getpgrp() == 1, getpgrp(), "init leads process group 1");

    int a = fork();
    if (a == 0)
    {
        char c;
        read(0, &c, 1);
        exit(1);
    }
    int b = fork();
    if (b == 0)
    {
        char c;
        read(0, &c, 1);
        exit(1);
    }
    if (a < 0 || b < 0)
        return;

    int r = setpgid(a, a);
    check_value(r == 0, r, "setpgid creates a group for a child");
    r = setpgid(b, a);
    check_value(r == 0, r, "setpgid moves a child into that group");
    check_value(getpgid(b) == a, getpgid(b), "getpgid reports the group");

    r = kill(-a, SIGTERM);
    check_value(r == 0, r, "kill(-pgid) signals the group");
    int reaped = 0;
    for (int i = 0; i < 2; i++)
    {
        int status = -1;
        int pid = waitpid(-a, &status, 0);
        if ((pid == a || pid == b) && WIFSIGNALED(status) &&
            WTERMSIG(status) == SIGTERM)
            reaped++;
    }
    check_value(reaped == 2, reaped, "every group member got the signal");
}

/* ---- Signal security probes -------------------------------------------- */

/* Handlers and restorers run in ring 3, so the kernel must refuse kernel
 * addresses; SIGKILL/SIGSTOP cannot be caught; pointers are validated. */
static void test_sec_sigaction(void)
{
    struct sigaction act;
    act.sa_handler = (sighandler_t)0x00100000; /* kernel image */
    act.sa_mask = 0;
    act.sa_flags = 0;
    act.sa_restorer = 0;
    int r = dit_syscall(SYS_sigaction, SIGUSR1, (unsigned int)&act, 0, 0, 0);
    check_value(r == -EINVAL, r, "security: kernel handler is -EINVAL");

    act.sa_handler = count_signal;
    act.sa_restorer = (void (*)(void))0x00100000;
    r = dit_syscall(SYS_sigaction, SIGUSR1, (unsigned int)&act, 0, 0, 0);
    check_value(r == -EINVAL, r, "security: kernel restorer is -EINVAL");

    r = install(SIGKILL, count_signal, 0);
    check_value(r == -EINVAL, r, "security: SIGKILL cannot be caught");
    r = install(SIGSTOP, SIG_IGN, 0);
    check_value(r == -EINVAL, r, "security: SIGSTOP cannot be ignored");
    r = install(NSIG, count_signal, 0);
    check_value(r == -EINVAL, r, "security: sigaction(NSIG) is -EINVAL");
    r = install(SIGUSR1, count_signal, 0x00000004u /* unknown flag */);
    check_value(r == -EINVAL, r, "security: unknown sa_flags are -EINVAL");

    /* The wrapper copies *act first, so call the kernel directly. */
    r = dit_syscall(SYS_sigaction, SIGUSR1, 0x00100000u, 0, 0, 0);
    check_value(r == -EFAULT, r, "security: sigaction act in kernel memory");
    r = dit_syscall(SYS_sigaction, SIGUSR1, 0, 0x00100000u, 0, 0);
    check_value(r == -EFAULT, r, "security: sigaction old in kernel memory");
    r = dit_syscall(SYS_sigprocmask, 7, 0, 0, 0, 0);
    check_value(r == 0, r, "sigprocmask ignores how without a set");
    sigset_t set = 0;
    r = sigprocmask(7, &set, 0);
    check_value(r == -EINVAL, r, "security: sigprocmask bad how is -EINVAL");
    r = dit_syscall(SYS_sigpending, 0x00100000u, 0, 0, 0, 0);
    check_value(r == -EFAULT, r, "security: sigpending into kernel memory");
}

/* Mirrors sched::SignalFrame (kernel/include/sched/signal.hpp). */
struct dit_signal_frame
{
    unsigned int magic, signo, eip, eflags, esp;
    unsigned int edi, esi, ebp, ebx, edx, ecx, eax;
    unsigned int saved_mask;
};
#define DIT_SIGNAL_FRAME_MAGIC 0x51A1F00Du

static volatile struct dit_signal_frame g_forged;
static unsigned int g_forged_stack[256];

/* Only reachable through the forged frame. With IOPL 0, cli faults. */
static void forged_target(void)
{
    __asm__ volatile("cli");
    exit(0);
}

/* sigreturn trusts nothing in the frame: a forged one asking for IOPL 3
 * with interrupts off must not grant either (CVE-class: sigreturn-oriented
 * privilege escalation). If it did, cli would succeed and exit(0). */
static void test_sec_sigreturn_forged(void)
{
    int pid = fork();
    if (pid == 0)
    {
        g_forged.magic = DIT_SIGNAL_FRAME_MAGIC;
        g_forged.eip = (unsigned int)forged_target;
        g_forged.esp = (unsigned int)&g_forged_stack[200];
        g_forged.eflags = 0x3002u; /* IOPL 3, IF clear */
        __asm__ volatile("mov %0, %%esp\n\t"
                         "mov %1, %%eax\n\t"
                         "int $0x80"
                         :
                         : "r"(&g_forged), "i"(SYS_sigreturn)
                         : "eax", "memory");
        exit(1);
    }
    int status = wait_status(pid, 0);
    check_value(WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV, status,
                "security: sigreturn cannot grant IOPL or clear IF");

    /* A frame with a bad magic (or no frame at all) kills the caller. */
    pid = fork();
    if (pid == 0)
    {
        g_forged.magic = 0;
        __asm__ volatile("mov %0, %%esp\n\t"
                         "mov %1, %%eax\n\t"
                         "int $0x80"
                         :
                         : "r"(&g_forged), "i"(SYS_sigreturn)
                         : "eax", "memory");
        exit(1);
    }
    status = wait_status(pid, 0);
    check_value(WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV, status,
                "security: a bad sigreturn frame is SIGSEGV");

    /* A frame pointing EIP into the kernel is refused the same way. */
    pid = fork();
    if (pid == 0)
    {
        g_forged.magic = DIT_SIGNAL_FRAME_MAGIC;
        g_forged.eip = 0x00100000u;
        g_forged.esp = (unsigned int)&g_forged_stack[200];
        g_forged.eflags = 0x202u;
        __asm__ volatile("mov %0, %%esp\n\t"
                         "mov %1, %%eax\n\t"
                         "int $0x80"
                         :
                         : "r"(&g_forged), "i"(SYS_sigreturn)
                         : "eax", "memory");
        exit(1);
    }
    status = wait_status(pid, 0);
    check_value(WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV, status,
                "security: sigreturn to a kernel EIP is SIGSEGV");
}

/* Group signals and setpgid obey the same permission rules as kill():
 * a child cannot reach init's group members or invent a group. */
static void test_sec_job_control(void)
{
    /* A sibling's group holds no descendant of the caller. */
    int victim = fork();
    if (victim == 0)
    {
        char c;
        read(0, &c, 1);
        exit(1);
    }
    if (victim < 0)
        return;
    setpgid(victim, victim);

    int pid = fork();
    if (pid == 0)
    {
        int ok = 1;
        if (kill(-victim, 0) != -EPERM)
            ok = 0;
        if (kill(-victim, SIGKILL) != -EPERM)
            ok = 0;
        /* A child cannot move its parent or a sibling. */
        if (setpgid(1, 0) != -ESRCH || setpgid(victim, 0) != -ESRCH)
            ok = 0;
        /* Joining a group nobody has is refused. */
        if (setpgid(0, 4242) != -EPERM)
            ok = 0;
        if (setpgid(-1, 0) != -EINVAL)
            ok = 0;
        exit(ok ? 0 : 1);
    }
    int status = wait_status(pid, 0);
    int code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    check_value(code == 0, code, "security: job-control permission checks");
    kill(victim, SIGKILL);
    status = wait_status(victim, 0);
    check(WIFSIGNALED(status), "the group-permission victim survived");

    int r = waitpid(-1, 0, 0x100);
    check_value(r == -EINVAL, r, "security: unknown waitpid options");
}

/* ---- Pipes, descriptors, working directory ------------------------------ */

static int str_eq(const char* a, const char* b)
{
    while (*a && *a == *b)
    {
        a++;
        b++;
    }
    return *a == *b;
}

/* Read @fd to EOF into @buf (NUL-terminated); returns the byte count. */
static int read_all(int fd, char* buf, int cap)
{
    int total = 0;
    for (;;)
    {
        int n = read(fd, buf + total, (unsigned)(cap - 1 - total));
        if (n <= 0)
            break;
        total += n;
        if (total >= cap - 1)
            break;
    }
    buf[total] = '\0';
    return total;
}

enum
{
    BIG_TRANSFER = 3 * 4096 + 100 /* more than the pipe holds */
};

/* Data written to one end comes out of the other; EOF once every writer
 * is gone; a writer blocks while the pipe is full. */
static void test_pipe(void)
{
    int p[2] = { -1, -1 };
    int r = pipe(p);
    check_value(r == 0, r, "pipe succeeds");
    if (r != 0)
        return;
    check(p[0] >= 0 && p[1] >= 0 && p[0] != p[1], "pipe returns two fds");

    r = write(p[1], "abc", 3);
    check_value(r == 3, r, "write into a pipe");
    char buf[8] = { 0 };
    r = read(p[0], buf, sizeof(buf));
    check_value(r == 3 && buf[0] == 'a' && buf[2] == 'c', r,
                "read back from a pipe");
    close(p[1]);
    r = read(p[0], buf, sizeof(buf));
    check_value(r == 0, r, "EOF once the write end is closed");
    close(p[0]);

    /* More data than the buffer holds: the child's writes must block until
     * the parent drains, and nothing may be lost or reordered. */
    if (pipe(p) != 0)
        return;
    int pid = fork();
    if (pid == 0)
    {
        close(p[0]);
        char chunk[1000];
        int sent = 0;
        while (sent < BIG_TRANSFER)
        {
            int n = BIG_TRANSFER - sent;
            if (n > (int)sizeof(chunk))
                n = (int)sizeof(chunk);
            for (int i = 0; i < n; i++)
                chunk[i] = (char)((sent + i) * 7);
            if (write(p[1], chunk, (unsigned)n) != n)
                exit(1);
            sent += n;
        }
        exit(0);
    }
    close(p[1]);
    int got = 0;
    int ordered = 1;
    char in[300];
    for (;;)
    {
        int n = read(p[0], in, sizeof(in));
        if (n <= 0)
            break;
        for (int i = 0; i < n; i++)
            if (in[i] != (char)((got + i) * 7))
                ordered = 0;
        got += n;
    }
    close(p[0]);
    int status = wait_status(pid, 0);
    check(WIFEXITED(status) && WEXITSTATUS(status) == 0,
          "the pipe writer finished");
    check_value(got == BIG_TRANSFER && ordered, got,
                "a pipe carries more than its capacity in order");
    printf("[selftest] pipe: %d bytes through a pipe\n", got);
}

/* Writing to a pipe nobody reads is SIGPIPE, or -EPIPE when ignored. */
static void test_pipe_epipe(void)
{
    int p[2];
    if (pipe(p) != 0)
        return;
    close(p[0]);
    int pid = fork();
    if (pid == 0)
    {
        write(p[1], "x", 1);
        exit(1);
    }
    int status = wait_status(pid, 0);
    check_value(WIFSIGNALED(status) && WTERMSIG(status) == SIGPIPE, status,
                "writing to a closed pipe raises SIGPIPE");

    install(SIGPIPE, SIG_IGN, 0);
    int r = write(p[1], "x", 1);
    check_value(r == -EPIPE, r, "with SIGPIPE ignored the write is -EPIPE");
    install(SIGPIPE, SIG_DFL, 0);
    close(p[1]);
}

/* dup2 redirects a child's stdout into a pipe; dup takes the lowest fd. */
static void test_dup(void)
{
    int p[2];
    if (pipe(p) != 0)
        return;
    int pid = fork();
    if (pid == 0)
    {
        if (dup2(p[1], 1) != 1)
            exit(1);
        close(p[0]);
        close(p[1]);
        printf("dup2-ok");
        exit(0);
    }
    close(p[1]);
    char buf[32];
    read_all(p[0], buf, sizeof(buf));
    close(p[0]);
    wait_status(pid, 0);
    check(str_eq(buf, "dup2-ok"), "dup2 redirects stdout into a pipe");

    /* close(0) frees the slot: open() then reuses it (lowest free fd). */
    int saved = dup(0);
    check_value(saved >= 3, saved, "dup returns a new fd");
    close(0);
    int fd = open("/hello.txt", 0);
    check_value(fd == 0, fd, "open reuses the lowest free fd");
    close(fd);
    int r = dup2(saved, 0);
    check_value(r == 0, r, "dup2 restores stdin");
    close(saved);
    struct stat st = { 0 };
    check(fstat(0, &st) == 0 && S_ISCHR(st.st_mode), "stdin is the console");
}

/* close() must release the file, not just the slot (the slot was cleared
 * before the file was released, leaking it). The kernel heap is 256 KiB,
 * so a leak runs these loops out of memory. */
static void test_fd_leaks(void)
{
    int failed = -1;
    for (int i = 0; i < 1500; i++)
    {
        int fd = open("/hello.txt", 0);
        if (fd < 0 || close(fd) != 0)
        {
            failed = i;
            break;
        }
    }
    check_value(failed < 0, failed, "open/close does not leak files");

    failed = -1;
    for (int i = 0; i < 150; i++)
    {
        int p[2];
        if (pipe(p) != 0 || close(p[0]) != 0 || close(p[1]) != 0)
        {
            failed = i;
            break;
        }
    }
    check_value(failed < 0, failed, "pipe/close does not leak pipes");
}

/* chdir/getcwd and relative paths resolve against the working directory. */
static void test_cwd(void)
{
    char buf[64];
    int r = getcwd(buf, sizeof(buf));
    check_value(r == 2 && str_eq(buf, "/"), r, "init starts in /");

    r = chdir("home");
    check_value(r == 0, r, "chdir to a relative directory");
    getcwd(buf, sizeof(buf));
    check(str_eq(buf, "/home"), "getcwd reports /home");
    int fd = open("notes.txt", 0);
    check_value(fd >= 0, fd, "relative open resolves in the cwd");
    if (fd >= 0)
        close(fd);
    struct stat st = { 0 };
    r = stat("../hello.txt", &st);
    check_value(r == 0, r, "stat resolves .. against the cwd");

    /* A child's chdir is its own. */
    int pid = fork();
    if (pid == 0)
    {
        char child[64];
        int ok = getcwd(child, sizeof(child)) > 0 && str_eq(child, "/home");
        exit(ok && chdir("/") == 0 ? 0 : 1);
    }
    int status = wait_status(pid, 0);
    check(WIFEXITED(status) && WEXITSTATUS(status) == 0,
          "fork inherits the cwd");
    getcwd(buf, sizeof(buf));
    check(str_eq(buf, "/home"), "a child's chdir does not move the parent");

    r = chdir("..");
    check_value(r == 0, r, "chdir ..");
    getcwd(buf, sizeof(buf));
    check(str_eq(buf, "/"), "chdir .. returns to /");

    r = chdir("/hello.txt");
    check_value(r == -ENOTDIR, r, "chdir to a file is -ENOTDIR");
    r = chdir("/no-such-dir");
    check_value(r == -ENOENT, r, "chdir to a missing dir is -ENOENT");
    r = chdir("");
    check_value(r == -ENOENT, r, "chdir(\"\") is -ENOENT");
    printf("[selftest] cwd: chdir/getcwd ok\n");
}

/* Job control on the console: a background read stops the reader with
 * SIGTTIN (or fails with -EIO when SIGTTIN is ignored), and tcsetpgrp
 * hands the terminal over. */
static void test_tty_job_control(void)
{
    int fg = tcgetpgrp(0);
    check_value(fg == 1, fg, "init owns the console");

    int pid = fork();
    if (pid == 0)
    {
        setpgid(0, 0);
        char c;
        read(0, &c, 1);
        exit(1);
    }
    setpgid(pid, pid);
    int status = wait_status(pid, WUNTRACED);
    check_value(WIFSTOPPED(status) && WSTOPSIG(status) == SIGTTIN, status,
                "a background read stops with SIGTTIN");
    kill(pid, SIGKILL);
    wait_status(pid, 0);

    pid = fork();
    if (pid == 0)
    {
        setpgid(0, 0);
        install(SIGTTIN, SIG_IGN, 0);
        char c;
        exit(read(0, &c, 1) == -EIO ? 0 : 1);
    }
    setpgid(pid, pid);
    status = wait_status(pid, 0);
    check(WIFEXITED(status) && WEXITSTATUS(status) == 0,
          "a background read with SIGTTIN ignored is -EIO");

    /* Hand the console to a child's group and take it back. init ignores
     * SIGTTOU while it is in the background, as a shell does. */
    install(SIGTTOU, SIG_IGN, 0);
    pid = fork();
    if (pid == 0)
        exit(tcgetpgrp(0) == getpgrp() ? 0 : 1);
    setpgid(pid, pid);
    int r = tcsetpgrp(0, pid);
    check_value(r == 0, r, "tcsetpgrp gives the console to a child group");
    status = wait_status(pid, 0);
    check(WIFEXITED(status) && WEXITSTATUS(status) == 0,
          "the child sees itself in the foreground");
    r = tcsetpgrp(0, getpgrp());
    check_value(r == 0, r, "a background shell takes the console back");
    check_value(tcgetpgrp(0) == 1, tcgetpgrp(0), "init owns the console again");

    /* A background group without SIGTTOU ignored is stopped instead. */
    pid = fork();
    if (pid == 0)
    {
        setpgid(0, 0);
        install(SIGTTOU, SIG_DFL, 0); /* fork inherited init's SIG_IGN */
        tcsetpgrp(0, getpgrp());
        exit(1);
    }
    setpgid(pid, pid);
    status = wait_status(pid, WUNTRACED);
    check_value(WIFSTOPPED(status) && WSTOPSIG(status) == SIGTTOU, status,
                "tcsetpgrp from the background stops with SIGTTOU");
    kill(pid, SIGKILL);
    wait_status(pid, 0);
    install(SIGTTOU, SIG_DFL, 0);
    check_value(tcgetpgrp(0) == 1, tcgetpgrp(0),
                "a stopped tcsetpgrp did not take the console");
    printf("[selftest] tty job control ok\n");
}

/* M41: init, blocked in read() like the shell at its prompt, must get
 * SIGCHLD when a zombie is reparented to it, or orphans pile up until the
 * next prompt. Chain: init -> A -> M -> G. G exits (a zombie of M), then M
 * exits, so G reaches init already dead. A only reports back afterwards, so
 * any SIGCHLD init sees before A's byte can only be G's. */
static void test_orphan_sigchld(void)
{
    int report[2];
    int hold[2]; /* keeps A alive until init has sampled the counter */
    if (pipe(report) != 0)
        return;
    if (pipe(hold) != 0)
    {
        close(report[0]);
        close(report[1]);
        return;
    }
    g_sig_count = 0;
    install(SIGCHLD, count_signal, SA_RESTART);
    int a = fork();
    if (a == 0)
    {
        close(report[0]);
        close(hold[1]);
        int gone[2];
        if (pipe(gone) != 0)
            exit(1);
        int m = fork();
        if (m == 0)
        {
            int g = fork();
            if (g == 0)
            {
                close(gone[0]);
                exit(6); /* closing gone[1] at exit wakes M */
            }
            close(gone[1]);
            sync_wait(gone[0]); /* EOF once G has exited */
            exit(0); /* G, a zombie, is reparented to init */
        }
        close(gone[0]);
        close(gone[1]);
        waitpid(m, 0, 0);
        write(report[1], "x", 1);
        /* No preemption: without this, A's own exit would signal init
         * before init looks at the counter. */
        sync_wait(hold[0]);
        exit(0);
    }
    close(report[1]);
    close(hold[0]);
    if (a < 0)
    {
        close(report[0]);
        close(hold[1]);
        install(SIGCHLD, SIG_DFL, 0);
        return;
    }
    /* SA_RESTART: the handler runs and the read carries on, as in the
     * shell. */
    sync_wait(report[0]);
    int before_a_exits = g_sig_count;
    close(report[0]);
    close(hold[1]); /* A sees EOF and exits */

    int reaped = 0;
    int orphan_code = -1;
    for (int i = 0; i < 2; i++)
    {
        int status = -1;
        int pid = waitpid(-1, &status, 0);
        if (pid > 0)
            reaped++;
        if (pid > 0 && pid != a && WIFEXITED(status))
            orphan_code = WEXITSTATUS(status);
    }
    install(SIGCHLD, SIG_DFL, 0);
    check_value(before_a_exits >= 1, before_a_exits,
                "init blocked in read() gets SIGCHLD for a reparented zombie "
                "(M41)");
    check_value(reaped == 2, reaped, "init reaps its child and the orphan");
    check_value(orphan_code == 6, orphan_code, "the orphan keeps its status");
}

/* malloc() must refuse sizes that would wrap its arithmetic (M38). */
static void test_malloc_overflow(void)
{
    void* huge = malloc((size_t)-1);
    check(huge == 0, "malloc(SIZE_MAX) is NULL (M38)");
    free(huge);
    void* wrap = malloc((size_t)-4);
    check(wrap == 0, "malloc(SIZE_MAX - 3) is NULL (M38)");
    free(wrap);
    void* big = malloc(64 * 1024 + 1);
    check(big == 0, "malloc beyond the heap is NULL");
    free(big);
    char* p = (char*)malloc(16);
    check(p != 0, "a small malloc still succeeds");
    if (p)
        p[15] = 'x';
    free(p);
}

/* ---- Descriptor and path security probes -------------------------------- */

static void test_sec_pipe_dup(void)
{
    int before = dup(0);
    close(before);

    int r = pipe((int*)0x00100000);
    check_value(r == -EFAULT, r, "security: pipe into kernel memory");
    r = pipe(0);
    check_value(r == -EFAULT, r, "security: pipe(NULL) is -EFAULT");
    int after = dup(0);
    close(after);
    check_value(after == before, after, "security: a failed pipe leaks no fd");

    r = dup2(0, 32);
    check_value(r == -EBADF, r, "security: dup2 past the table is -EBADF");
    r = dup2(-1, 3);
    check_value(r == -EBADF, r, "security: dup2(-1) is -EBADF");
    r = dup2(20, 3);
    check_value(r == -EBADF, r, "security: dup2 of a closed fd is -EBADF");
    r = dup(-1);
    check_value(r == -EBADF, r, "security: dup(-1) is -EBADF");
    r = close(32);
    check_value(r == -EBADF, r, "security: close past the table is -EBADF");

    int p[2];
    if (pipe(p) != 0)
        return;
    char c = 0;
    r = read(p[1], &c, 1);
    check_value(r == -EBADF, r, "security: reading a write end is -EBADF");
    r = write(p[0], &c, 1);
    check_value(r == -EBADF, r, "security: writing a read end is -EBADF");
    r = ioctl(p[0], TTY_IOC_GETMODE, 0);
    check_value(r == -ENOTTY, r, "security: tty ioctl on a pipe is -ENOTTY");
    char dents[DIT_DIRENT_SIZE];
    r = getdents(p[0], dents, sizeof(dents));
    check_value(r < 0, r, "security: getdents on a pipe fails cleanly");
    r = getdents(0, dents, sizeof(dents));
    check_value(r < 0, r, "security: getdents on the console fails cleanly");

    /* An inode-less file must still fill every byte of struct stat. */
    struct stat st;
    for (unsigned i = 0; i < sizeof(st); i++)
        ((unsigned char*)&st)[i] = 0xAA;
    r = fstat(p[0], &st);
    int leftover = 0;
    for (unsigned i = 0; i < sizeof(st); i++)
        if (((const unsigned char*)&st)[i] == 0xAA)
            leftover++;
    check_value(r == 0 && S_ISFIFO(st.st_mode) && leftover == 0, leftover,
                "security: fstat of a pipe writes every byte");
    close(p[0]);
    close(p[1]);
}

static void test_sec_paths(void)
{
    char buf[64];
    int r = getcwd((char*)0x00100000, 64);
    check_value(r == -EFAULT, r, "security: getcwd into kernel memory");
    r = getcwd(buf, 1);
    check_value(r == -ERANGE, r, "security: getcwd short buffer is -ERANGE");
    r = chdir((const char*)0x00100000);
    check_value(r == -EFAULT, r, "security: chdir path in kernel memory");

    static char long_path[300];
    for (int i = 0; i < (int)sizeof(long_path) - 1; i++)
        long_path[i] = (i % 2) ? '/' : 'a';
    long_path[sizeof(long_path) - 1] = '\0';
    r = chdir(long_path);
    check_value(r == -ENAMETOOLONG, r, "security: an over-long path");

    /* ".." never climbs above the root. */
    r = chdir("/../../..");
    check_value(r == 0, r, "chdir above the root stays at /");
    getcwd(buf, sizeof(buf));
    check(str_eq(buf, "/"), "security: .. cannot escape the root");

    r = tcsetpgrp(0, 4242);
    check_value(r == -EPERM, r, "security: tcsetpgrp to no group is -EPERM");
    r = tcsetpgrp(0, 0);
    check_value(r == -EINVAL, r, "security: tcsetpgrp(0) is -EINVAL");
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
    test_signal_handler();
    test_signal_mask();
    test_sigsuspend();
    test_signal_eintr();
    test_sa_restart();
    test_sigchld();
    test_fault_signals();
    test_signal_spinning_child();
    test_stop_continue();
    test_process_groups();
    test_sec_sigaction();
    test_sec_sigreturn_forged();
    test_sec_job_control();
    test_pipe();
    test_pipe_epipe();
    test_dup();
    test_fd_leaks();
    test_cwd();
    test_tty_job_control();
    test_sec_pipe_dup();
    test_sec_paths();
    test_orphan_sigchld();
    test_malloc_overflow();

    if (failures == 0)
        printf("[selftest] ok\n");
    else
        printf("[selftest] %d failure(s)\n", failures);

    userland_shell_main();
    return 0;
}
