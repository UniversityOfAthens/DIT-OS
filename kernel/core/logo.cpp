// DIT-OS Kernel - Boot logo

#include "core/logo.hpp"
#include "core/console.hpp"
#include "core/version.hpp"

namespace core
{

void logo_print()
{
    console_puts("\n  DIT-OS ");
    console_puts(version_string());
    console_puts("\n\n");
}

} // namespace core
