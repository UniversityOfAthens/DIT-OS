#ifndef KERNEL_FS_RAMFS_RAMFS_HPP
#define KERNEL_FS_RAMFS_RAMFS_HPP

#include <stdint.h>

namespace ramfs
{

/** Register ramfs and mount it as the root filesystem ('/'). */
void init();

/** Create a file with @size bytes of @data in the root directory, replacing
 *  nothing: an existing name is left alone. Used to publish a program that is
 *  linked into the kernel so execve() can load it. */
void seed_file(const char* name, const void* data, uint32_t size);

} // namespace ramfs

#endif
