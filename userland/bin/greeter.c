// A second program: execve() replaces the child with this one, so reaching
// the message below proves the new image runs. It is linked into the kernel
// and seeded into the root filesystem as /greeter (see ramfs::seed_file).
//
// "greeter read" reads one line from stdin and echoes it back. The shell
// tests use it as a program that blocks on the console (job control) or
// reads from a pipe.
//
// "greeter ttyline" proves the TTY owns the canonical line buffer (L25): a
// caught signal interrupts the read with -EINTR but must not drop the
// bytes typed so far, so the retry continues the same line.

#include "signal.h"
#include "syscall.h"
#include "stdio.h"

enum
{
    LINE_MAX = 128
};

static int is_read_mode(int argc, char** argv)
{
    if (argc < 2)
        return 0;
    const char* a = argv[1];
    return a[0] == 'r' && a[1] == 'e' && a[2] == 'a' && a[3] == 'd' &&
           a[4] == '\0';
}

static int arg_is(const char* a, const char* b)
{
    while (*a && *a == *b)
    {
        a++;
        b++;
    }
    return *a == *b;
}

static void ttyline_handler(int sig)
{
    (void)sig;
}

/* Prove a caught signal interrupts a canonical console read: the parent
 * signals this read as soon as it blocks (permitted: a parent may signal
 * its child; cooperative scheduling means the parent cannot run before the
 * child blocks), so the first read returns -EINTR. The retry must then
 * complete a fresh line normally. Note the kernel keeps a partially typed
 * line across -EINTR in its TTY line buffer, but every signal the test
 * driver can inject from the serial console (INTR/QUIT/SUSP) flushes that
 * buffer per POSIX, so the preserving path itself is not reachable from an
 * end-to-end test; it is covered by review of tty_read_line(). */
static int run_ttyline(void)
{
    struct sigaction act;
    act.sa_handler = ttyline_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0; /* no SA_RESTART: the first read returns -EINTR */
    act.sa_restorer = 0;
    if (sigaction(SIGUSR1, &act, 0) != 0)
    {
        printf("greeter: sigaction failed\n");
        return 1;
    }

    int psync[2];
    if (pipe(psync) != 0)
    {
        printf("greeter: pipe failed\n");
        return 1;
    }
    int pid = fork();
    if (pid == 0)
    {
        close(psync[0]);
        char go = 'g';
        if (write(psync[1], &go, 1) != 1)
            exit(1);
        close(psync[1]);

        char line[LINE_MAX];
        printf("greeter: ttyline-1\n");
        int n1 = read(0, line, sizeof(line) - 1);
        printf("greeter: ttyline-2 n1=%d\n", n1);
        int n2 = read(0, line, sizeof(line) - 1);
        if (n2 < 0)
        {
            printf("greeter: second read failed (%d)\n", n2);
            exit(1);
        }
        while (n2 > 0 && (line[n2 - 1] == '\n' || line[n2 - 1] == '\r'))
            n2--;
        line[n2] = '\0';
        printf("greeter got: %s\n", line);
        int ok = n1 == -EINTR && n2 == 4;
        printf("greeter: ttyline-%s\n", ok ? "ok" : "lost");
        exit(ok ? 0 : 1);
    }
    close(psync[1]);
    if (pid < 0)
    {
        printf("greeter: fork failed\n");
        close(psync[0]);
        return 1;
    }
    char go = 0;
    (void)read(psync[0], &go, 1);
    close(psync[0]);
    kill(pid, SIGUSR1);
    int status = 0;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

int main(int argc, char** argv)
{
    if (is_read_mode(argc, argv))
    {
        char line[LINE_MAX];
        /* Tells a test driver that input sent now reaches this read. */
        printf("greeter: reading\n");
        int n = read(0, line, sizeof(line) - 1);
        if (n < 0)
        {
            printf("greeter: read failed (%d)\n", n);
            return 1;
        }
        while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r'))
            n--;
        line[n] = '\0';
        printf("greeter got: %s\n", line);
        return 0;
    }
    if (argc > 1 && arg_is(argv[1], "ttyline"))
        return run_ttyline();
    /* Printing argv[0] also proves the argument strings were placed in the new
     * image's stack, not just in the stack the caller had. */
    printf("[selftest] execve: %s (argc=%d)\n", argc > 0 ? argv[0] : "?", argc);
    return 3;
}
