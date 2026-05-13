#ifndef KERNEL_FS_VFS_HPP
#define KERNEL_FS_VFS_HPP

#include <stdint.h>
#include "fs/inode.hpp"
#include "fs/dentry.hpp"
#include "fs/file.hpp"
#include "fs/superblock.hpp"
#include "fs/mount.hpp"
#include "fs/stat.hpp"
#include "fs/fcntl.hpp"

namespace fs
{

struct dirent;

/** High-level VFS operations (implemented by VFS layer using inode/sb ops) */

/** Open by path; returns file* or nullptr on error (errno set) */
file* vfs_open(const char* path, int flags);
/** Close and release file */
void vfs_close(file* f);
/** Read from file; return bytes read or negative errno */
int vfs_read(file* f, void* buf, uint32_t size);
/** Write to file; return bytes written or negative errno */
int vfs_write(file* f, const void* buf, uint32_t size);
/** Seek; return new position or negative errno */
int64_t vfs_lseek(file* f, int64_t offset, int whence);

/** Get inode for path; caller must release inode */
inode* vfs_path_lookup(const char* path, dentry** out_dentry);
/** Stat by path */
int vfs_stat(const char* path, struct stat* st);

/** Read one directory entry (directories only); see FileOps::readdir */
int vfs_readdir(file* f, dirent* de);
/** Mount a filesystem */
int vfs_mount(const char* dev, const char* path, const char* fstype,
              void* data);

} // namespace fs

#endif
