// DIT-OS Kernel - Command line parsing

#include "core/cmdline.hpp"
#include <stdint.h>

namespace core
{

static const char* s_cmdline = "";

void cmdline_set(const char* str)
{
    s_cmdline = str ? str : "";
}

const char* cmdline_get()
{
    return s_cmdline;
}

static bool match_option(const char* line, const char* opt, const char*& value)
{
    value = nullptr;
    while (*line) {
        while (*line == ' ')
            line++;
        if (!*line)
            break;
        const char* start = line;
        while (*line && *line != ' ' && *line != '=')
            line++;
        const char* end = line;
        const char* p = opt;
        while (start < end && *p && *start == *p) {
            start++;
            p++;
        }
        if (start == end && !*p) {
            if (*line == '=') {
                line++;
                value = line;
                while (*line && *line != ' ')
                    line++;
            }
            return true;
        }
        while (*line && *line != ' ')
            line++;
    }
    return false;
}

bool cmdline_has(const char* option)
{
    const char* val;
    return match_option(s_cmdline, option, val);
}

const char* cmdline_get_option(const char* option)
{
    const char* value;
    if (!match_option(s_cmdline, option, value))
        return nullptr;
    return value;
}

} // namespace core
