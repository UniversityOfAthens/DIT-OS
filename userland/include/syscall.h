/* DIT-OS Userland - Syscall ABI (temporary)
 *
 * NOTE: int $0x80 path is currently being debugged in the kernel.
 * For booting and userland shell bring-up, we route read/write/exit/getpid
 * through temporary C ABI wrappers exported by the kernel:
 *   - dit_sys_read/write/exit/getpid
 */
#ifndef USERLAND_SYSCALL_H
#define USERLAND_SYSCALL_H

#define SYS_read   0
#define SYS_write  1
#define SYS_exit  6
#define SYS_getpid 10

// Kernel-exported C ABI wrappers (implemented in kernel build)
#ifdef __cplusplus
extern "C" {
#endif
int dit_sys_read(int fd, void* buf, unsigned count);
int dit_sys_write(int fd, const void* buf, unsigned count);
void dit_sys_exit(int code);
int dit_sys_getpid(void);
#ifdef __cplusplus
}
#endif

static inline int read(int fd, void* buf, unsigned count) {
    (void)SYS_read;
    return dit_sys_read(fd, buf, count);
}
static inline int write(int fd, const void* buf, unsigned count) {
    (void)SYS_write;
    return dit_sys_write(fd, buf, count);
}
static inline void exit(int code) {
    (void)SYS_exit;
    dit_sys_exit(code);
    __builtin_unreachable();
}

static inline int getpid(void) {
    (void)SYS_getpid;
    return dit_sys_getpid();
}

#endif
