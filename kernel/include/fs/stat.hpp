#ifndef KERNEL_FS_STAT_HPP
#define KERNEL_FS_STAT_HPP

#include <stdint.h>

namespace fs
{

/** File type and mode bits (Unix-style) */
using mode_t = uint32_t;

#define S_IFMT 0170000
#define S_IFREG 0100000
#define S_IFDIR 0040000
#define S_IFCHR 0020000
#define S_IFBLK 0060000
#define S_IFIFO 0010000
#define S_IFLNK 0120000
#define S_IFSOCK 0140000

#define S_IRWXU 0000700
#define S_IRUSR 0000400
#define S_IWUSR 0000200
#define S_IXUSR 0000100
#define S_IRWXG 0000070
#define S_IRGRP 0000040
#define S_IWGRP 0000020
#define S_IXGRP 0000010
#define S_IRWXO 0000007
#define S_IROTH 0000004
#define S_IWOTH 0000002
#define S_IXOTH 0000001

struct stat {
    uint32_t st_dev;
    uint32_t st_ino;
    mode_t st_mode;
    uint32_t st_nlink;
    uint32_t st_uid;
    uint32_t st_gid;
    uint32_t st_rdev;
    uint64_t st_size;
    uint64_t st_blksize;
    uint64_t st_blocks;
    int64_t st_atime;
    int64_t st_mtime;
    int64_t st_ctime;
};

} // namespace fs

#endif
