/** DIT-OS Kernel - Assert macro; uses core panic when NDEBUG not set. */
#ifndef KERNEL_ASSERT_HPP
#define KERNEL_ASSERT_HPP

#include "core/kassert.hpp"
#include "core/panic.hpp"

#ifdef NDEBUG
#define assert(cond) ((void)0)
#else
#define assert(cond) KASSERT(cond)
#endif

#endif
