// DIT-OS Kernel - sys_read stub (fd 0/1/2 -> serial for now)

#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include "syscall/fd.hpp"
#include "fs/vfs.hpp"
#include "dev/serial.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall {

extern SyscallResult sys_read_impl(const SyscallArgs* args);

SyscallResult sys_read_impl(const SyscallArgs* args) {
    int fd = (int)args->args[0];
    uintptr_t buf = (uintptr_t)args->args[1];
    uint32_t count = (uint32_t)args->args[2];
    if (count == 0) return 0;
    if (fd == 0) {
        /* stdin: blocking read from serial with immediate local echo */
        int n = 0;
        char* p = (char*)buf;
        while (n < (int)count) {
            int c = dev::serial_getchar(0);
            if (c < 0) {
                __asm__ volatile("pause");
                continue;
            }

            char ch = (char)(c & 0xFF);

            /* Basic line editing for backspace/delete */
            if (ch == '\b' || ch == 0x7F) {
                if (n > 0) {
                    n--;
                    dev::serial_putchar('\b');
                    dev::serial_putchar(' ');
                    dev::serial_putchar('\b');
                }
                continue;
            }

            if (ch == '\r' || ch == '\n') {
                if (n < (int)count) p[n++] = '\n';
                dev::serial_putchar('\r');
                dev::serial_putchar('\n');
                break;
            }

            p[n++] = ch;
            dev::serial_putchar(ch);
        }
        return n;
    }
    fs::file* vf = fd_get(fd);
    if (vf) return fs::vfs_read(vf, (void*)buf, count);
    return -EBADF;
}

} // namespace syscall
