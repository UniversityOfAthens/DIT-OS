#ifndef KERNEL_FS_DIRENT_HPP
#define KERNEL_FS_DIRENT_HPP

#include "core/compiler.hpp"
#include <stddef.h>
#include <stdint.h>

namespace fs
{

/** Maximum name length in struct dirent (avoid clashing with libc NAME_MAX). */
#define FS_NAME_MAX 256

/** Size of the dirent copied by getdents; part of the syscall ABI. */
constexpr uint32_t DIRENT_SIZE = 264;

/* Packed so no compiler can insert padding between the fields; the userland
 * mirror and kernel/syscall/abi_layout.cpp pin the same layout. */
struct dirent
{
    uint32_t d_ino;
    uint8_t d_type; /* DT_REG, DT_DIR, etc. */
    uint8_t d_pad[3];
    char d_name[FS_NAME_MAX];
} KERNEL_PACKED;

static_assert(sizeof(dirent) == DIRENT_SIZE,
              "struct dirent is part of the syscall ABI");

#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12

} // namespace fs

#endif
