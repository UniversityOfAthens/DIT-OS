/** DIT-OS Kernel - Path resolution for syscalls (working directory). */
#ifndef KERNEL_SYSCALL_PATH_HPP
#define KERNEL_SYSCALL_PATH_HPP

#include <stdint.h>

namespace syscall
{

/** Longest path a syscall accepts, including the NUL. */
constexpr uint32_t PATH_MAX = 256;

/** Make @path absolute against @base (an absolute, normalised directory)
 *  and normalise it into @out: empty and "." components go, ".." drops the
 *  previous component (never above "/"). Returns the length, or
 *  -ENAMETOOLONG when the result does not fit @cap, or -ENOENT for an empty
 *  @path (POSIX). */
int path_normalize(char* out, uint32_t cap, const char* base, const char* path);

/** Fetch the user string at @user_path once and resolve it against the
 *  current task's working directory (path_normalize). Returns the length of
 *  the absolute path in @out, or -EFAULT / -ENAMETOOLONG / -ENOENT. */
int path_from_user(char* out, uint32_t cap, uintptr_t user_path);

} // namespace syscall

#endif
