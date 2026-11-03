/* DIT-OS Userland - Shell line editor with tab completion
 *
 * Runs with the console in raw mode: the kernel neither echoes nor edits, so
 * every byte arrives here and this file does the echo, backspace, Ctrl+C and
 * Tab handling. The cursor always stays at the end of the line.
 */

#include "readline.h"
#include "syscall.h"

enum
{
    KEY_CTRL_C = 0x03,
    KEY_BACKSPACE = 0x08,
    KEY_TAB = 0x09,
    KEY_LF = 0x0A,
    KEY_CR = 0x0D,
    KEY_ESC = 0x1B,
    KEY_DELETE = 0x7F,
};

enum
{
    NAME_MAX_LEN = 256,
    PATH_MAX_LEN = 128,
    DIRENT_BUF_LEN = 512
};

/* Escape sequences (arrow keys etc.) are swallowed: ESC, then '[' or 'O',
 * then parameter bytes up to a final byte in 0x40-0x7E. */
enum esc_state
{
    ESC_NONE,
    ESC_START,
    ESC_BODY
};

static int s_raw;
static char s_pending[32]; /* bytes read but not yet consumed */
static int s_pending_len;
static int s_pending_pos;
static int s_last_was_cr; /* treat CR LF as one Enter */

/* Candidates for one completion: count, longest common prefix, and whether
 * the (single) match is a directory. */
struct match_set
{
    int count;
    int is_dir;
    int lcp_len;
    char lcp[NAME_MAX_LEN];
};

static int str_len(const char* s)
{
    int n = 0;
    while (s[n])
        n++;
    return n;
}

static int has_prefix(const char* s, const char* pfx, int pfx_len)
{
    for (int i = 0; i < pfx_len; i++)
    {
        if (s[i] != pfx[i])
            return 0;
    }
    return 1;
}

static void put(const char* s, int n)
{
    if (n > 0)
        (void)write(1, s, (unsigned)n);
}

static void put_str(const char* s)
{
    put(s, str_len(s));
}

void readline_init(void)
{
    s_raw = ioctl(0, TTY_IOC_SETMODE, 0) == 0;
}

/* Next input byte, or -1 on read error. A signal handler (the shell's
 * SIGCHLD reaper) interrupting the read is not an error: read again. */
static int next_byte(void)
{
    if (s_pending_pos == s_pending_len)
    {
        int n;
        do
            n = read(0, s_pending, sizeof(s_pending));
        while (n == -EINTR);
        if (n <= 0)
            return -1;
        s_pending_len = n;
        s_pending_pos = 0;
    }
    return (unsigned char)s_pending[s_pending_pos++];
}

static void match_add(struct match_set* m, const char* name, int is_dir)
{
    if (m->count == 0)
    {
        int i = 0;
        while (i < NAME_MAX_LEN - 1 && name[i])
        {
            m->lcp[i] = name[i];
            i++;
        }
        m->lcp_len = i;
    }
    else
    {
        int i = 0;
        while (i < m->lcp_len && m->lcp[i] == name[i])
            i++;
        m->lcp_len = i;
    }
    m->count++;
    m->is_dir = is_dir;
}

/* Collect (or, with @print, list) commands starting with @base. */
static void scan_commands(const char* const* commands, const char* base,
                          int base_len, struct match_set* m, int print)
{
    for (int i = 0; commands[i]; i++)
    {
        if (!has_prefix(commands[i], base, base_len))
            continue;
        if (print)
        {
            put_str(commands[i]);
            put_str("  ");
        }
        else
        {
            match_add(m, commands[i], 0);
        }
    }
}

/* Collect (or list) entries of directory @dir whose names start with @base.
 * Returns -1 if the directory cannot be opened or read. */
static int scan_dir(const char* dir, const char* base, int base_len,
                    struct match_set* m, int print)
{
    int fd = open(dir, 0);
    if (fd < 0)
        return -1;
    char buf[DIRENT_BUF_LEN];
    int rc = 0;
    for (;;)
    {
        int n = getdents(fd, buf, sizeof(buf));
        if (n < 0)
            rc = -1;
        if (n <= 0)
            break;
        for (int off = 0; off + DIT_DIRENT_SIZE <= n; off += DIT_DIRENT_SIZE)
        {
            const struct dit_dirent* de = (const struct dit_dirent*)(buf + off);
            if (!has_prefix(de->d_name, base, base_len))
                continue;
            int is_dir = de->d_type == DIT_DT_DIR;
            if (print)
            {
                put_str(de->d_name);
                put_str(is_dir ? "/  " : "  ");
            }
            else
            {
                match_add(m, de->d_name, is_dir);
            }
        }
    }
    (void)close(fd);
    return rc;
}

/* Directory to scan for the part of @word before its last '/': the working
 * directory (".") for a bare name, otherwise that prefix as typed, which the
 * kernel resolves against the working directory when it is relative. */
static void completion_dir(char* out, const char* word, int dir_len)
{
    int o = 0;
    if (dir_len == 0)
        out[o++] = '.';
    for (int i = 0; i < dir_len && o < PATH_MAX_LEN - 1; i++)
        out[o++] = word[i];
    out[o] = '\0';
}

static int append(char* buf, int* len, int cap, const char* s, int n)
{
    if (*len + n > cap - 1)
        return 0;
    for (int i = 0; i < n; i++)
        buf[(*len)++] = s[i];
    buf[*len] = '\0';
    put(s, n);
    return 1;
}

static void complete(char* buf, int* len, int cap, const char* prompt,
                     const char* const* commands)
{
    int start = *len;
    while (start > 0 && buf[start - 1] != ' ' && buf[start - 1] != '\t')
        start--;
    int first_word = 1;
    for (int i = 0; i < start; i++)
    {
        if (buf[i] != ' ' && buf[i] != '\t')
            first_word = 0;
    }

    const char* word = buf + start;
    int word_len = *len - start;
    const char* base = word;
    int base_len = word_len;
    char dir[PATH_MAX_LEN];

    struct match_set m;
    m.count = 0;
    m.is_dir = 0;
    m.lcp_len = 0;
    if (first_word)
    {
        scan_commands(commands, base, base_len, &m, 0);
    }
    else
    {
        int dir_len = word_len;
        while (dir_len > 0 && word[dir_len - 1] != '/')
            dir_len--;
        base = word + dir_len;
        base_len = word_len - dir_len;
        completion_dir(dir, word, dir_len);
        (void)scan_dir(dir, base, base_len, &m, 0);
    }

    if (m.count == 0)
    {
        put_str("\a");
        return;
    }

    int added = m.lcp_len - base_len;
    if (added > 0 && !append(buf, len, cap, m.lcp + base_len, added))
    {
        put_str("\a");
        return;
    }
    if (m.count == 1)
    {
        (void)append(buf, len, cap, m.is_dir ? "/" : " ", 1);
        return;
    }
    if (added > 0)
        return;

    /* Ambiguous and nothing more in common: list the candidates, then
     * redraw the prompt and the line. */
    put_str("\r\n");
    if (first_word)
        scan_commands(commands, base, base_len, &m, 1);
    else
        (void)scan_dir(dir, base, base_len, &m, 1);
    put_str("\r\n");
    put_str(prompt);
    put(buf, *len);
}

int readline(const char* prompt, char* buf, int cap,
             const char* const* commands)
{
    put_str(prompt);
    if (!s_raw)
    {
        int n;
        do
            n = read(0, buf, (unsigned)(cap - 1));
        while (n == -EINTR);
        if (n < 0)
            return -1;
        buf[n] = '\0';
        return n;
    }

    int len = 0;
    enum esc_state esc = ESC_NONE;
    buf[0] = '\0';
    for (;;)
    {
        int c = next_byte();
        if (c < 0)
            return -1;
        int was_cr = s_last_was_cr;
        s_last_was_cr = c == KEY_CR;

        if (esc == ESC_START)
        {
            esc = (c == '[' || c == 'O') ? ESC_BODY : ESC_NONE;
            continue;
        }
        if (esc == ESC_BODY)
        {
            if (c >= 0x40 && c <= 0x7E)
                esc = ESC_NONE;
            continue;
        }

        switch (c)
        {
        case KEY_LF:
            if (was_cr)
                break; /* second half of CR LF */
            /* fall through */
        case KEY_CR:
            put_str("\r\n");
            return len;
        case KEY_BACKSPACE:
        case KEY_DELETE:
            if (len > 0)
            {
                buf[--len] = '\0';
                put_str("\b \b");
            }
            break;
        case KEY_CTRL_C:
            put_str("^C\r\n");
            len = 0;
            buf[0] = '\0';
            put_str(prompt);
            break;
        case KEY_TAB:
            complete(buf, &len, cap, prompt, commands);
            break;
        case KEY_ESC:
            esc = ESC_START;
            break;
        default:
            if (c >= 0x20 && c < KEY_DELETE)
            {
                char ch = (char)c;
                if (!append(buf, &len, cap, &ch, 1))
                    put_str("\a");
            }
            break;
        }
    }
}
