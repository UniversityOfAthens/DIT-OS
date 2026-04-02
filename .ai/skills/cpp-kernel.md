# C++ Kernel Development Guidelines

## Scope

Applies to all code under `kernel/` and `bootloader/src/` (C++ portions).

## Compiler Configuration

```
-std=c++17 -ffreestanding -fno-rtti -fno-exceptions
-fno-stack-protector -fno-pie -fno-pic -m32
-Wall -Wextra -Werror
```

## Namespaces

All kernel code must be within the `kernel` namespace hierarchy:

```cpp
namespace kernel {
namespace core {
    // Core kernel functionality
}

namespace mm {
    // Memory management
}

namespace sched {
    // Scheduler
}

namespace dev {
    // Device drivers
}

namespace fs {
    // Filesystem
}

namespace syscall {
    // System calls
}
}
```

Bootloader code uses `namespace boot`.

## No Standard Library

Freestanding compilation means **no** access to:

- `<iostream>`, `<string>`, `<vector>`, `<map>`, etc.
- `<new>` (no `operator new`/`operator delete` by default)
- `<exception>`, `<typeinfo>`
- Most of `<algorithm>`, `<utility>` (some are freestanding in C++17)

**Allowed** freestanding headers in C++17:

- `<cstdint>`, `<cstddef>`, `<climits>`, `<cfloat>`
- `<type_traits>` (C++17 freestanding subset)
- `<utility>` (limited)
- `<initializer_list>`
- `<new>` (placement new only)
- `<cstdarg>` (for variadic functions)
- `<cstring>` (limited, implement manually if needed)

When in doubt, implement the functionality yourself in `kernel/lib/`.

## Memory Management

### No Heap Allocation (Initially)

The kernel heap may not be available early in boot. Use:

- **Static arrays** for fixed-size buffers
- **Stack allocation** for temporary data (watch stack size limits)
- **Placement new** for constructing objects in known memory regions

### When Heap Is Available

Use kernel-specific allocators, never raw `new`/`delete`:

```cpp
// Use kernel allocators
void* ptr = mm::kmalloc(size);
mm::kfree(ptr);

// Or slab allocator for fixed-size objects
struct Task* task = slab_alloc(task_cache);
slab_free(task_cache, task);
```

### Alignment

Use `alignas()` for structures that require specific alignment:

```cpp
struct alignas(4096) PageTable {
    u64 entries[1024];
};
```

## Pointers and Casting

### Prefer `reinterpret_cast` for Hardware Addresses

```cpp
// Good: explicit and clear
volatile u32* serial_data = reinterpret_cast<volatile u32*>(0x3F8);

// Bad: C-style cast hides intent
u32* serial_data = (u32*)0x3F8;
```

### Use `volatile` for Memory-Mapped I/O

```cpp
struct SerialPort {
    volatile u8 data;       // 0x3F8
    volatile u8 ier;        // 0x3F9
    volatile u8 iir;        // 0x3FA
    volatile u8 lcr;        // 0x3FB
};
```

### Null Pointer Checks

Always check pointers from the memory allocator:

```cpp
void* ptr = mm::kmalloc(size);
if (!ptr) {
    core::panic("kmalloc failed: out of memory\n");
}
```

## Inline Assembly

Use `asm volatile` with proper clobbers:

```cpp
// Good
static inline void outb(u16 port, u8 value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port) : "memory");
}

// Good: with output
static inline u8 inb(u16 port) {
    u8 result;
    asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
```

Rules:

- Always use `volatile` for I/O operations
- List all clobbered registers
- Use `"memory"` clobber when the asm affects memory
- Prefer named input/output operands over positional

## Structs and Packing

Use `KERNEL_PACKED` for hardware structures:

```cpp
struct KERNEL_PACKED GDTEntry {
    u16 limit_low;
    u16 base_low;
    u8 base_middle;
    u8 access;
    u8 granularity;
    u8 base_high;
};
```

**Never** pack structs that don't need it - alignment matters for performance.

## constexpr and Templates

### Use `constexpr` Liberally

```cpp
constexpr u32 PAGE_SIZE = 4096;
constexpr u32 PAGE_SHIFT = 12;
constexpr u32 PAGE_ALIGN(u32 addr) { return (addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1); }
```

### Templates Are Allowed

Templates work in freestanding mode. Use them for type-safe containers:

```cpp
template<typename T>
class Vector {
    // ...
};
```

Avoid:

- Exception-throwing template code
- Templates requiring `<type_traits>` full library
- Overly complex metaprogramming (keep it readable)

## Bit Manipulation

Use explicit bit operations with named constants:

```cpp
// Good
constexpr u32 PAGE_PRESENT = (1 << 0);
constexpr u32 PAGE_WRITE = (1 << 1);
constexpr u32 PAGE_USER = (1 << 2);

flags |= PAGE_PRESENT | PAGE_WRITE;
flags &= ~PAGE_USER;

// Bad: magic numbers
flags |= 0x3;
flags &= ~0x4;
```

Helper functions for common operations:

```cpp
constexpr bool bit_is_set(u32 value, u8 bit) {
    return (value >> bit) & 1;
}

constexpr u32 set_bit(u32 value, u8 bit) {
    return value | (1 << bit);
}

constexpr u32 clear_bit(u32 value, u8 bit) {
    return value & ~(1 << bit);
}
```

## Logging and Debugging

### Use `printk` for Kernel Logging

```cpp
core::printk(KERN_INFO "Scheduler: task %d started\n", task->pid);
core::printk(KERN_ERR "Failed to allocate page: %p\n", addr);
```

### Log Levels

| Level | Usage |
|-------|-------|
| `KERN_EMERG` | System is unusable |
| `KERN_ALERT` | Action must be taken immediately |
| `KERN_CRIT` | Critical conditions |
| `KERN_ERR` | Error conditions |
| `KERN_WARNING` | Warning conditions |
| `KERN_NOTICE` | Normal but significant |
| `KERN_INFO` | Informational |
| `KERN_DEBUG` | Debug-level messages |

### Assertions

Use `KERNEL_ASSERT` for internal consistency checks:

```cpp
KERNEL_ASSERT(page_table != nullptr);
KERNEL_ASSERT(offset < PAGE_SIZE);
```

## Initialization Order

Kernel initialization follows a strict order. Respect dependencies:

1. **Early init**: Serial console, basic memory detection
2. **CPU init**: GDT, IDT, TSS, PIC/APIC
3. **Memory init**: PMM, VMM, heap
4. **Device init**: Timer, keyboard, disk controllers
5. **Filesystem init**: VFS, ramfs
6. **Scheduler init**: Task system, first user process

Never access a subsystem before it is initialized.

## Absolute Rules

1. **No `new`/`delete`** - use kernel allocators
2. **No exceptions** - use error codes and `panic()`
3. **No RTTI** - no `dynamic_cast`, `typeid`
4. **No STL containers** - implement or use kernel equivalents
5. **No floating point** - unless implementing FPU support
6. **No thread-unsafe globals without protection** - use locks
7. **No busy-waiting without `cpu_relax()`** - yield to scheduler
8. **No uninitialized variables** - the compiler will warn, fix them
