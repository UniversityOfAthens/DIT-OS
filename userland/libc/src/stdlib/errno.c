// DIT-OS userland libc - the POSIX errno variable
//
// One per process (userland has no threads). The raw syscall wrappers set it
// to the absolute errno on failure while still returning -errno themselves;
// see userland/libc/include/errno.h.

int errno;
