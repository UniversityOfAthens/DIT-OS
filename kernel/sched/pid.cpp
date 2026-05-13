// DIT-OS Kernel - PID allocation (minimal, monotonic)

#include "sched/sched.hpp"
#include "sched/task.hpp"
#include <stdint.h>

namespace sched
{

static uint32_t s_next_pid = 1;

uint32_t pid_alloc()
{
    return s_next_pid++;
}

uint32_t current_pid()
{
    return current_task ? current_task->pid : 0;
}

} // namespace sched
