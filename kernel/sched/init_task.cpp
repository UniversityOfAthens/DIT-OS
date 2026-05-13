// DIT-OS Kernel - Init task entry (returns shell as first process)

#include "sched/sched.hpp"

extern "C" void userland_init_main();

namespace sched
{

void (*get_init_entry())()
{
    return userland_init_main;
}

} // namespace sched
