// A second program: execve() replaces the child with this one, so reaching
// the message below proves the new image runs. It is linked into the kernel
// and seeded into the root filesystem as /greeter (see ramfs::seed_file).
//
// "greeter read" reads one line from stdin and echoes it back. The shell
// tests use it as a program that blocks on the console (job control) or
// reads from a pipe.

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
    /* Printing argv[0] also proves the argument strings were placed in the new
     * image's stack, not just in the stack the caller had. */
    printf("[selftest] execve: %s (argc=%d)\n", argc > 0 ? argv[0] : "?", argc);
    return 3;
}
