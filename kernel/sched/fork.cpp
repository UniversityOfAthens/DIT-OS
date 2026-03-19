// DIT-OS Kernel - Fork (stub)
//
// Full fork() requires cloning user context/stack and duplicating address space.
// The current kernel scheduler runs a set of kernel tasks without a usermode
// return trampoline, so for now we provide a placeholder implementation.

#include <stdint.h>

namespace sched {

int fork_current() {
    return -38; /* -ENOSYS */
}

} // namespace sched

