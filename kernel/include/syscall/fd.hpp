/** Per-process file descriptor tables. Every slot holds a refcounted open
 *  file: the console (fds 0-2 at boot), a VFS file or a pipe end. */
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
/** stdin, stdout and stderr: the console in the first process. */
constexpr int STDIO_FD_COUNT = 3;

/** Number of processes that can have open files at the same time. */
constexpr int FD_TABLE_COUNT = 16;

void fd_init();

/** The open file at @fd in the current process, or nullptr. */
fs::file* fd_get(int fd);

/** Install @f at the lowest free fd, or -EMFILE. Takes over the caller's
 *  reference on success. */
int fd_install(fs::file* f);

/** Close @fd: clear the slot and drop its reference. -EBADF if not open. */
int fd_close(int fd);

/** dup(): a new fd (the lowest free) sharing @fd's open file. Returns it,
 *  or -EBADF / -EMFILE. */
int fd_dup(int fd);

/** dup2(): make @newfd share @oldfd's open file, closing @newfd first if it
 *  was open. Returns @newfd, or -EBADF. @oldfd == @newfd only validates. */
int fd_dup2(int oldfd, int newfd);

/** Create an empty table for a new process; returns its index, or -1 when
 *  every table is in use. */
int fd_table_create();

/** Put a console file on fds 0-2 of @table (the first process's stdio).
 *  Returns false when out of memory. */
bool fd_table_install_stdio(int table);

/** Close every file in @table and mark it free. */
void fd_table_release(int table);

/** Copy @src's open files into a new table for the calling process. The file
 *  objects are shared with @src, so the two processes also share the read
 *  offset of a file. Returns the new table index, or -1. */
int fd_table_dup(int src);

} // namespace syscall

#endif
