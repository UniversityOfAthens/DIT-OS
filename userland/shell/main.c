// DIT-OS Userland - Minimal shell loop with Bash-like prompt customization

#include "readline.h"
#include "syscall.h"

enum
{
    MAX_VAR = 64,
    MAX_LINE = 128,
    MAX_PROMPT = 256,
    MAX_PATH = 128
};

static char g_user[MAX_VAR] = "user";
static char g_host[MAX_VAR] = "dit";
static char g_pwd[MAX_VAR] = "~";
static char g_ps1[MAX_VAR] = "\\u@\\h:\\w\\$ ";

/* Builtins offered by tab completion (keep in sync with the dispatch in
 * userland_shell_main). */
static const char* const s_commands[] = {
    "cat", "echo", "exit", "export", "ls", 0,
};

static void poweroff_qemu(void)
{
    (void)ioctl(1, TTY_IOC_POWER_OFF, 0);
    for (;;)
        __asm__ volatile("hlt");
}

/* Children whose parent exited are reparented to PID 1 (this process), so
 * drain them before each prompt (M40). */
static void reap_children(void)
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0)
    {
        /* Status is intentionally discarded; the zombie is released. */
    }
}

static int str_len(const char* s)
{
    int n = 0;
    while (s[n])
        n++;
    return n;
}

static int starts_with(const char* s, const char* pfx)
{
    while (*pfx)
    {
        if (*s++ != *pfx++)
            return 0;
    }
    return 1;
}

/** Exact command word @cmd followed by end or whitespace. */
static int cmd_is(const char* buf, const char* cmd)
{
    const char* p = buf;
    while (*cmd)
    {
        if (*p++ != *cmd++)
            return 0;
    }
    return *p == '\0' || *p == ' ' || *p == '\t';
}

static void write_str(const char* s)
{
    int n = str_len(s);
    if (n > 0)
        (void)write(1, s, (unsigned)n);
}

static int str_eq(const char* a, const char* b)
{
    while (*a && *b)
    {
        if (*a != *b)
            return 0;
        a++;
        b++;
    }
    return *a == *b;
}

static void copy_with_unescape(char* dst, int dst_cap, const char* src)
{
    int di = 0;
    for (int si = 0; di < dst_cap - 1 && src[si]; si++)
    {
        if (src[si] == '\\' && src[si + 1])
        {
            si++;
            char esc = src[si];
            if (esc == 'e')
                dst[di++] = 27;
            else if (esc == 'n')
                dst[di++] = '\n';
            else if (esc == 'r')
                dst[di++] = '\r';
            else if (esc == '\\')
                dst[di++] = '\\';
            else
            {
                /* Preserve prompt tokens like \u, \h, \w, \$ */
                if (di < dst_cap - 2)
                {
                    dst[di++] = '\\';
                    dst[di++] = esc;
                }
                else
                {
                    dst[di++] = esc;
                }
            }
        }
        else
        {
            dst[di++] = src[si];
        }
    }
    dst[di] = '\0';
}

static void set_var(const char* name, const char* value)
{
    if (str_eq(name, "USER"))
        copy_with_unescape(g_user, MAX_VAR, value);
    else if (str_eq(name, "HOST"))
        copy_with_unescape(g_host, MAX_VAR, value);
    else if (str_eq(name, "PWD"))
        copy_with_unescape(g_pwd, MAX_VAR, value);
    else if (str_eq(name, "PS1"))
        copy_with_unescape(g_ps1, MAX_VAR, value);
}

static void render_prompt(char* out, int out_cap)
{
    int oi = 0;
    for (int i = 0; oi < out_cap - 1 && g_ps1[i]; i++)
    {
        if (g_ps1[i] == '\\' && g_ps1[i + 1])
        {
            char t = g_ps1[++i];
            const char* repl = 0;
            char one[2];
            if (t == 'u')
                repl = g_user;
            else if (t == 'h')
                repl = g_host;
            else if (t == 'w')
                repl = g_pwd;
            else if (t == '$')
            {
                one[0] = '$';
                one[1] = '\0';
                repl = one;
            }
            else if (t == 'e')
            {
                out[oi++] = 27;
                continue;
            }
            else
            {
                one[0] = t;
                one[1] = '\0';
                repl = one;
            }
            for (int k = 0; oi < out_cap - 1 && repl[k]; k++)
                out[oi++] = repl[k];
            continue;
        }
        out[oi++] = g_ps1[i];
    }
    out[oi] = '\0';
}

static void apply_export_line(const char* line)
{
    const char* p = line;
    while (*p == ' ' || *p == '\t')
        p++;
    if (!starts_with(p, "export "))
        return;
    p += 7;

    const char* eq = p;
    while (*eq && *eq != '=')
        eq++;
    if (*eq != '=')
        return;

    char name[16];
    int ni = 0;
    while (p < eq && ni < (int)sizeof(name) - 1)
        name[ni++] = *p++;
    name[ni] = '\0';

    const char* value = eq + 1;
    int len = str_len(value);
    if (len >= 2 && value[0] == '"' && value[len - 1] == '"')
    {
        static char tmp[MAX_VAR];
        int ti = 0;
        for (int i = 1; i < len - 1 && ti < MAX_VAR - 1; i++)
            tmp[ti++] = value[i];
        tmp[ti] = '\0';
        set_var(name, tmp);
        return;
    }
    set_var(name, value);
}

static void skip_ws(const char** p)
{
    while (**p == ' ' || **p == '\t' || **p == '\r' || **p == '\n')
        (*p)++;
}

/** Strip CR/LF and trailing spaces from a full input line (serial often ends
 * with \r only). */
static void trim_input_line(char* buf)
{
    int n = str_len(buf);
    while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r' ||
                     buf[n - 1] == ' ' || buf[n - 1] == '\t'))
    {
        buf[n - 1] = '\0';
        n--;
    }
}

static void trim_trailing_path(char* s)
{
    int n = str_len(s);
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\r' ||
                     s[n - 1] == '\n'))
    {
        s[n - 1] = '\0';
        n--;
    }
}

/** Absolute path under ramfs root (leading slash). */
static void normalize_path(char* out, int cap, const char* path)
{
    if (!path || !path[0])
    {
        if (cap > 1)
        {
            out[0] = '/';
            out[1] = '\0';
        }
        return;
    }
    while (*path == ' ' || *path == '\t' || *path == '\r' || *path == '\n')
        path++;
    if (!path[0])
    {
        if (cap > 1)
        {
            out[0] = '/';
            out[1] = '\0';
        }
        return;
    }
    /* Relative paths: strip leading "./" segments (./hello -> hello under /).
     */
    while (path[0] == '.' && path[1] == '/')
        path += 2;
    if (!path[0])
    {
        if (cap > 1)
        {
            out[0] = '/';
            out[1] = '\0';
        }
        return;
    }
    if (path[0] == '/')
    {
        int i = 0;
        while (i < cap - 1 && path[i])
        {
            out[i] = path[i];
            i++;
        }
        out[i] = '\0';
        trim_trailing_path(out);
        return;
    }
    if (cap < 2)
        return;
    out[0] = '/';
    int i = 1;
    int j = 0;
    while (i < cap - 1 && path[j])
        out[i++] = path[j++];
    out[i] = '\0';
    trim_trailing_path(out);
}

static void cmd_ls(const char* arg)
{
    char pathbuf[MAX_PATH];
    if (!arg || !arg[0])
        normalize_path(pathbuf, sizeof(pathbuf), "");
    else
        normalize_path(pathbuf, sizeof(pathbuf), arg);

    int fd = open(pathbuf, 0);
    if (fd < 0)
    {
        write_str("ls: cannot open directory\r\n");
        return;
    }
    char buf[512];
    for (;;)
    {
        int n = getdents(fd, buf, sizeof(buf));
        if (n <= 0)
            break;
        int off = 0;
        while (off + (int)sizeof(struct dit_dirent) <= n)
        {
            const struct dit_dirent* de = (const struct dit_dirent*)(buf + off);
            write_str(de->d_name);
            write_str("\r\n");
            off += (int)sizeof(struct dit_dirent);
        }
    }
    (void)close(fd);
}

static void cmd_cat(const char* arg)
{
    if (!arg || !arg[0])
    {
        write_str("cat: missing operand\r\n");
        return;
    }
    char pathbuf[MAX_PATH];
    normalize_path(pathbuf, sizeof(pathbuf), arg);

    int fd = open(pathbuf, 0);
    if (fd < 0)
    {
        write_str("cat: cannot open file\r\n");
        return;
    }
    char chunk[256];
    for (;;)
    {
        int n = read(fd, chunk, sizeof(chunk));
        if (n <= 0)
            break;
        (void)write(1, chunk, (unsigned)n);
    }
    (void)close(fd);
}

static void load_ditrc_defaults(void)
{
    /* Fallback .ditrc profile until userspace file-open syscall exists */
    static const char* rc_lines[] = {
        "export USER=user", "export HOST=dit", "export PWD=~",
        "export PS1=\\e[1;32m\\u@\\h\\e[0m:\\e[1;34m\\w\\e[0m\\$ ", 0
    };
    for (int i = 0; rc_lines[i]; i++)
        apply_export_line(rc_lines[i]);
}

void userland_shell_main(void)
{
    char buf[MAX_LINE];
    char prompt[MAX_PROMPT];

    load_ditrc_defaults();
    readline_init();

    for (;;)
    {
        reap_children();
        render_prompt(prompt, MAX_PROMPT);
        if (readline(prompt, buf, sizeof(buf), s_commands) < 0)
            continue;
        trim_input_line(buf);

        if (cmd_is(buf, "exit"))
        {
            write_str("Bye.\r\n");
            poweroff_qemu();
        }
        else if (cmd_is(buf, "ls"))
        {
            const char* rest = buf + 2;
            skip_ws(&rest);
            cmd_ls(rest);
        }
        else if (cmd_is(buf, "cat"))
        {
            const char* rest = buf + 3;
            skip_ws(&rest);
            cmd_cat(rest);
        }
        else if (starts_with(buf, "echo"))
        {
            const char* msg = buf + 4;
            while (*msg == ' ' || *msg == '\t')
                msg++;
            int len = str_len(msg);
            if (len >= 2 && msg[0] == '"' && msg[len - 1] == '"')
            {
                msg++;
                len -= 2;
            }
            if (len > 0)
                (void)write(1, msg, (unsigned)len);
            write_str("\r\n");
        }
        else if (starts_with(buf, "export "))
        {
            apply_export_line(buf);
        }
    }
}
