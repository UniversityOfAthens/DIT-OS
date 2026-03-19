#ifndef KERNEL_FS_FCNTL_HPP
#define KERNEL_FS_FCNTL_HPP

#include <stdint.h>

namespace fs {

/** Open flags (POSIX-style) */
#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_ACCMODE   0x0003

#define O_CREAT     0x0040
#define O_EXCL      0x0080
#define O_NOCTTY    0x0100
#define O_TRUNC     0x0200
#define O_APPEND    0x0400
#define O_NONBLOCK  0x0800
#define O_NDELAY    O_NONBLOCK
#define O_SYNC      0x1000
#define O_CLOEXEC   0x2000

} // namespace fs

#endif
