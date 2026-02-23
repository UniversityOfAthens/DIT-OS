/** DIT-OS Kernel - Stdlib declarations (freestanding: no full libc). */
#ifndef KERNEL_STDLIB_HPP
#define KERNEL_STDLIB_HPP

#include "stddef.hpp"

extern "C" {

void  abort(void) __attribute__((noreturn));
void  __attribute__((noreturn)) _Exit(int status);
int   atoi(const char* nptr);

/* Kernel heap (implemented in mm/heap.cpp or equivalent) */
void* malloc(size_t size);
void  free(void* ptr);
void* realloc(void* ptr, size_t size);
void* calloc(size_t nmemb, size_t size);

} // extern "C"

#endif
