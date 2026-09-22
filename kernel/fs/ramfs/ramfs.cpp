// DIT-OS Kernel - RAM filesystem (minimal)
//
// This implements just enough VFS integration to provide a working root.

#include "ramfs.hpp"

#include "fs/fs.hpp"
#include "fs/vfs.hpp"
#include "fs/superblock.hpp"
#include "fs/inode.hpp"
#include "fs/dentry.hpp"
#include "fs/file.hpp"
#include "fs/stat.hpp"
#include "fs/ops.hpp"
#include "fs/dirent.hpp"

#include "mm/kmalloc.hpp"

#include "errno_defs.hpp"

#include <stdint.h>

namespace ramfs
{

// Helpers from inode.cpp
fs::inode* inode_create_dir(fs::superblock* sb);
fs::inode* inode_create_file(fs::superblock* sb);

struct ramfs_entry {
    char name[256];
    fs::inode* ino;
    ramfs_entry* next;
};

struct ramfs_dir {
    ramfs_entry* entries;
};

struct ramfs_file {
    char* data;
    uint32_t size;
    uint32_t cap;
};

static uint32_t s_max_file_cap = 64 * 1024; /* simple cap for now */

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

static void kmemcpy(void* dest, const void* src, uint32_t n)
{
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (uint32_t i = 0; i < n; i++)
        d[i] = s[i];
}

static fs::SuperblockOps s_sb_ops;
static fs::InodeOps s_inode_ops;
static fs::FileOps s_file_ops;

static int inode_lookup(fs::inode* dir, fs::dentry* target)
{
    if (!dir || !target)
        return -EINVAL;
    if ((dir->i_mode & S_IFMT) != S_IFDIR)
        return -ENOTDIR;

    ramfs_dir* d = (ramfs_dir*)dir->i_private;
    if (!d)
        return -ENOENT;

    for (ramfs_entry* e = d->entries; e; e = e->next) {
        if (kstrcmp(e->name, target->d_name) == 0) {
            target->d_inode = e->ino;
            return 0;
        }
    }
    return -ENOENT;
}

static int inode_create(fs::inode* dir, fs::dentry* target, int mode)
{
    (void)mode; /* currently create() always makes a regular file */
    if (!dir || !target)
        return -EINVAL;
    if ((dir->i_mode & S_IFMT) != S_IFDIR)
        return -ENOTDIR;

    ramfs_dir* d = (ramfs_dir*)dir->i_private;
    if (!d)
        return -ENOENT;

    // Disallow duplicates.
    for (ramfs_entry* e = d->entries; e; e = e->next) {
        if (kstrcmp(e->name, target->d_name) == 0)
            return -EEXIST;
    }

    fs::inode* ino = inode_create_file(dir->i_sb);
    if (!ino)
        return -ENOMEM;
    ino->i_ops = &s_inode_ops;
    ino->i_file_ops = &s_file_ops;

    ramfs_entry* ne = (ramfs_entry*)mm::kmalloc(sizeof(ramfs_entry));
    if (!ne)
        return -ENOMEM;
    for (uint32_t i = 0; i < sizeof(ramfs_entry); i++)
        ((uint8_t*)ne)[i] = 0;

    for (uint32_t i = 0; i < 256 && target->d_name[i]; i++)
        ne->name[i] = target->d_name[i];
    ne->name[255] = '\0';
    ne->ino = ino;
    ne->next = d->entries;
    d->entries = ne;

    target->d_inode = ino;
    return 0;
}

static int inode_mkdir(fs::inode* dir, fs::dentry* target, int mode)
{
    (void)mode; /* currently create mkdir without special permissions */
    if (!dir || !target)
        return -EINVAL;
    if ((dir->i_mode & S_IFMT) != S_IFDIR)
        return -ENOTDIR;

    ramfs_dir* d = (ramfs_dir*)dir->i_private;
    if (!d)
        return -ENOENT;

    for (ramfs_entry* e = d->entries; e; e = e->next) {
        if (kstrcmp(e->name, target->d_name) == 0)
            return -EEXIST;
    }

    fs::inode* ino = inode_create_dir(dir->i_sb);
    if (!ino)
        return -ENOMEM;
    ino->i_ops = &s_inode_ops;
    ino->i_file_ops = &s_file_ops;

    ramfs_entry* ne = (ramfs_entry*)mm::kmalloc(sizeof(ramfs_entry));
    if (!ne)
        return -ENOMEM;
    for (uint32_t i = 0; i < sizeof(ramfs_entry); i++)
        ((uint8_t*)ne)[i] = 0;

    for (uint32_t i = 0; i < 256 && target->d_name[i]; i++)
        ne->name[i] = target->d_name[i];
    ne->name[255] = '\0';
    ne->ino = ino;
    ne->next = d->entries;
    d->entries = ne;

    target->d_inode = ino;
    return 0;
}

static int inode_unlink(fs::inode* dir, fs::dentry* target)
{
    if (!dir || !target)
        return -EINVAL;
    if ((dir->i_mode & S_IFMT) != S_IFDIR)
        return -ENOTDIR;

    ramfs_dir* d = (ramfs_dir*)dir->i_private;
    if (!d)
        return -ENOENT;

    ramfs_entry** pp = &d->entries;
    while (*pp) {
        ramfs_entry* e = *pp;
        if (kstrcmp(e->name, target->d_name) == 0) {
            *pp = e->next;
            /* We intentionally do not free the inode/file data in this minimal
             * implementation. */
            mm::kfree(e);
            return 0;
        }
        pp = &(*pp)->next;
    }
    return -ENOENT;
}

static int file_open(fs::inode* ino, fs::file* f)
{
    (void)ino;
    if (!f)
        return -EINVAL;
    f->f_pos = 0;
    return 0;
}

static int file_read(fs::file* f, void* buf, uint32_t size)
{
    if (!f || !buf)
        return -EINVAL;
    if (!f->f_inode)
        return -EBADF;
    if ((f->f_inode->i_mode & S_IFMT) != S_IFREG)
        return -EISDIR;

    ramfs_file* rf = (ramfs_file*)f->f_inode->i_private;
    if (!rf)
        return -EINVAL;

    if (f->f_pos >= rf->size)
        return 0;
    uint32_t avail = rf->size - (uint32_t)f->f_pos;
    uint32_t n = (size < avail) ? size : avail;

    if (n)
        kmemcpy(buf, rf->data + f->f_pos, n);
    f->f_pos += n;
    return (int)n;
}

static int file_write(fs::file* f, const void* buf, uint32_t size)
{
    if (!f || !buf)
        return -EINVAL;
    if (!f->f_inode)
        return -EBADF;
    if ((f->f_inode->i_mode & S_IFMT) != S_IFREG)
        return -EISDIR;

    ramfs_file* rf = (ramfs_file*)f->f_inode->i_private;
    if (!rf)
        return -EINVAL;

    // Compare without adding first so a huge size cannot wrap end_pos.
    if (f->f_pos > s_max_file_cap || size > s_max_file_cap - f->f_pos)
        return -ENOSPC;
    uint32_t end_pos = (uint32_t)f->f_pos + size;

    if (end_pos > rf->cap) {
        uint32_t new_cap = rf->cap ? rf->cap : 1024;
        while (new_cap < end_pos) {
            new_cap = new_cap * 2;
            if (new_cap > s_max_file_cap) {
                new_cap = s_max_file_cap;
                break;
            }
        }
        if (new_cap < end_pos)
            return -ENOSPC;

        char* new_data = (char*)mm::kmalloc(new_cap);
        if (!new_data)
            return -ENOMEM;
        for (uint32_t i = 0; i < new_cap; i++)
            new_data[i] = 0;
        if (rf->data && rf->size)
            kmemcpy(new_data, rf->data, rf->size);
        mm::kfree(rf->data);
        rf->data = new_data;
        rf->cap = new_cap;
    }

    kmemcpy(rf->data + f->f_pos, buf, size);
    f->f_pos += size;
    if ((uint32_t)f->f_pos > rf->size)
        rf->size = (uint32_t)f->f_pos;

    f->f_inode->i_size = rf->size;
    return (int)size;
}

static int file_release(fs::inode* ino, fs::file* f)
{
    (void)ino;
    (void)f;
    return 0;
}

/** Directory read: one entry per call; f_pos counts entries already returned.
 */
static int file_readdir(fs::file* f, fs::dirent* out)
{
    if (!f || !out)
        return -EINVAL;
    if (!f->f_inode)
        return -EINVAL;
    if ((f->f_inode->i_mode & S_IFMT) != S_IFDIR)
        return -ENOTDIR;

    ramfs_dir* d = (ramfs_dir*)f->f_inode->i_private;
    if (!d)
        return -EINVAL;

    ramfs_entry* e = d->entries;
    uint32_t skip = (uint32_t)f->f_pos;
    while (e && skip > 0) {
        e = e->next;
        skip--;
    }
    if (!e)
        return 0;

    out->d_ino = e->ino->i_ino;
    out->d_type = ((e->ino->i_mode & S_IFMT) == S_IFDIR) ? DT_DIR : DT_REG;
    out->d_pad[0] = 0;
    out->d_pad[1] = 0;
    out->d_pad[2] = 0;
    for (uint32_t i = 0; i < sizeof(out->d_name); i++)
        out->d_name[i] = e->name[i];

    f->f_pos += 1;
    return 1;
}

static fs::inode* s_root_inode = nullptr;

static uint32_t kstrlen(const char* s)
{
    uint32_t n = 0;
    while (s[n])
        n++;
    return n;
}

static void seed_text_file(fs::inode* root, const char* name, const char* text)
{
    if (!root || !name || !text)
        return;
    fs::dentry t;
    for (uint32_t i = 0; i < sizeof(t); i++)
        ((uint8_t*)&t)[i] = 0;
    uint32_t j = 0;
    while (name[j] && j < sizeof(t.d_name) - 1) {
        t.d_name[j] = name[j];
        j++;
    }
    t.d_name[j] = '\0';
    if (inode_create(root, &t, 0644) != 0)
        return;
    fs::inode* ino = t.d_inode;
    if (!ino)
        return;
    ramfs_file* rf = (ramfs_file*)ino->i_private;
    if (!rf)
        return;
    uint32_t len = kstrlen(text);
    rf->data = (char*)mm::kmalloc(len ? len : 1);
    if (!rf->data)
        return;
    if (len)
        kmemcpy(rf->data, text, len);
    rf->size = len;
    rf->cap = len;
    ino->i_size = len;
}

static void seed_demo_files()
{
    if (!s_root_inode)
        return;
    seed_text_file(s_root_inode, "hello.txt", "Hello from DIT-OS ramfs.\n");
    seed_text_file(s_root_inode, "README.txt",
                   "Try: ls\n      cat hello.txt\n");
}

static int ramfs_get_sb(fs::fs_type* type, const char* dev, void* data,
                        fs::superblock** out)
{
    (void)type;
    (void)dev;
    (void)data;
    if (!out)
        return -EINVAL;

    fs::superblock* sb = (fs::superblock*)mm::kmalloc(sizeof(fs::superblock));
    if (!sb)
        return -ENOMEM;
    for (uint32_t i = 0; i < sizeof(fs::superblock); i++)
        ((uint8_t*)sb)[i] = 0;

    sb->s_blocksize = 512;
    sb->s_blocks = 0;
    sb->s_free_blocks = 0;
    sb->s_free_inodes = 0;
    sb->s_fs_info = nullptr;
    sb->s_ops = &s_sb_ops;

    fs::inode* root = inode_create_dir(sb);
    if (!root)
        return -ENOMEM;
    root->i_ops = &s_inode_ops;
    root->i_file_ops = &s_file_ops;

    sb->s_root = root;
    s_root_inode = root;
    *out = sb;
    return 0;
}

static void ramfs_kill_sb(fs::superblock* sb)
{
    (void)sb;
    /* no persistence, so nothing to do */
}

static fs::fs_type s_ramfs_type = {
    .name = "ramfs",
    .get_sb = ramfs_get_sb,
    .kill_sb = ramfs_kill_sb,
    .next = nullptr,
};

static bool s_inited = false;

void init()
{
    if (s_inited)
        return;
    s_inited = true;

    // Wire operation tables (must happen before mount/get_sb).
    s_sb_ops.alloc_inode = nullptr;
    s_sb_ops.destroy_inode = nullptr;
    s_sb_ops.sync = nullptr;

    s_inode_ops.lookup = inode_lookup;
    s_inode_ops.create = inode_create;
    s_inode_ops.mkdir = inode_mkdir;
    s_inode_ops.unlink = inode_unlink;

    s_file_ops.open = file_open;
    s_file_ops.read = file_read;
    s_file_ops.write = file_write;
    s_file_ops.release = file_release;
    s_file_ops.readdir = file_readdir;

    fs::register_filesystem(&s_ramfs_type);
    fs::vfs_mount("ramfs", "/", "ramfs", nullptr);
    seed_demo_files();
}

} // namespace ramfs
