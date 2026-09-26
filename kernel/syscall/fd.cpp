// DIT-OS Kernel - Per-process file descriptor tables

#include "syscall/fd.hpp"
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

void fd_table_release(int table)
{
    if (table < 0 || table >= FD_TABLE_COUNT || !s_table_used[table])
        return;
    for (int i = FIRST_FILE_FD; i < FD_TABLE_SIZE; i++)
    {
        if (!s_tables[table][i])
            continue;
        /* Drops one reference; the file goes away with the last holder. */
        fs::vfs_close(s_tables[table][i]);
        s_tables[table][i] = nullptr;
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
    for (int i = FIRST_FILE_FD; i < FD_TABLE_SIZE; i++)
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
    if (fd < FIRST_FILE_FD || fd >= FD_TABLE_SIZE)
        return nullptr;
    return s_tables[current_table()][fd];
}

int fd_install(fs::file* f)
{
    if (!f)
        return -EINVAL;
    fs::file** table = s_tables[current_table()];
    for (int i = FIRST_FILE_FD; i < FD_TABLE_SIZE; i++)
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
    if (fd < FIRST_FILE_FD || fd >= FD_TABLE_SIZE || !table[fd])
        return -EBADF;
    table[fd] = nullptr;
    fs::vfs_close(table[fd]);
    return 0;
}

} // namespace syscall
