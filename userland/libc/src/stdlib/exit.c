// Minimal userland exit() via SYS_exit

#include "stdlib.h"
#include "syscall.h"

void _exit(int code) {
    // Delegate to the current syscall wrapper from userland/include/syscall.h.
    exit(code);
}

