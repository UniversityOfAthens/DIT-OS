#ifndef KERNEL_CORE_COMPILER_HPP
#define KERNEL_CORE_COMPILER_HPP

#if defined(__GNUC__) && !defined(__clang__)
#include "core/compiler_gcc.hpp"
#elif defined(__clang__)
#include "core/compiler_clang.hpp"
#else
#define KERNEL_PACKED
#define KERNEL_NORETURN
#define KERNEL_UNUSED(x) (void)(x)
#define KERNEL_ALIGNED(x)
#define KERNEL_SECTION(x)
#define KERNEL_WEAK
#define KERNEL_PURE
#define KERNEL_CONST
#endif

#endif
