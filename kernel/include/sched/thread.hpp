#ifndef KERNEL_SCHED_THREAD_HPP
#define KERNEL_SCHED_THREAD_HPP

#include <stdint.h>
#include "sched/task.hpp"
#include "sched/process.hpp"

namespace sched {

/** Thread = task tied to a process (for now 1:1) */
struct Thread {
    Task     task;
    Process* process;
};

} // namespace sched

#endif
