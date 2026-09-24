/** DIT-OS Kernel - Character classification (freestanding). */
#ifndef KERNEL_CTYPE_HPP
#define KERNEL_CTYPE_HPP

#include "stdint.hpp"

extern "C"
{
    int isalnum(int c);
    int isalpha(int c);
    int isblank(int c);
    int iscntrl(int c);
    int isdigit(int c);
    int isgraph(int c);
    int islower(int c);
    int isprint(int c);
    int ispunct(int c);
    int isspace(int c);
    int isupper(int c);
    int isxdigit(int c);

    int tolower(int c);
    int toupper(int c);

} // extern "C"

#endif
