#ifndef KERNEL_ARCH_IO_H
#define KERNEL_ARCH_IO_H

#ifdef __cplusplus
#include <cstdarg>
#else
#include <stdarg.h>
#endif

static inline u8 inb(u16 port)
{
    u8 result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(u16 port, u8 value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline u16 inw(u16 port)
{
    u16 result;
    __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outw(u16 port, u16 value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline void insw(u16 port, void* buffer, u32 count)
{
    __asm__ volatile("rep insw" : "+D"(buffer), "+c"(count) : "d"(port));
}

static inline void outsw(u16 port, const void* buffer, u32 count)
{
    __asm__ volatile("rep outsw" : "+S"(buffer), "+c"(count) : "d"(port));
}

#endif
