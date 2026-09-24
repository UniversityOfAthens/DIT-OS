/** DIT-OS Kernel - Process descriptor (parent/child tree, signals, exit
 * status). */
#ifndef KERNEL_SCHED_PROCESS_HPP
#define KERNEL_SCHED_PROCESS_HPP

#include <stdint.h>
#include "sched/signal.hpp"

namespace sched
{

struct Task;

/** One slot per real signal (1..31); index with (sig - 1). */
constexpr int SIGNAL_ACTION_SLOTS = 31;

enum class ProcessState : uint8_t
{
    Running,
    Sleeping,
    Zombie,
    Stopped,
};

struct Process
{
    int pid;
    int ppid;
    ProcessState state;
    /** Valid when state == Zombie (consumer reads via wait). */
    int exit_code;
    Task* main_task;
    Process* parent;
    /** Singly-linked list of children (sibling_next). */
    Process* children_head;
    Process* sibling_next;
    sigset_t sig_pending;
    sigset_t sig_blocked;
    SignalAction actions[SIGNAL_ACTION_SLOTS];
};

} // namespace sched

#endif
