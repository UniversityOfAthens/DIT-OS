#ifndef KERNEL_CORE_COMPILER_GCC_HPP
#define KERNEL_CORE_COMPILER_GCC_HPP

#define KERNEL_PACKED __attribute__((packed))
#define KERNEL_NORETURN __attribute__((noreturn))
#define KERNEL_UNUSED(x) (void)(x)
#define KERNEL_ALIGNED(x) __attribute__((aligned(x)))
#define KERNEL_SECTION(x) __attribute__((section(x)))
#define KERNEL_WEAK __attribute__((weak))
#define KERNEL_PURE __attribute__((pure))
#define KERNEL_CONST __attribute__((const))
#define KERNEL_PRINTF(fmt, first) __attribute__((format(printf, fmt, first)))
#define KERNEL_ALWAYS_INLINE __attribute__((always_inline)) inline

#endif
