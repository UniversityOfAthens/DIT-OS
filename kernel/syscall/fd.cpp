// DIT-OS Kernel - File descriptor table for syscalls

#include "syscall/fd.hpp"
#include "fs/vfs.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

static fs::file* g_fd[FD_TABLE_SIZE];

void fd_init()
{
    for (int i = 0; i < FD_TABLE_SIZE; i++)
        g_fd[i] = nullptr;
}

fs::file* fd_get(int fd)
{
    if (fd < FIRST_FILE_FD || fd >= FD_TABLE_SIZE)
        return nullptr;
    return g_fd[fd];
}

int fd_install(fs::file* f)
{
    if (!f)
        return -EINVAL;
    for (int i = FIRST_FILE_FD; i < FD_TABLE_SIZE; i++)
    {
        if (g_fd[i] == nullptr)
        {
            g_fd[i] = f;
            return i;
        }
    }
    return -EMFILE;
}

int fd_close(int fd)
{
    fs::file* f = fd_get(fd);
    if (!f)
        return -EBADF;
    fs::vfs_close(f);
    g_fd[fd] = nullptr;
    return 0;
}

} // namespace syscall
