// DIT-OS Kernel - Minimal shell (runs as init task; uses syscalls for
// read/write/exit)

#include "syscall/syscall.hpp"
#include "dev/serial.hpp"
#include <stdint.h>

namespace sched
{

static int sys_read(int fd, void* buf, unsigned count)
{
    return syscall::syscall_read(fd, buf, count);
}
static int sys_write(int fd, const void* buf, unsigned count)
{
    return syscall::syscall_write(fd, buf, count);
}
static void sys_exit(int code)
{
    syscall::syscall_exit(code);
}

static void shell_main()
{
    /* Prove we reached the init task (in case syscall path fails) */
    dev::serial_puts("\r\n[init] shell started\r\n");
    const char* prompt = "\r\nDIT-OS shell> ";
    char buf[80];
    sys_write(1, prompt, __builtin_strlen(prompt));
    for (;;) {
        int n = sys_read(0, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            sys_write(1, buf, n);
            if (n >= 4 && buf[0] == 'e' && buf[1] == 'x' && buf[2] == 'i' &&
                buf[3] == 't') {
                sys_write(1, "\r\nBye.\r\n", 8);
                sys_exit(0);
            }
            sys_write(1, prompt, __builtin_strlen(prompt));
        }
        /* No input (EAGAIN): yield so we don't spam prompt */
        asm volatile("sti; hlt");
    }
}

extern "C" void userland_shell_main();

void (*get_shell_entry())()
{
    return userland_shell_main;
}

} // namespace sched
