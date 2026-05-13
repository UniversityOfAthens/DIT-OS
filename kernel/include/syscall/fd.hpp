/** Per-process file descriptor table (single address space; fds >= 3 are VFS
 * files). */
#ifndef KERNEL_SYSCALL_FD_HPP
#define KERNEL_SYSCALL_FD_HPP

#include <stdint.h>

namespace fs
{
struct file;
}

namespace syscall
{

constexpr int FD_TABLE_SIZE = 32;
constexpr int FIRST_FILE_FD = 3;

void fd_init();

fs::file* fd_get(int fd);

/** Install @f at lowest free fd >= FIRST_FILE_FD, or -EMFILE. */
int fd_install(fs::file* f);

/** Close VFS file and clear slot; EBADF if not an open VFS fd. */
int fd_close(int fd);

} // namespace syscall

#endif
