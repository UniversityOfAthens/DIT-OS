#ifndef KERNEL_SCHED_PROCESS_HPP
#define KERNEL_SCHED_PROCESS_HPP

#include <stdint.h>

namespace sched {

struct Task;

/** Minimal process: one thread (task) for now */
struct Process {
    int     pid;
    Task*   main_task;
};

} // namespace sched

#endif
