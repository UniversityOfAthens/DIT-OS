/** DIT-OS Kernel - Scheduler timer / sleep queue types (tick base: dev::timer_ticks). */
#ifndef KERNEL_SCHED_TIMER_HPP
#define KERNEL_SCHED_TIMER_HPP

#include <stdint.h>

namespace sched {

struct Task;

/** Relative sleep: wake after N ticks from dev::timer_ticks(). */
struct SleepRequest {
    Task*         task;
    uint32_t      remaining_ticks;
    SleepRequest* next;
};

/** Absolute wakeup tick (monotonic, same domain as dev::timer_ticks()). */
struct SleepUntil {
    Task*       task;
    uint32_t    deadline_tick;
    SleepUntil* next;
};

/** Placeholder for POSIX interval timers (future itimers). */
struct IntervalTimer {
    uint32_t interval_ticks;
    uint32_t value_ticks;
    int      which; /* ITIMER_REAL, etc. */
};

} // namespace sched

#endif
