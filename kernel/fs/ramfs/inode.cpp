// DIT-OS Kernel - ramfs inode helpers

#include "fs/inode.hpp"
#include "fs/superblock.hpp"
#include "fs/stat.hpp"

#include "mm/kmalloc.hpp"

#include <stdint.h>

namespace ramfs
{

struct ramfs_entry
{
    char name[256];
    fs::inode* ino;
    ramfs_entry* next;
};

struct ramfs_dir
{
    ramfs_entry* entries;
};

struct ramfs_file
{
    char* data;
    uint32_t size;
    uint32_t cap;
};

static uint32_t s_next_ino = 1;

static ramfs_dir* dir_alloc()
{
    ramfs_dir* d = (ramfs_dir*)mm::kmalloc(sizeof(ramfs_dir));
    if (!d)
        return nullptr;
    d->entries = nullptr;
    return d;
}

static ramfs_file* file_alloc()
{
    ramfs_file* f = (ramfs_file*)mm::kmalloc(sizeof(ramfs_file));
    if (!f)
        return nullptr;
    f->data = nullptr;
    f->size = 0;
    f->cap = 0;
    return f;
}

fs::inode* inode_create_dir(fs::superblock* sb)
{
    fs::inode* ino = (fs::inode*)mm::kmalloc(sizeof(fs::inode));
    if (!ino)
        return nullptr;
    for (uint32_t i = 0; i < sizeof(fs::inode); i++)
        ((uint8_t*)ino)[i] = 0;

    ino->i_ino = s_next_ino++;
    ino->i_mode = S_IFDIR | S_IRWXU;
    ino->i_nlink = 1;
    ino->i_uid = 0;
    ino->i_gid = 0;
    ino->i_size = 0;
    ino->i_blocks = 0;
    ino->i_atime = 0;
    ino->i_mtime = 0;
    ino->i_ctime = 0;
    ino->i_sb = sb;
    ino->i_private = dir_alloc();
    ino->i_ops = nullptr;
    ino->i_file_ops = nullptr;
    return ino;
}

fs::inode* inode_create_file(fs::superblock* sb)
{
    fs::inode* ino = (fs::inode*)mm::kmalloc(sizeof(fs::inode));
    if (!ino)
        return nullptr;
    for (uint32_t i = 0; i < sizeof(fs::inode); i++)
        ((uint8_t*)ino)[i] = 0;

    ino->i_ino = s_next_ino++;
    ino->i_mode = S_IFREG | S_IRWXU;
    ino->i_nlink = 1;
    ino->i_uid = 0;
    ino->i_gid = 0;
    ino->i_size = 0;
    ino->i_blocks = 0;
    ino->i_atime = 0;
    ino->i_mtime = 0;
    ino->i_ctime = 0;
    ino->i_sb = sb;
    ino->i_private = file_alloc();
    ino->i_ops = nullptr;
    ino->i_file_ops = nullptr;
    return ino;
}

} // namespace ramfs
