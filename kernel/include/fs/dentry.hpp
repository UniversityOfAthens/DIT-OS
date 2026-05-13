#ifndef KERNEL_FS_DENTRY_HPP
#define KERNEL_FS_DENTRY_HPP

#include <stdint.h>

namespace fs
{

struct inode;

/** Dentry: directory entry (name -> inode), part of path lookup cache */
struct dentry {
    char d_name[256];
    inode* d_inode;
    dentry* d_parent;
    dentry* d_child; /* first child; siblings via d_sibling */
    dentry* d_sibling;
    uint32_t d_refcnt;
};

} // namespace fs

#endif
