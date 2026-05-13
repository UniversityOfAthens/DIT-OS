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
#define SYS_getdents 16

/** Must match kernel fs::dirent layout (see kernel/include/fs/dirent.hpp). */
#define DIT_DIRENT_SIZE 264
struct dit_dirent {
    unsigned int d_ino;
    unsigned char d_type;
    unsigned char d_pad[3];
    char d_name[256];
};

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

#endif
