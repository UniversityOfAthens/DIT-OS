#ifndef KERNEL_FS_SELECT_HPP
#define KERNEL_FS_SELECT_HPP

#include <stdint.h>

namespace fs {

/** Maximum fd set size (NFD = number of bits in fd_set) */
#define NFD 256
#define FD_SETSIZE NFD

typedef struct fd_set {
    uint8_t fds[NFD / 8];
} fd_set;

#define FD_ZERO(set)   do { for (unsigned _i = 0; _i < sizeof(fd_set); _i++) (set)->fds[_i] = 0; } while (0)
#define FD_SET(fd, set)    do { (set)->fds[(fd)/8] |= (1U << ((fd) % 8)); } while (0)
#define FD_CLR(fd, set)    do { (set)->fds[(fd)/8] &= ~(1U << ((fd) % 8)); } while (0)
#define FD_ISSET(fd, set)  ((set)->fds[(fd)/8] & (1U << ((fd) % 8)))

/** Select: wait for readiness on read/write/except fd sets. timeout can be nullptr. Returns count of ready fds or negative errno */
int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, void* timeout);

} // namespace fs

#endif
