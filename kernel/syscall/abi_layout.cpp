// DIT-OS Kernel - compile-time syscall ABI layout checks
//
// The only translation unit that includes both the kernel and userland
// definitions of the structures that cross int 0x80. Any size or field
// offset drift fails the build instead of corrupting a user buffer at
// runtime.

#include "fs/dirent.hpp"
#include "fs/stat.hpp"
#include "syscall.h"
#include "sys/stat.h"
#include <stddef.h>

#define ABI_OFFSET(kernel_type, user_type, field)                             \
    static_assert(offsetof(kernel_type, field) == offsetof(user_type, field), \
                  #field " offset differs between kernel and userland")

static_assert(sizeof(fs::dirent) == sizeof(struct dit_dirent),
              "struct dirent size differs between kernel and userland");
ABI_OFFSET(fs::dirent, struct dit_dirent, d_ino);
ABI_OFFSET(fs::dirent, struct dit_dirent, d_type);
ABI_OFFSET(fs::dirent, struct dit_dirent, d_pad);
ABI_OFFSET(fs::dirent, struct dit_dirent, d_name);

static_assert(sizeof(fs::stat) == sizeof(struct stat),
              "struct stat size differs between kernel and userland");
ABI_OFFSET(fs::stat, struct stat, st_dev);
ABI_OFFSET(fs::stat, struct stat, st_ino);
ABI_OFFSET(fs::stat, struct stat, st_mode);
ABI_OFFSET(fs::stat, struct stat, st_nlink);
ABI_OFFSET(fs::stat, struct stat, st_uid);
ABI_OFFSET(fs::stat, struct stat, st_gid);
ABI_OFFSET(fs::stat, struct stat, st_rdev);
ABI_OFFSET(fs::stat, struct stat, st_size);
ABI_OFFSET(fs::stat, struct stat, st_blksize);
ABI_OFFSET(fs::stat, struct stat, st_blocks);
ABI_OFFSET(fs::stat, struct stat, st_atime);
ABI_OFFSET(fs::stat, struct stat, st_mtime);
ABI_OFFSET(fs::stat, struct stat, st_ctime);

#undef ABI_OFFSET
