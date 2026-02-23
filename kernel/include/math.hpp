/** DIT-OS Kernel - Minimal math / min-max (freestanding). */
#ifndef KERNEL_MATH_HPP
#define KERNEL_MATH_HPP

namespace kernel {

template<typename T>
inline const T& min(const T& a, const T& b) {
    return a < b ? a : b;
}

template<typename T>
inline const T& max(const T& a, const T& b) {
    return a > b ? a : b;
}

template<typename T>
inline T div_round_up(T x, T y) {
    return (x + y - 1) / y;
}

} // namespace kernel

#endif
