// Minimal userland exit() via SYS_exit

#include "stdlib.h"
#include "syscall.h"

void _exit(int code) {
    // Delegate to the syscall wrapper from userland/include/syscall.h.
    syscall1(SYS_exit, code);
    __builtin_unreachable();
}

