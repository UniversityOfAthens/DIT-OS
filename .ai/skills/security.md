# Security Best Practices

## Scope

Applies to all code in DIT-OS. Security in an OS is critical since kernel code runs with full privileges.

## Principle of Least Privilege

1. **User code never runs in kernel mode** - syscalls are the only gateway
2. **Validate all user-supplied data** - never trust userspace
3. **Minimize interrupt-disable time** - re-enable ASAP to avoid deadlocks
4. **Isolate address spaces** - each process gets its own page tables

## Input Validation

### Syscall Boundary

All data crossing the syscall boundary is untrusted:

```cpp
// Kernel syscall handler
int sys_read(int fd, void* buf, size_t count) {
    // Validate fd
    if (fd < 0 || fd >= MAX_FDS) {
        return -EBADF;
    }

    // Validate buffer is in user address space
    if (!is_user_address(buf) || !is_user_address((u8*)buf + count - 1)) {
        return -EFAULT;
    }

    // Validate count
    if (count == 0) {
        return 0;
    }

    // ... proceed with actual read
}
```

### Never Trust Pointers from Userspace

```cpp
// Bad: dereference directly
void* user_ptr = (void*)user_arg;
char c = *(char*)user_ptr;  // PAGE FAULT if invalid

// Good: use safe copy with fault handling
char c;
if (!copy_from_user(&c, user_ptr, 1)) {
    return -EFAULT;
}
```

## Memory Safety

### Buffer Overflow Prevention

```cpp
// Bad
void copy_string(char* dst, const char* src) {
    while (*src) *dst++ = *src++;
}

// Good
int copy_string(char* dst, size_t dst_size, const char* src) {
    size_t i;
    for (i = 0; i < dst_size - 1 && src[i]; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
    if (src[i]) return -ENAMETOOLONG;
    return 0;
}
```

### Integer Overflow

Always check arithmetic before it overflows:

```cpp
// Bad: may overflow
size_t total = page_count * PAGE_SIZE;

// Good: check first
if (page_count > SIZE_MAX / PAGE_SIZE) {
    return -ENOMEM;
}
size_t total = page_count * PAGE_SIZE;
```

### Null Pointer Dereference

Always check allocations:

```cpp
void* ptr = kmalloc(size);
if (!ptr) {
    panic("kmalloc: out of memory");
}
```

## Concurrency Safety

### Lock Ordering

Establish a global lock order to prevent deadlocks:

```
1. Interrupt lock (highest)
2. Scheduler lock
3. Memory manager lock
4. Device locks
5. Filesystem locks (lowest)
```

Never acquire locks out of order.

### Lock Patterns

```cpp
// Scoped lock pattern
void protected_operation() {
    spin_lock(&global_lock);
    // ... critical section ...
    spin_unlock(&global_lock);
}

// Interrupt-safe pattern
void protected_with_irq() {
    u32 flags;
    spin_lock_irqsave(&lock, flags);
    // ... critical section ...
    spin_unlock_irqrestore(&lock, flags);
}
```

### Disable Interrupts Minimally

```cpp
// Bad: long critical section with interrupts disabled
cli();
for (int i = 0; i < 1000; i++) {
    expensive_operation();
}
sti();

// Good: disable only for brief pointer swap
cli();
task_t* next = dequeue_task();
sti();
if (next) context_switch(next);
```

## Address Space Isolation

### User vs Kernel Address Space

```cpp
#define USER_SPACE_START   0x00000000
#define USER_SPACE_END     0x003FFFFF  // 4 MiB for user
#define KERNEL_SPACE_START 0x00400000
#define KERNEL_SPACE_END   0xFFFFFFFF

bool is_user_address(const void* addr) {
    uintptr_t a = (uintptr_t)addr;
    return a >= USER_SPACE_START && a < USER_SPACE_END;
}

bool is_kernel_address(const void* addr) {
    uintptr_t a = (uintptr_t)addr;
    return a >= KERNEL_SPACE_START;
}
```

### Page Table Permissions

- Kernel pages: supervisor-only (ring 0), read/write
- User pages: user-accessible (ring 3), read/write/exec as needed
- Never map kernel pages as user-accessible

```cpp
// Page flags
#define PAGE_USER    (1 << 2)  // User-accessible
#define PAGE_WRITE   (1 << 1)  // Writable
#define PAGE_PRESENT (1 << 0)  // Present

// Kernel page: present + writable, NOT user
u32 kernel_flags = PAGE_PRESENT | PAGE_WRITE;

// User page: present + writable + user
u32 user_flags = PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
```

## Syscall Security

### Syscall Table Validation

```cpp
// Bound-check syscall number
static const syscall_handler_t syscall_table[MAX_SYSCALLS] = {
    [SYS_EXIT]   = sys_exit,
    [SYS_FORK]   = sys_fork,
    [SYS_READ]   = sys_read,
    [SYS_WRITE]  = sys_write,
    [SYS_GETPID] = sys_getpid,
};

int dispatch_syscall(u32 num, u32 arg1, u32 arg2, u32 arg3) {
    if (num >= MAX_SYSCALLS || !syscall_table[num]) {
        return -ENOSYS;
    }
    return syscall_table[num](arg1, arg2, arg3);
}
```

### Stack Canary (Future)

When feasible, add stack canaries:

```cpp
#define STACK_CANARY_VALUE 0xDEADBEEF

struct Task {
    u32 stack_canary;   // must be first
    u32 pid;
    // ...
    u32 stack_canary_end;
};

void check_canary(Task* t) {
    if (t->stack_canary != STACK_CANARY_VALUE ||
        t->stack_canary_end != STACK_CANARY_VALUE) {
        panic("Stack overflow detected");
    }
}
```

## Hardware Security

### GDT Ring Levels

- Ring 0: Kernel code
- Ring 3: User code (when implemented)
- Never allow user code to run in ring 0

### IDT Gate Types

```cpp
// Kernel interrupt gate: DPL=0, only kernel can trigger
set_idt_gate(vector, handler, GATE_INTERRUPT, 0);

// User syscall gate: DPL=3, user can trigger via int 0x80
set_idt_gate(0x80, syscall_handler, GATE_TRAP, 3);
```

### CR0/CR4 Bits

Always set security-relevant CPU flags:

```cpp
// CR0
u32 cr0 = read_cr0();
cr0 |= (1 << 1);   // Monitor co-processor
cr0 |= (1 << 16);  // Write-protect (protect kernel pages)
write_cr0(cr0);

// CR4
u32 cr4 = read_cr4();
cr4 |= (1 << 2);   // Timestamp disable
cr4 |= (1 << 7);   // Page global enable
write_cr4(cr4);
```

## Secrets

1. **Never hardcode passwords or keys** in source code
2. **Never log sensitive data** - no memory dumps with passwords
3. **Zero sensitive memory on free** - clear stack contents, key material

```cpp
void secure_zero(void* ptr, size_t size) {
    volatile u8* p = (volatile u8*)ptr;
    while (size--) *p++ = 0;
}
```

## Compilation Security Flags

Always use:

```makefile
CFLAGS += -fstack-protector      # Stack canaries
CFLAGS += -D_FORTIFY_SOURCE=2    # Buffer overflow checks
CFLAGS += -Wformat -Wformat-security  # Format string warnings
CFLAGS += -Werror               # Warnings are errors
```

Note: `-fstack-protector` may not work in freestanding mode. Implement manual canaries instead.

## Absolute Rules

1. **Never dereference user-space pointers directly** - use `copy_from_user` / `copy_to_user`
2. **Never disable interrupts longer than necessary**
3. **Never map kernel pages as user-accessible**
4. **Never trust syscall arguments** - validate everything
5. **Never use unbounded `memcpy`/`strcpy`** - always pass size
6. **Never return kernel addresses to userspace**
7. **Never allow unsigned values to wrap into negative** - check bounds first
8. **Never ignore error returns from memory allocators**
