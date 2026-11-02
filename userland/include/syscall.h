#ifndef USERLAND_SYSCALL_H
#define USERLAND_SYSCALL_H

#define SYS_read 0
#define SYS_write 1
#define SYS_open 2
#define SYS_close 3
#define SYS_stat 4
#define SYS_fstat 5
#define SYS_exit 6
#define SYS_fork 7
#define SYS_execve 8
#define SYS_waitpid 9
#define SYS_getpid 10
#define SYS_mmap 11
#define SYS_munmap 12
#define SYS_brk 13
#define SYS_kill 14
#define SYS_ioctl 15
#define SYS_getdents 16

#define TTY_IOC_GETMODE 0x5401
#define TTY_IOC_SETMODE 0x5402
#define TTY_IOC_POWER_OFF 0x5403
#define TTY_MODE_ECHO 0x1
#define TTY_MODE_CANON 0x2

/* waitpid */
#define WNOHANG 1

/* waitpid status decoding (Linux-compatible layout; see
 * kernel/include/sched/wait.hpp). */
#define WIFEXITED(s) (((s) & 0x7f) == 0)
#define WEXITSTATUS(s) (((s) >> 8) & 0xff)
#define WIFSIGNALED(s) (((s) & 0x7f) != 0 && ((s) & 0x7f) != 0x7f)
#define WTERMSIG(s) ((s) & 0x7f)

/* Signals (same values as kernel/include/sched/signal.hpp). */
#define SIGKILL 9
#define SIGSEGV 11
#define SIGTERM 15
#define SIGCHLD 17
#define SIGSTOP 19
#define NSIG 32

/* Errno values (same as kernel/include/errno_defs.hpp). Wrappers return the
 * raw negative value, not -1 with errno set. */
#define EPERM 1
#define ENOENT 2
#define ESRCH 3
#define EBADF 9
#define ECHILD 10
#define EFAULT 14
#define EINVAL 22
#define ENOSYS 38

/* mmap */
#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define MAP_PRIVATE 0x02
#define MAP_ANONYMOUS 0x20

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

static inline int fork(void)
{
    return dit_syscall(SYS_fork, 0, 0, 0, 0, 0);
}

static inline int waitpid(int pid, int* status, int options)
{
    return dit_syscall(SYS_waitpid, (unsigned int)pid, (unsigned int)status,
                       (unsigned int)options, 0, 0);
}

static inline int kill(int pid, int sig)
{
    return dit_syscall(SYS_kill, (unsigned int)pid, (unsigned int)sig, 0, 0, 0);
}

/* Replaces the running program; only returns if it failed. */
static inline int execve(const char* path, char* const argv[],
                         char* const envp[])
{
    return dit_syscall(SYS_execve, (unsigned int)path, (unsigned int)argv,
                       (unsigned int)envp, 0, 0);
}

/* Query the current break with addr 0, or set it. Returns the resulting
 * break. */
static inline void* brk(void* addr)
{
    return (void*)(unsigned int)dit_syscall(SYS_brk, (unsigned int)addr, 0, 0,
                                            0, 0);
}

/* Anonymous private mapping; the kernel picks the address. */
static inline void* mmap(unsigned length, int prot, int flags)
{
    return (void*)(unsigned int)dit_syscall(
        SYS_mmap, 0, length, (unsigned int)prot, (unsigned int)flags, 0);
}

static inline int munmap(void* addr, unsigned length)
{
    return dit_syscall(SYS_munmap, (unsigned int)addr, length, 0, 0, 0);
}

#endif
