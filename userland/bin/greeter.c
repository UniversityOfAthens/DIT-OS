// A second program for the boot self-test: execve() replaces the child
// with this one, so reaching the message below proves the new image runs.
// It is linked into the kernel and seeded into the root filesystem as
// /greeter (see ramfs::seed_file).

#include "syscall.h"
#include "stdio.h"

int main(int argc, char** argv)
{
    /* Printing argv[0] also proves the argument strings were placed in the new
     * image's stack, not just in the stack the caller had. */
    printf("[selftest] execve: %s (argc=%d)\n", argc > 0 ? argv[0] : "?", argc);
    return 3;
}
