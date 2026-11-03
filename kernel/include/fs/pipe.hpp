/** DIT-OS Kernel - Anonymous pipes (pipe(2)). */
#ifndef KERNEL_FS_PIPE_HPP
#define KERNEL_FS_PIPE_HPP

#include <stdint.h>

namespace fs
{

struct file;

/** Bytes a pipe buffers. Writes of up to PIPE_BUF bytes are atomic. */
constexpr uint32_t PIPE_CAPACITY = 4096;
constexpr uint32_t PIPE_BUF = 512;

/** Create a pipe: @read_end and @write_end each receive a new open file
 *  (refcount 1). Returns 0, or -ENOMEM (nothing is allocated then). */
int pipe_create(file** read_end, file** write_end);

/** True if @f is either end of a pipe. */
bool pipe_is_pipe(const file* f);

} // namespace fs

#endif
