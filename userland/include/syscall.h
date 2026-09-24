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

#define TTY_IOC_GETMODE 0x5401
#define TTY_IOC_SETMODE 0x5402
#define TTY_IOC_POWER_OFF 0x5403
#define TTY_MODE_ECHO 0x1
#define TTY_MODE_CANON 0x2

#define DIT_DIRENT_SIZE 264
struct dit_dirent
{
    unsigned int d_ino;
    unsigned char d_type;
    unsigned char d_pad[3];
    char d_name[256];
};
#define DIT_DT_DIR 4

static inline int dit_syscall(int number, unsigned int arg0, unsigned int arg1,
                              unsigned int arg2, unsigned int arg3,
                              unsigned int arg4)
{
    register unsigned int rax __asm__("eax") = (unsigned int)number;
    register unsigned int rbx __asm__("ebx") = arg0;
    register unsigned int rcx __asm__("ecx") = arg1;
    register unsigned int rdx __asm__("edx") = arg2;
    register unsigned int rsi __asm__("esi") = arg3;
    register unsigned int rdi __asm__("rdi") = arg4;
    __asm__ volatile("int $0x80"
                     : "+a"(rax)
                     : "b"(rbx), "c"(rcx), "d"(rdx), "S"(rsi), "D"(rdi)
                     : "memory");
    return (int)rax;
}

static inline int read(int fd, void* buf, unsigned count)
{
    return dit_syscall(SYS_read, (unsigned int)fd, (unsigned int)buf, count, 0,
                       0);
}

static inline int write(int fd, const void* buf, unsigned count)
{
    return dit_syscall(SYS_write, (unsigned int)fd, (unsigned int)buf, count, 0,
                       0);
}

static inline void exit(int code)
{
    (void)dit_syscall(SYS_exit, (unsigned int)code, 0, 0, 0, 0);
    __builtin_unreachable();
}

static inline int getpid(void)
{
    return dit_syscall(SYS_getpid, 0, 0, 0, 0, 0);
}

static inline int open(const char* path, int flags)
{
    return dit_syscall(SYS_open, (unsigned int)path, (unsigned int)flags, 0, 0,
                       0);
}

static inline int close(int fd)
{
    return dit_syscall(SYS_close, (unsigned int)fd, 0, 0, 0, 0);
}

static inline int getdents(int fd, void* buf, unsigned count)
{
    return dit_syscall(SYS_getdents, (unsigned int)fd, (unsigned int)buf, count,
                       0, 0);
}

static inline int ioctl(int fd, unsigned long request, unsigned long arg)
{
    return dit_syscall(SYS_ioctl, (unsigned int)fd, (unsigned int)request,
                       (unsigned int)arg, 0, 0);
}

#endif
