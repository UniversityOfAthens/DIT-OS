#ifndef KERNEL_CORE_CMDLINE_HPP
#define KERNEL_CORE_CMDLINE_HPP

namespace core
{

/** Set kernel command line (e.g. from bootloader). */
void cmdline_set(const char* str);

/** Get full command line. */
const char* cmdline_get();

/** Check if option is present (e.g. "quiet", "debug"). */
bool cmdline_has(const char* option);

/** Get value for option (e.g. "root=hd0" returns "hd0"). Returns nullptr if not
 * found. */
const char* cmdline_get_option(const char* option);

} // namespace core

#endif
