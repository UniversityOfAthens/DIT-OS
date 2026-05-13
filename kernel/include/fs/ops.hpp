// DIT-OS Kernel - VFS operation tables (minimal)

#ifndef KERNEL_FS_OPS_HPP
#define KERNEL_FS_OPS_HPP

#include <stdint.h>

namespace fs
{

struct inode;
struct dentry;
struct file;
struct superblock;

struct dirent;

struct SuperblockOps {
    inode* (*alloc_inode)(superblock*);
    void (*destroy_inode)(inode*);
    int (*sync)(superblock*);
};

struct InodeOps {
    /** Lookup child by name inside @dir; on success sets target->d_inode. */
    int (*lookup)(inode* dir, dentry* target);

    /** Create a filesystem object (regular file for now). */
    int (*create)(inode* dir, dentry* target, int mode);

    /** Create directory. */
    int (*mkdir)(inode* dir, dentry* target, int mode);

    /** Remove directory entry. */
    int (*unlink)(inode* dir, dentry* target);
};

struct FileOps {
    /** Optional open hook. */
    int (*open)(inode* ino, file* f);

    /** Read from file at f->f_pos; must update f_pos. */
    int (*read)(file* f, void* buf, uint32_t size);

    /** Write to file at f->f_pos; must update f_pos. */
    int (*write)(file* f, const void* buf, uint32_t size);

    /** Optional release hook. */
    int (*release)(inode* ino, file* f);

    /** Read one directory entry; returns 1 if @de filled, 0 at EOF, negative
     * errno. */
    int (*readdir)(file* f, dirent* de);
};

} // namespace fs

#endif
