/* DIT-OS Userland - Syscall ABI (temporary)
 *
 * Kernel-exported C ABI wrappers (dit_sys_*) route to the syscall dispatcher.
 */
#ifndef USERLAND_SYSCALL_H
#define USERLAND_SYSCALL_H

#define SYS_read 0
#define SYS_write 1
#define SYS_open 2
#define SYS_close 3
#define SYS_exit 6
#define SYS_getpid 10
#define SYS_ioctl 15
#define SYS_getdents 16

/** Console TTY ioctls on fds 0-2 (must match kernel syscall/tty.hpp).
 * TTY_IOC_GETMODE returns the mode; TTY_IOC_SETMODE takes it as the value. */
#define TTY_IOC_GETMODE 0x5401
#define TTY_IOC_SETMODE 0x5402
#define TTY_MODE_ECHO 0x1 /* echo input back */
#define TTY_MODE_CANON 0x2 /* kernel line editing, read() returns lines */

/** Must match kernel fs::dirent layout (see kernel/include/fs/dirent.hpp). */
#define DIT_DIRENT_SIZE 264
struct dit_dirent {
    unsigned int d_ino;
    unsigned char d_type;
    unsigned char d_pad[3];
    char d_name[256];
};
#define DIT_DT_DIR 4 /* d_type of a directory (kernel DT_DIR) */

#ifdef __cplusplus
extern "C" {
#endif
int dit_sys_read(int fd, void* buf, unsigned count);
int dit_sys_write(int fd, const void* buf, unsigned count);
void dit_sys_exit(int code);
int dit_sys_getpid(void);
int dit_sys_open(const char* path, int flags);
int dit_sys_close(int fd);
int dit_sys_getdents(int fd, void* buf, unsigned count);
int dit_sys_ioctl(int fd, unsigned long request, unsigned long arg);
#ifdef __cplusplus
}
#endif

static inline int read(int fd, void* buf, unsigned count)
{
    (void)SYS_read;
    return dit_sys_read(fd, buf, count);
}
static inline int write(int fd, const void* buf, unsigned count)
{
    (void)SYS_write;
    return dit_sys_write(fd, buf, count);
}
static inline void exit(int code)
{
    (void)SYS_exit;
    dit_sys_exit(code);
    __builtin_unreachable();
}

static inline int getpid(void)
{
    (void)SYS_getpid;
    return dit_sys_getpid();
}

static inline int open(const char* path, int flags)
{
    (void)SYS_open;
    return dit_sys_open(path, flags);
}

static inline int close(int fd)
{
    (void)SYS_close;
    return dit_sys_close(fd);
}

static inline int getdents(int fd, void* buf, unsigned count)
{
    (void)SYS_getdents;
    return dit_sys_getdents(fd, buf, count);
}

/* Simplified from the variadic POSIX/Linux prototype: the argument is always
 * passed by value. */
static inline int ioctl(int fd, unsigned long request, unsigned long arg)
{
    (void)SYS_ioctl;
    return dit_sys_ioctl(fd, request, arg);
}

#endif
