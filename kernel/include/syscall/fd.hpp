/** Per-process file descriptor tables (fds >= 3 are VFS files). */
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

/** Number of processes that can have open files at the same time. */
constexpr int FD_TABLE_COUNT = 16;

void fd_init();

fs::file* fd_get(int fd);

/** Install @f at lowest free fd >= FIRST_FILE_FD, or -EMFILE. */
int fd_install(fs::file* f);

/** Close VFS file and clear slot; EBADF if not an open VFS fd. */
int fd_close(int fd);

/** Create an empty table for a new process; returns its index, or -1 when
 *  every table is in use. */
int fd_table_create();

/** Close every file in @table and mark it free. */
void fd_table_release(int table);

/** Copy @src's open files into a new table for the calling process. The file
 *  objects are shared with @src, so the two processes also share the read
 *  offset of a file. Returns the new table index, or -1. */
int fd_table_dup(int src);

} // namespace syscall

#endif
