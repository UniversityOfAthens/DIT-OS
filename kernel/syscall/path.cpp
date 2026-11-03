// DIT-OS Kernel - Path resolution for syscalls (working directory)

#include "syscall/path.hpp"
#include "sched/sched.hpp"
#include "sched/task.hpp"
#include "mm/uaccess.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

namespace
{

/* Append one component to the absolute path in @out (length *@len).
 * Returns false when it does not fit (NUL included). */
bool append_component(char* out, uint32_t cap, uint32_t* len, const char* comp,
                      uint32_t comp_len)
{
    uint32_t sep = out[*len - 1] == '/' ? 0 : 1;
    if (comp_len > cap || *len + sep + comp_len >= cap)
        return false;
    if (sep)
        out[(*len)++] = '/';
    for (uint32_t i = 0; i < comp_len; i++)
        out[(*len)++] = comp[i];
    out[*len] = '\0';
    return true;
}

/* Drop the last component of the absolute path in @out, keeping "/". */
void drop_component(char* out, uint32_t* len)
{
    while (*len > 1 && out[*len - 1] != '/')
        (*len)--;
    if (*len > 1)
        (*len)--; /* the separator before it */
    out[*len] = '\0';
}

/* Fold the components of @path into @out (already absolute). */
bool fold(char* out, uint32_t cap, uint32_t* len, const char* path)
{
    const char* p = path;
    while (*p)
    {
        while (*p == '/')
            p++;
        const char* comp = p;
        uint32_t comp_len = 0;
        while (comp[comp_len] && comp[comp_len] != '/')
            comp_len++;
        p = comp + comp_len;
        if (comp_len == 0 || (comp_len == 1 && comp[0] == '.'))
            continue;
        if (comp_len == 2 && comp[0] == '.' && comp[1] == '.')
        {
            drop_component(out, len);
            continue;
        }
        if (!append_component(out, cap, len, comp, comp_len))
            return false;
    }
    return true;
}

} // namespace

int path_normalize(char* out, uint32_t cap, const char* base, const char* path)
{
    if (!out || cap < 2 || !path)
        return -ENAMETOOLONG;
    if (path[0] == '\0')
        return -ENOENT;
    out[0] = '/';
    out[1] = '\0';
    uint32_t len = 1;
    /* The base is trusted (the task's cwd), but fold it like any path so a
     * corrupt one can never escape the bounds of @out. */
    if (path[0] != '/' && base && !fold(out, cap, &len, base))
        return -ENAMETOOLONG;
    if (!fold(out, cap, &len, path))
        return -ENAMETOOLONG;
    return static_cast<int>(len);
}

int path_from_user(char* out, uint32_t cap, uintptr_t user_path)
{
    char raw[PATH_MAX];
    int length = mm::strncpy_from_user(raw, user_path, sizeof(raw));
    if (length < 0)
        return length;
    if (length >= static_cast<int>(sizeof(raw)) - 1)
        return -ENAMETOOLONG;
    sched::Task* t = sched::current_task;
    return path_normalize(out, cap, t ? t->cwd : nullptr, raw);
}

} // namespace syscall
