#ifndef KERNEL_FS_FS_HPP
#define KERNEL_FS_FS_HPP

#include <stdint.h>

namespace fs {

struct superblock;
struct inode;
struct dentry;
struct file;

/** Filesystem type: name and ops to mount/read superblock */
struct fs_type {
    const char*     name;
    int             (*get_sb)(fs_type* type, const char* dev, void* data, superblock** out);
    void            (*kill_sb)(superblock* sb);
    fs_type*        next;
};

/** Register a filesystem type */
int register_filesystem(fs_type* type);
/** Unregister a filesystem type */
int unregister_filesystem(fs_type* type);

} // namespace fs

#endif
