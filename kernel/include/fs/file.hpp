#ifndef KERNEL_FS_FILE_HPP
#define KERNEL_FS_FILE_HPP

#include <stdint.h>
#include "fs/ops.hpp"

namespace fs
{

struct inode;
struct dentry;

/** Open file: one per fd, links to dentry/inode and holds position */
struct file
{
    uint32_t f_flags;
    uint64_t f_pos;
    dentry* f_dentry;
    inode* f_inode;
    void* f_private; /* fs/driver-specific */
    FileOps* f_ops;
};

} // namespace fs

#endif
