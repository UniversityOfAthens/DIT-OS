#ifndef KERNEL_FS_MOUNT_HPP
#define KERNEL_FS_MOUNT_HPP

#include <stdint.h>

namespace fs
{

struct superblock;
struct dentry;

/** Mount: one per mounted filesystem */
struct mount
{
    superblock* mnt_sb;
    dentry* mnt_mountpoint; /* dentry where this is mounted */
    dentry* mnt_root; /* root of this fs */
    mount* mnt_next;
    uint32_t mnt_flags;
};

} // namespace fs

#endif
