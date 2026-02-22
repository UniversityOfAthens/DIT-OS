#ifndef KERNEL_CORE_KASSERT_HPP
#define KERNEL_CORE_KASSERT_HPP

#include "core/panic.hpp"

#define KASSERT(cond) do { if (!(cond)) core::panic("assert failed: " #cond); } while (0)
#define KASSERT_MSG(cond, msg) do { if (!(cond)) core::panic(msg); } while (0)

#endif
