// DIT-OS Kernel - Version information

#include "core/version.hpp"

namespace core
{

static const char* const s_version = "DIT-OS 0.0.9-alpha";

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
    return 9;
}
unsigned int version_patch()
{
    return 0;
}

} // namespace core
