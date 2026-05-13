#ifndef KERNEL_CORE_PANIC_HPP
#define KERNEL_CORE_PANIC_HPP

namespace core
{

void panic(const char* msg);
void panicf(const char* fmt, ...);

} // namespace core

#endif
