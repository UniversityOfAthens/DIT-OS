#ifndef KERNEL_FS_DIRENT_HPP
#define KERNEL_FS_DIRENT_HPP

#include <stdint.h>

namespace fs {

/** Maximum name length in struct dirent */
#define NAME_MAX 256

struct dirent {
    uint32_t d_ino;
    uint8_t  d_type;   /* DT_REG, DT_DIR, etc. */
    char     d_name[NAME_MAX];
};

#define DT_UNKNOWN 0
#define DT_FIFO    1
#define DT_CHR     2
#define DT_DIR     4
#define DT_BLK     6
#define DT_REG     8
#define DT_LNK     10
#define DT_SOCK    12

} // namespace fs

#endif
