#include "syscall.h"

void userland_shell_main(void);

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    userland_shell_main();
    return 0;
}
