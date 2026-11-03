// DIT-OS Kernel - Version information

#include "core/version.hpp"

namespace core
{

static const char* const s_version = "DIT-OS 0.0.11-alpha";

const char* version_string()
{
    return s_version;
}
unsigned int version_major()
{
    return 0;
}
unsigned int version_minor()
{
    return 0;
}
unsigned int version_patch()
{
    return 11;
}

} // namespace core
