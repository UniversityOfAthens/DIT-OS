#ifndef KERNEL_FS_SUPERBLOCK_HPP
#define KERNEL_FS_SUPERBLOCK_HPP

#include <stdint.h>
#include "fs/ops.hpp"

namespace fs {

struct inode;

/** Superblock: per-mounted-filesystem metadata */
struct superblock {
    uint32_t    s_blocksize;
    uint64_t    s_blocks;
    uint64_t    s_free_blocks;
    uint64_t    s_free_inodes;
    inode*      s_root;     /* root inode of this fs */
    void*       s_fs_info;  /* fs-specific (e.g. ext2 super block) */
    SuperblockOps* s_ops;
};

} // namespace fs

#endif
