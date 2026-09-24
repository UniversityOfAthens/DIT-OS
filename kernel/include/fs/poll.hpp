#ifndef KERNEL_FS_POLL_HPP
#define KERNEL_FS_POLL_HPP

#include <stdint.h>

namespace fs
{

/** Poll events (can be combined) */
#define POLLIN 0x0001
#define POLLPRI 0x0002
#define POLLOUT 0x0004
#define POLLERR 0x0008
#define POLLHUP 0x0010
#define POLLNVAL 0x0020
#define POLLRDNORM POLLIN
#define POLLWRNORM POLLOUT

struct pollfd
{
    int fd;
    uint16_t events; /* requested events */
    uint16_t revents; /* returned events */
};

/** Check readiness of fds; timeout_ms -1 = block, 0 = non-block. Returns count
 * of ready fds or negative errno */
int poll(struct pollfd* fds, uint32_t nfds, int timeout_ms);

} // namespace fs

#endif
