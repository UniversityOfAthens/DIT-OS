# C Userland Development Guidelines

## Scope

Applies to all code under `userland/` including libc, shell, and user binaries.

## Compiler Configuration

```
-std=c11 -ffreestanding -Wall -Wextra -Werror
```

Note: Userland is freestanding because libc is custom, not system-provided.

## Project Structure

```
userland/
├── libc/           # C library implementation
├── shell/          # Shell binary
├── bin/            # Core user binaries (init, ls, cat, etc.)
├── include/        # Public headers (syscall.h, etc.)
└── crt0.asm        # C runtime startup (calls main)
```

## Type System

### Use Fixed-Width Types

Always include `<stdint.h>` (freestanding). Never use raw `int`, `long`, `char` for sizes.

```c
#include <stdint.h>

uint8_t  type_byte;
uint16_t type_half;
uint32_t type_word;
uint64_t type_dword;

int32_t  signed_val;
size_t   offset;
```

### Error Return Convention

Syscalls and libc functions follow Unix convention:

```c
// On success: return >= 0 (bytes read, etc.)
// On error: return -1, set errno
ssize_t sys_read(int fd, void* buf, size_t count);

// Usage
ssize_t n = read(fd, buf, sizeof(buf));
if (n < 0) {
    // error handling
}
```

## Syscall Interface

### Gateway: int 0x80

```c
// Userland syscall wrapper pattern
int syscall_0(int num);
int syscall_1(int num, int arg1);
int syscall_2(int num, int arg1, int arg2);
int syscall_3(int num, int arg1, int arg2, int arg3);

// EAX = syscall number, EBX/ECX/EDX = args
// Return in EAX; negative = -errno
```

### Syscall Number Registry

Always use the symbolic constants from `userland/include/syscall.h`:

```c
#define SYS_EXIT    1
#define SYS_FORK    2
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_GETPID  5

// Never use raw numbers
// Bad:  syscall_1(1, status);
// Good: syscall_1(SYS_EXIT, status);
```

### Adding a New Syscall

1. Add number to `userland/include/syscall.h` and `kernel/include/syscall/syscall_numbers.hpp` (keep in sync)
2. Add handler to `kernel/syscall/syscall_table.cpp`
3. Add kernel implementation in `kernel/syscall/sys_<name>.cpp`
4. Add libc wrapper in `userland/libc/`
5. Test with userland binary

## Memory Functions

Implement in `userland/libc/memory.c`:

```c
void* memset(void* s, int c, size_t n);
void* memcpy(void* dst, const void* src, size_t n);
void* memmove(void* dst, const void* src, size_t n);
int   memcmp(const void* s1, const void* s2, size_t n);
```

### malloc / free

When heap is implemented:

```c
void* malloc(size_t size);
void  free(void* ptr);
void* calloc(size_t nmemb, size_t size);
void* realloc(void* ptr, size_t size);
```

Until then, use static buffers and stack allocation.

## String Functions

Implement in `userland/libc/string.c`:

```c
size_t strlen(const char* s);
char*  strcpy(char* dst, const char* src);
char*  strncpy(char* dst, const char* src, size_t n);
int    strcmp(const char* s1, const char* s2);
int    strncmp(const char* s1, const char* s2, size_t n);
char*  strcat(char* dst, const char* src);
char*  strchr(const char* s, int c);
char*  strrchr(const char* s, int c);
```

## I/O Functions

Implement in `userland/libc/io.c` using syscall wrappers:

```c
ssize_t read(int fd, void* buf, size_t count);
ssize_t write(int fd, const void* buf, size_t count);
int     open(const char* path, int flags, ...);
int     close(int fd);
```

### File Descriptors

| FD | Purpose |
|----|---------|
| 0  | stdin  |
| 1  | stdout |
| 2  | stderr |

## printf Implementation

Use `va_list` from `<stdarg.h>`. Never allocate dynamically.

```c
// Minimal supported formats: %s %d %u %x %p %c %%
int printf(const char* fmt, ...);
int fprintf(int fd, const char* fmt, ...);
int sprintf(char* str, const char* fmt, ...);
int snprintf(char* str, size_t size, const char* fmt, ...);
```

## Error Handling

Define `errno` and standard values in `userland/libc/errno.c` / `include/errno.h`:

```c
#define EPERM   1   /* Operation not permitted */
#define ENOENT  2   /* No such file or directory */
#define EIO     5   /* I/O error */
#define ENOMEM  12  /* Out of memory */
#define EINVAL  22  /* Invalid argument */
#define ENOSYS  38  /* Function not implemented */

extern int errno;
```

## CRT0 (C Runtime Startup)

In `userland/crt0.asm`:

1. Set up stack pointer
2. Clear BSS section (zero)
3. Call `main(argc, argv)`
4. Call `exit(return_value)`

The entry point is `_start`, not `main`.

## User Binaries

### Pattern

```c
// userland/bin/ls.c
#include <stdint.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    // Minimal implementation
    write(1, "ls\n", 3);
    return 0;
}
```

### Adding a Binary

1. Create `userland/bin/<name>.c`
2. Add to `userland/Makefile` (build rule + install to disk image)
3. Install as `/bin/<name>`

## Absolute Rules

1. **No dynamic memory before heap is initialized** - use static buffers
2. **Never trust kernel return values blindly** - always check syscall return codes
3. **No inline assembly in userland except syscall wrappers** - keep portability
4. **All libc functions must match POSIX prototypes** - for compatibility
5. **No globals in shared libc** - reentrancy matters
