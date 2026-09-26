// DIT-OS Kernel - sys_waitpid

#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include "sched/sched.hpp"
#include "mm/uaccess.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_waitpid_impl(const SyscallArgs* args);

/* The only defined option; anything else is rejected. */
constexpr uint32_t WNOHANG = 1;

SyscallResult sys_waitpid_impl(const SyscallArgs* args)
{
    int32_t pid = static_cast<int32_t>(args->args[0]);
    uintptr_t status = static_cast<uintptr_t>(args->args[1]);
    uint32_t options = static_cast<uint32_t>(args->args[2]);
    if (options & ~WNOHANG)
        return -EINVAL;
    if (status && !mm::user_range_ok(status, sizeof(int32_t)))
        return -EFAULT;

    int32_t code = 0;
    int result = sched::wait_child(pid, &code, (options & WNOHANG) != 0);
    if (result <= 0)
        return result;
    if (status && mm::copy_to_user(status, &code, sizeof(code)) < 0)
        return -EFAULT;
    return result;
}

} // namespace syscall
