#ifndef KERNEL_DEV_BLOCKDEV_HPP
#define KERNEL_DEV_BLOCKDEV_HPP

#include <stdint.h>

namespace dev
{

/** Block device interface: fixed block size, read/write by block index */
struct BlockDev {
    /** Block size in bytes (e.g. 512) */
    uint32_t block_size;
    /** Total number of blocks */
    uint64_t num_blocks;
    /** Read one or more blocks. Return 0 on success, negative on error. */
    int (*read)(BlockDev* bdev, uint64_t block, uint32_t count, void* buf);
    /** Write one or more blocks. Return 0 on success, negative on error. */
    int (*write)(BlockDev* bdev, uint64_t block, uint32_t count,
                 const void* buf);
    void* driver_data;
};

} // namespace dev

#endif
