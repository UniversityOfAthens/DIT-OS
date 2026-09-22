// DIT-OS Kernel - Minimal VFS core
//
// This is intentionally small: it supports filesystem registration, mounting,
// and pathname lookup suitable for the first filesystem (ramfs).

#include "fs/vfs.hpp"
#include "fs/fs.hpp"
#include "fs/namei.hpp"
#include "fs/mount.hpp"
#include "fs/superblock.hpp"
#include "fs/inode.hpp"
#include "fs/dentry.hpp"
#include "fs/file.hpp"
#include "fs/stat.hpp"
#include "fs/ops.hpp"
#include "fs/dirent.hpp"

#include "core/panic.hpp"
#include "mm/kmalloc.hpp"
#include "errno_defs.hpp"

#include <stdint.h>

namespace fs
{

static int kstrcmp(const char* a, const char* b)
{
    if (a == b)
        return 0;
    if (!a || !b)
        return a ? 1 : -1;
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return (int)((unsigned char)*a) - (int)((unsigned char)*b);
}

static fs_type* s_fs_types = nullptr;
static mount* s_mounts = nullptr;
static mount* s_root_mount = nullptr;

static fs_type* find_fs_type(const char* name)
{
    for (fs_type* t = s_fs_types; t; t = t->next) {
        if (!t->name)
            continue;
        if (kstrcmp(t->name, name) == 0)
            return t;
    }
    return nullptr;
}

static dentry* alloc_dentry()
{
    dentry* d = (dentry*)mm::kmalloc(sizeof(dentry));
    if (!d)
        return nullptr;
    // Best-effort zero init (no bzero in freestanding).
    for (uint32_t i = 0; i < sizeof(dentry); i++)
        ((uint8_t*)d)[i] = 0;
    d->d_refcnt = 1;
    return d;
}

static inode* alloc_inode()
{
    inode* ino = (inode*)mm::kmalloc(sizeof(inode));
    if (!ino)
        return nullptr;
    for (uint32_t i = 0; i < sizeof(inode); i++)
        ((uint8_t*)ino)[i] = 0;
    return ino;
}

static inode* traverse_path(const char* path, dentry** out_dentry)
{
    if (!path || path[0] != '/')
        return nullptr;

    if (!s_root_mount || !s_root_mount->mnt_sb || !s_root_mount->mnt_root)
        return nullptr;

    inode* curr_ino = s_root_mount->mnt_root->d_inode;
    if (!curr_ino)
        return nullptr;

    // Root directory dentry we use as starting point.
    dentry* curr_d = alloc_dentry();
    if (!curr_d)
        return nullptr;
    // "/" for root.
    curr_d->d_name[0] = '/';
    curr_d->d_name[1] = '\0';
    curr_d->d_parent = nullptr;
    curr_d->d_inode = curr_ino;

    // Special-case "/" -> root.
    if (path[0] == '/' && path[1] == '\0') {
        if (out_dentry)
            *out_dentry = curr_d;
        return curr_ino;
    }

    const char* p = path;
    while (*p == '/')
        p++;

    // Each dentry owns one reference on its parent, so dentry_put() on the
    // leaf releases every ancestor that is no longer shared.
    for (;;) {
        char name[256];
        uint32_t n = 0;

        while (*p && *p != '/') {
            if (n == sizeof(name) - 1) {
                dentry_put(curr_d); // component too long (ENAMETOOLONG)
                return nullptr;
            }
            name[n++] = *p++;
        }
        name[n] = '\0';

        if (n == 0)
            break;

        /* Skip "."; treat ".." as parent directory (POSIX path semantics). */
        if (kstrcmp(name, ".") == 0) {
            while (*p == '/')
                p++;
            if (!*p)
                break;
            continue;
        }
        if (kstrcmp(name, "..") == 0) {
            if (curr_d->d_parent) {
                dentry* old = curr_d;
                curr_d = curr_d->d_parent;
                curr_d->d_refcnt++; // keep parent alive past dentry_put(old)
                curr_ino = curr_d->d_inode;
                dentry_put(old);
            }
            while (*p == '/')
                p++;
            if (!*p)
                break;
            continue;
        }

        if (!curr_ino->i_ops || !curr_ino->i_ops->lookup) {
            dentry_put(curr_d);
            return nullptr;
        }

        dentry* next_d = alloc_dentry();
        if (!next_d) {
            dentry_put(curr_d);
            return nullptr;
        }
        // Copy component name into dentry.
        for (uint32_t i = 0; i <= n; i++)
            next_d->d_name[i] = name[i];
        next_d->d_parent = curr_d; // takes over our reference on curr_d

        // Lookup child inode by name inside curr_ino's inode ops.
        int rc = curr_ino->i_ops->lookup(curr_ino, next_d);
        if (rc < 0 || !next_d->d_inode) {
            dentry_put(next_d); // also releases curr_d
            return nullptr;
        }

        curr_d = next_d;
        curr_ino = next_d->d_inode;

        while (*p == '/')
            p++;
        if (!*p)
            break;
    }

    if (out_dentry)
        *out_dentry = curr_d;
    else
        dentry_put(curr_d);

    return curr_ino;
}

inode* vfs_path_lookup(const char* path, dentry** out_dentry)
{
    return traverse_path(path, out_dentry);
}

file* vfs_open(const char* path, int flags)
{
    (void)flags;
    dentry* d = nullptr;
    inode* ino = vfs_path_lookup(path, &d);
    if (!ino || !d)
        return nullptr;

    file* f = (file*)mm::kmalloc(sizeof(file));
    if (!f) {
        dentry_put(d);
        return nullptr;
    }
    for (uint32_t i = 0; i < sizeof(file); i++)
        ((uint8_t*)f)[i] = 0;

    f->f_pos = 0;
    f->f_dentry = d;
    f->f_inode = ino;
    f->f_ops = ino->i_file_ops;
    if (f->f_ops && f->f_ops->open && f->f_ops->open(ino, f) < 0) {
        dentry_put(d);
        mm::kfree(f);
        return nullptr;
    }
    return f;
}

void vfs_close(file* f)
{
    if (!f)
        return;
    if (f->f_ops && f->f_ops->release)
        f->f_ops->release(f->f_inode, f);
    if (f->f_dentry)
        dentry_put(f->f_dentry);
    mm::kfree(f);
}

int vfs_read(file* f, void* buf, uint32_t size)
{
    if (!f || !f->f_ops || !f->f_ops->read)
        return -EINVAL;
    return f->f_ops->read(f, buf, size);
}

int vfs_write(file* f, const void* buf, uint32_t size)
{
    if (!f || !f->f_ops || !f->f_ops->write)
        return -EINVAL;
    return f->f_ops->write(f, buf, size);
}

int64_t vfs_lseek(file* f, int64_t offset, int whence)
{
    if (!f)
        return -EINVAL;
    const int SEEK_SET = 0;
    const int SEEK_CUR = 1;
    const int SEEK_END = 2;

    uint64_t new_pos = 0;
    if (whence == SEEK_SET) {
        new_pos = (uint64_t)offset;
    } else if (whence == SEEK_CUR) {
        new_pos = (uint64_t)((int64_t)f->f_pos + offset);
    } else if (whence == SEEK_END) {
        new_pos = (uint64_t)f->f_inode->i_size + (uint64_t)offset;
    } else {
        return -EINVAL;
    }
    f->f_pos = new_pos;
    return (int64_t)f->f_pos;
}

int namei(const char* path, uint32_t flags, dentry** out_dentry,
          inode** out_inode)
{
    (void)flags;
    if (!out_dentry || !out_inode)
        return -EINVAL;
    *out_dentry = nullptr;
    *out_inode = nullptr;
    inode* ino = vfs_path_lookup(path, out_dentry);
    if (!ino)
        return -ENOENT;
    *out_inode = ino;
    return 0;
}

void dentry_put(dentry* d)
{
    // Freeing a dentry drops the reference it holds on its parent, so walk up
    // the chain instead of recursing.
    while (d) {
        if (d->d_refcnt == 0)
            core::panic("dentry_put: refcount underflow");
        if (--d->d_refcnt > 0)
            return;
        dentry* parent = d->d_parent;
        mm::kfree(d);
        d = parent;
    }
}

void inode_put(inode* ino)
{
    (void)ino;
    // Inode lifetime is managed by the filesystem superblock in this minimal
    // VFS.
}

int vfs_readdir(file* f, dirent* de)
{
    if (!f || !de)
        return -EINVAL;
    if (!f->f_ops || !f->f_ops->readdir)
        return -EINVAL;
    return f->f_ops->readdir(f, de);
}

int vfs_stat(const char* path, struct stat* st)
{
    if (!path || !st)
        return -EINVAL;
    dentry* d = nullptr;
    inode* ino = vfs_path_lookup(path, &d);
    if (!ino || !d)
        return -ENOENT;

    st->st_ino = ino->i_ino;
    st->st_mode = ino->i_mode;
    st->st_nlink = ino->i_nlink;
    st->st_uid = ino->i_uid;
    st->st_gid = ino->i_gid;
    st->st_size = ino->i_size;
    st->st_blocks = ino->i_blocks;
    st->st_atime = ino->i_atime;
    st->st_mtime = ino->i_mtime;
    st->st_ctime = ino->i_ctime;
    st->st_dev = 0;
    st->st_rdev = 0;
    st->st_blksize = 0;

    dentry_put(d);
    return 0;
}

static void add_mount(mount* m)
{
    m->mnt_next = s_mounts;
    s_mounts = m;
}

int vfs_mount(const char* dev, const char* path, const char* fstype, void* data)
{
    (void)dev;
    (void)data;
    if (!path || path[0] != '/')
        return -EINVAL;

    fs_type* type = find_fs_type(fstype);
    if (!type || !type->get_sb)
        return -ENODEV;

    superblock* sb = nullptr;
    int rc = type->get_sb(type, dev, data, &sb);
    if (rc < 0 || !sb)
        return rc;

    dentry* mnt_point = alloc_dentry();
    if (!mnt_point)
        return -ENOMEM;
    // Mountpoint name: only "/" is supported right now.
    mnt_point->d_name[0] = path[0];
    mnt_point->d_name[1] = '\0';
    mnt_point->d_parent = nullptr;
    mnt_point->d_inode = sb->s_root;

    mount* m = (mount*)mm::kmalloc(sizeof(mount));
    if (!m)
        return -ENOMEM;
    for (uint32_t i = 0; i < sizeof(mount); i++)
        ((uint8_t*)m)[i] = 0;
    m->mnt_sb = sb;
    m->mnt_mountpoint = mnt_point;
    m->mnt_root = mnt_point;

    add_mount(m);
    if (path[0] == '/' && path[1] == '\0')
        s_root_mount = m;
    return 0;
}

int register_filesystem(fs_type* type)
{
    if (!type || !type->name)
        return -EINVAL;
    type->next = s_fs_types;
    s_fs_types = type;
    return 0;
}

int unregister_filesystem(fs_type* type)
{
    if (!type)
        return -EINVAL;
    fs_type** pp = &s_fs_types;
    while (*pp) {
        if (*pp == type) {
            *pp = type->next;
            type->next = nullptr;
            return 0;
        }
        pp = &(*pp)->next;
    }
    return -ENOENT;
}

} // namespace fs
