// DIT-OS Kernel - Minimal spinlock mutex (not yet integrated)

#include "sched/mutex.hpp"

namespace sched
{

static inline uint32_t xchg32(volatile uint32_t* addr, uint32_t newval)
{
    uint32_t old;
    asm volatile("xchg %0, %1"
                 : "=r"(old), "+m"(*addr)
                 : "0"(newval)
                 : "memory");
    return old;
}

void mutex_init(Mutex* m)
{
    m->locked = 0;
}

void mutex_lock(Mutex* m)
{
    while (xchg32(&m->locked, 1) != 0)
    {
        asm volatile("pause");
    }
}

void mutex_unlock(Mutex* m)
{
    m->locked = 0;
}

} // namespace sched
