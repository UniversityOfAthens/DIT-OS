// DIT-OS Kernel - sys_waitpid

#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include "sched/sched.hpp"
#include "sched/wait.hpp"
#include "mm/uaccess.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_waitpid_impl(const SyscallArgs* args);

/* Every option wait_child() understands; anything else is rejected. */
constexpr uint32_t WAIT_OPTIONS = sched::WNOHANG | sched::WUNTRACED |
                                  sched::WCONTINUED;

SyscallResult sys_waitpid_impl(const SyscallArgs* args)
{
    /* The pid is a signed 32-bit selector (negative = process group);
     * anything wider would truncate into a different selector. */
    if (args->args[0] > static_cast<uint64_t>(UINT32_MAX))
        return -EINVAL;
    int32_t pid = static_cast<int32_t>(static_cast<uint32_t>(args->args[0]));
    uintptr_t status = static_cast<uintptr_t>(args->args[1]);
    if (args->args[2] & ~static_cast<uint64_t>(WAIT_OPTIONS))
        return -EINVAL;
    int options = static_cast<int>(args->args[2]);
    if (status && !mm::user_range_ok(status, sizeof(int32_t)))
        return -EFAULT;

    int32_t code = 0;
    int result = sched::wait_child(pid, &code, options);
    if (result <= 0)
        return result;
    if (status && mm::copy_to_user(status, &code, sizeof(code)) < 0)
        return -EFAULT;
    return result;
}

} // namespace syscall
