#ifndef KERNEL_FS_NAMEI_HPP
#define KERNEL_FS_NAMEI_HPP

#include <stdint.h>

namespace fs
{

struct dentry;
struct inode;

/** Pathname lookup flags */
#define LOOKUP_FOLLOW 0x01 /* follow trailing symlinks */
#define LOOKUP_DIRECTORY 0x02 /* must be directory */
#define LOOKUP_NOFOLLOW 0x04 /* do not follow symlinks */

/**
 * namei: resolve path to dentry (and inode).
 * Returns 0 on success, negative errno on error.
 * *out_dentry and *out_inode set on success; caller must release refs.
 */
int namei(const char* path, uint32_t flags, dentry** out_dentry,
          inode** out_inode);

/** Release dentry reference */
void dentry_put(dentry* d);
/** Release inode reference */
void inode_put(inode* ino);

} // namespace fs

#endif
