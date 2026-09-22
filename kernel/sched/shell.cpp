// DIT-OS Kernel - Shell entry point
//
// The shell itself lives in userland/shell/ and is linked into the kernel for
// now; this only hands its entry point to the scheduler.

#include "sched/sched.hpp"

extern "C" void userland_shell_main();

namespace sched
{

void (*get_shell_entry())()
{
    return userland_shell_main;
}

} // namespace sched
