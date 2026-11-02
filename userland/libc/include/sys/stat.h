#ifndef USERLAND_LIBC_SYS_STAT_H
#define USERLAND_LIBC_SYS_STAT_H

// DIT-OS libc - file metadata (stat/fstat)
//
// struct stat mirrors kernel/include/fs/stat.hpp byte for byte. Both sides are
// built for i386, where 64-bit fields are 4-byte aligned, so the natural
// layout is the same on both sides of the syscall boundary.
//
// Like the other wrappers in syscall.h these return 0 on success or a negative
// errno, not -1 with errno set (there is no userland errno yet).

#include <syscall.h>

struct stat
{
    unsigned int st_dev;
    unsigned int st_ino;
    unsigned int st_mode;
    unsigned int st_nlink;
    unsigned int st_uid;
    unsigned int st_gid;
    unsigned int st_rdev;
    unsigned long long st_size;
    unsigned long long st_blksize;
    unsigned long long st_blocks;
    long long st_atime;
    long long st_mtime;
    long long st_ctime;
};

/* Mirrors fs::stat in kernel/include/fs/stat.hpp; both sides pin the size so
 * ABI drift fails the build instead of corrupting a user buffer. */
_Static_assert(sizeof(struct stat) == 76,
               "struct stat is part of the syscall ABI");

/* File type and mode bits (same values as the kernel). */
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

#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)

static inline int stat(const char* path, struct stat* st)
{
    return dit_syscall(SYS_stat, (unsigned int)path, (unsigned int)st, 0, 0, 0);
}

static inline int fstat(int fd, struct stat* st)
{
    return dit_syscall(SYS_fstat, (unsigned int)fd, (unsigned int)st, 0, 0, 0);
}

#endif // USERLAND_LIBC_SYS_STAT_H
