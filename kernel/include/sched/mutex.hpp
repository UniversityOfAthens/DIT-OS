#ifndef KERNEL_SCHED_MUTEX_HPP
#define KERNEL_SCHED_MUTEX_HPP

#include <stdint.h>

namespace sched
{

struct Mutex
{
    volatile uint32_t locked;
};

void mutex_init(Mutex* mutex);
void mutex_lock(Mutex* mutex);
void mutex_unlock(Mutex* mutex);

}

#endif
