---
name: filesystem-vfs
description: The DIT-OS VFS layer and ramfs — object model (superblock, inode, dentry, file), operation tables, path lookup, what exists versus what is only declared, and how to add a filesystem. Load before editing kernel/fs/ or kernel/include/fs/.
---

# Filesystem / VFS

## Layers

```
sys_open / sys_read / sys_stat / sys_getdents ...   (kernel/syscall/)
        ↓   kernel buffers only, never user pointers
VFS  fs::vfs_*                                      (kernel/fs/vfs.cpp)
        ↓   ops tables
ramfs                                               (kernel/fs/ramfs/)
```

Syscall handlers call `fs::vfs_*`; they never call a filesystem driver
directly.

## What exists

- **Implemented:** `kernel/fs/vfs.cpp` (open, close, read, write, lseek,
  path lookup, stat, readdir, mount, `namei`) and `ramfs`
  (`ramfs.cpp`, `inode.cpp`): in-memory files and directories, files seeded
  at boot (`ramfs::seed_file`, e.g. `greeter`, `hello.txt`).
- **Declared only:** headers such as `buffer.hpp`, `poll.hpp`, `select.hpp`
  have no implementation. There is no buffer cache, dcache, block layer or
  on-disk filesystem yet. Do not call what is not implemented; grep first.

## Object model (`kernel/include/fs/`)

C-style `snake_case` structs in namespace `fs`: `superblock`, `inode`,
`dentry`, `file`, `dirent`, `fs_type`.

Operation tables (`ops.hpp`), filled with static function pointers by the
driver:

```cpp
struct InodeOps  { lookup, create, mkdir, unlink };      // (inode* dir, dentry*, ...)
struct FileOps   { open, read, write, release, readdir }; // read/write take uint32_t size
struct SuperblockOps { alloc_inode?, destroy_inode, sync };
```

`fs_type` has `get_sb` / `kill_sb`. Return `0`/byte count on success,
`-errno` on failure. Check `ops.hpp` for the exact signatures.

## Path lookup

- Done by the VFS (`vfs_path_lookup`, `namei`); a filesystem only
  implements `lookup(dir, dentry)` for one component.
- Must handle `/`, repeated `/`, `.`, `..` (clamped at root), empty
  components and over-long names (`-ENAMETOOLONG`).
- Release references with `dentry_put` / `inode_put`.

## Rules

- Lengths are `uint32_t`; check `offset + size` for overflow and against
  the file cap (`s_max_file_cap` in ramfs) before growing.
- `lseek` must reject negative results and unknown `whence` (`-EINVAL`).
- Reads at or past EOF return 0.
- Copy names with an explicit bound and always NUL-terminate
  (`d_name` capacity − 1).
- Zero every `dirent`/`stat` before filling it — it goes to user space.
- Free an inode only when nothing references it.
- Error codes: `-ENOENT`, `-EEXIST`, `-ENOTDIR`, `-EISDIR`, `-ENOSPC`,
  `-ENOMEM`, `-EINVAL`, `-EIO`.

## Adding a filesystem

1. `kernel/fs/<name>/` with `<name>.cpp`, `<name>.hpp`, listed in
   `kernel/Makefile`.
2. Static `InodeOps`, `FileOps`, `SuperblockOps` tables and an `fs_type`
   with `get_sb`/`kill_sb`.
3. Register and mount through the VFS (`vfs_mount`).
4. Anything parsed from disk is untrusted input: bounds-check every size,
   offset, count and name (CVE catalog: GRUB2 ext2 symlink overflow).
5. Self-test in `init.c`, security probes, docs.
