#ifndef KERNEL_FS_INODE_HPP
#define KERNEL_FS_INODE_HPP

#include <stdint.h>
#include "fs/stat.hpp"
#include "fs/ops.hpp"

namespace fs
{

struct superblock;

/** Inode: represents a file or directory on disk (or in-memory view) */
struct inode {
    uint32_t i_ino;
    mode_t i_mode;
    uint32_t i_nlink;
    uint32_t i_uid;
    uint32_t i_gid;
    uint64_t i_size;
    uint64_t i_blocks;
    int64_t i_atime;
    int64_t i_mtime;
    int64_t i_ctime;
    superblock* i_sb;
    void* i_private; /* fs-specific data */
    InodeOps* i_ops;
    FileOps* i_file_ops;
};

} // namespace fs

#endif
