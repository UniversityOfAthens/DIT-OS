#ifndef KERNEL_FS_RAMFS_RAMFS_HPP
#define KERNEL_FS_RAMFS_RAMFS_HPP

namespace ramfs
{

/** Register ramfs and mount it as the root filesystem ('/'). */
void init();

} // namespace ramfs

#endif
