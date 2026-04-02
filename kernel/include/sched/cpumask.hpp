/** DIT-OS Kernel - CPU masks for future SMP affinity / load balancing. */
#ifndef KERNEL_SCHED_CPUMASK_HPP
#define KERNEL_SCHED_CPUMASK_HPP

#include <stdint.h>

namespace sched {

/** Max CPUs representable in one cpumask_t (expand when raising SMP limits). */
constexpr unsigned SCHED_MAX_CPUS = 64;

using cpumask_t = uint64_t;

inline void cpumask_clear(cpumask_t* m) {
    if (m)
        *m = 0;
}

inline void cpumask_setall(cpumask_t* m) {
    if (m)
        *m = ~cpumask_t(0);
}

inline void cpumask_set_cpu(cpumask_t* m, unsigned cpu) {
    if (m && cpu < SCHED_MAX_CPUS)
        *m |= (cpumask_t(1) << cpu);
}

inline void cpumask_clear_cpu(cpumask_t* m, unsigned cpu) {
    if (m && cpu < SCHED_MAX_CPUS)
        *m &= ~(cpumask_t(1) << cpu);
}

inline bool cpumask_test_cpu(cpumask_t m, unsigned cpu) {
    return cpu < SCHED_MAX_CPUS && (m & (cpumask_t(1) << cpu)) != 0;
}

inline bool cpumask_empty(cpumask_t m) {
    return m == 0;
}

inline bool cpumask_equal(cpumask_t a, cpumask_t b) {
    return a == b;
}

/** First set bit index, or SCHED_MAX_CPUS if none. */
inline unsigned cpumask_first(cpumask_t m) {
    if (m == 0)
        return SCHED_MAX_CPUS;
    unsigned i = 0;
    while (i < SCHED_MAX_CPUS && ((m >> i) & 1) == 0)
        ++i;
    return i;
}

} // namespace sched

#endif
