// DIT-OS Kernel - Per-process file descriptor tables

#include "syscall/fd.hpp"
#include "syscall/tty.hpp"
#include "fs/file.hpp"
#include "fs/vfs.hpp"
#include "sched/sched.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

/* Table 0 belongs to the kernel itself (before any task exists). */
static fs::file* s_tables[FD_TABLE_COUNT][FD_TABLE_SIZE];
static bool s_table_used[FD_TABLE_COUNT];

static int current_table()
{
    if (sched::current_task && sched::current_task->fd_table >= 0)
        return sched::current_task->fd_table;
    return 0;
}

static bool fd_in_range(int fd)
{
    return fd >= 0 && fd < FD_TABLE_SIZE;
}

void fd_init()
{
    for (int t = 0; t < FD_TABLE_COUNT; t++)
    {
        s_table_used[t] = (t == 0);
        for (int i = 0; i < FD_TABLE_SIZE; i++)
            s_tables[t][i] = nullptr;
    }
}

int fd_table_create()
{
    for (int t = 0; t < FD_TABLE_COUNT; t++)
    {
        if (s_table_used[t])
            continue;
        s_table_used[t] = true;
        for (int i = 0; i < FD_TABLE_SIZE; i++)
            s_tables[t][i] = nullptr;
        return t;
    }
    return -1;
}

bool fd_table_install_stdio(int table)
{
    if (table < 0 || table >= FD_TABLE_COUNT || !s_table_used[table])
        return false;
    /* One open console shared by all three, like a login terminal. */
    fs::file* console = tty_open_console();
    if (!console)
        return false;
    for (int i = 0; i < STDIO_FD_COUNT; i++)
    {
        if (i > 0)
            console->f_refcount++;
        s_tables[table][i] = console;
    }
    return true;
}

void fd_table_release(int table)
{
    if (table < 0 || table >= FD_TABLE_COUNT || !s_table_used[table])
        return;
    for (int i = 0; i < FD_TABLE_SIZE; i++)
    {
        fs::file* f = s_tables[table][i];
        if (!f)
            continue;
        s_tables[table][i] = nullptr;
        /* Drops one reference; the file goes away with the last holder. */
        fs::vfs_close(f);
    }
    s_table_used[table] = false;
}

int fd_table_dup(int src)
{
    if (src < 0 || src >= FD_TABLE_COUNT || !s_table_used[src])
        return -1;
    int dst = fd_table_create();
    if (dst < 0)
        return -1;
    for (int i = 0; i < FD_TABLE_SIZE; i++)
    {
        if (!s_tables[src][i])
            continue;
        s_tables[src][i]->f_refcount++;
        s_tables[dst][i] = s_tables[src][i];
    }
    return dst;
}

fs::file* fd_get(int fd)
{
    if (!fd_in_range(fd))
        return nullptr;
    return s_tables[current_table()][fd];
}

int fd_install(fs::file* f)
{
    if (!f)
        return -EINVAL;
    fs::file** table = s_tables[current_table()];
    for (int i = 0; i < FD_TABLE_SIZE; i++)
    {
        if (table[i] == nullptr)
        {
            table[i] = f;
            return i;
        }
    }
    return -EMFILE;
}

int fd_close(int fd)
{
    fs::file** table = s_tables[current_table()];
    if (!fd_in_range(fd) || !table[fd])
        return -EBADF;
    /* Unlink first, then drop the reference of the file that was there. */
    fs::file* f = table[fd];
    table[fd] = nullptr;
    fs::vfs_close(f);
    return 0;
}

int fd_dup(int fd)
{
    fs::file* f = fd_get(fd);
    if (!f)
        return -EBADF;
    int newfd = fd_install(f);
    if (newfd >= 0)
        f->f_refcount++;
    return newfd;
}

int fd_dup2(int oldfd, int newfd)
{
    fs::file* f = fd_get(oldfd);
    if (!f || !fd_in_range(newfd))
        return -EBADF;
    if (oldfd == newfd)
        return newfd;
    fs::file** table = s_tables[current_table()];
    /* Take the new reference before dropping the old file, and install it
     * before closing, so the slot is never observed empty or dangling. */
    f->f_refcount++;
    fs::file* previous = table[newfd];
    table[newfd] = f;
    if (previous)
        fs::vfs_close(previous);
    return newfd;
}

} // namespace syscall
