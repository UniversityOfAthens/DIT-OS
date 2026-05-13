#ifndef KERNEL_FS_BUFFER_HPP
#define KERNEL_FS_BUFFER_HPP

#include <stdint.h>

namespace fs
{

/** Block buffer (buffer cache): one block of file system data in memory */
struct buffer_head {
    uint8_t* b_data;
    uint64_t b_blocknr; /* block number on device */
    uint32_t b_size; /* block size */
    uint32_t b_refcnt;
    uint8_t b_dirty;
    uint8_t b_uptodate;
    void* b_dev_private;
};

/** Get a buffer for block blocknr on device; returns buffer_head* or nullptr.
 * Call buffer_release when done */
buffer_head* buffer_get(void* dev, uint64_t blocknr, uint32_t block_size);
/** Mark buffer dirty (to be written back) */
void buffer_mark_dirty(buffer_head* bh);
/** Release reference on buffer */
void buffer_release(buffer_head* bh);
/** Read block from device into buffer; returns 0 or negative errno */
int buffer_read(buffer_head* bh);
/** Write buffer to device; returns 0 or negative errno */
int buffer_write(buffer_head* bh);

} // namespace fs

#endif
