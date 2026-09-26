// DIT-OS init: runs the kernel self-test, then the shell.
//
// The self-test covers the process model from user space, so a broken
// page-fault handler, copy-on-write fork or ELF loader shows up at boot
// instead of at the first shell command that happens to need it.

#include "syscall.h"
#include "stdio.h"

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
    check_value(status == 7, status, "waitpid returns the exit code");

    int none = waitpid(-1, 0, 0);
    check_value(none == -10, none, "waitpid without children is -ECHILD");
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
    check_value(status == 3, status,
                "the exec'd program ran and exited with 3");
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    test_memory();
    test_fork();
    test_execve();

    if (failures == 0)
        printf("[selftest] ok\n");
    else
        printf("[selftest] %d failure(s)\n", failures);

    userland_shell_main();
    return 0;
}
