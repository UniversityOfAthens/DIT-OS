// DIT-OS Userland - init task entry (for now: directly start shell)

#include "syscall.h"

void userland_shell_main(void);

void userland_init_main(void)
{
    userland_shell_main();
    exit(0);
}
