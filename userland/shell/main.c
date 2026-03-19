// DIT-OS Userland - Minimal shell loop with Bash-like prompt customization

#include "syscall.h"

enum { MAX_VAR = 64, MAX_LINE = 128, MAX_PROMPT = 256 };

static char g_user[MAX_VAR] = "user";
static char g_host[MAX_VAR] = "dit";
static char g_pwd[MAX_VAR] = "~";
static char g_ps1[MAX_VAR] = "\\u@\\h:\\w\\$ ";

static inline void outw(unsigned short port, unsigned short val) {
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

static void poweroff_qemu(void) {
    outw(0x604, 0x2000); /* QEMU/Bochs ACPI poweroff */
    for (;;) __asm__ volatile("hlt");
}

static int str_len(const char* s) {
    int n = 0;
    while (s[n]) n++;
    return n;
}

static int starts_with(const char* s, const char* pfx) {
    while (*pfx) {
        if (*s++ != *pfx++) return 0;
    }
    return 1;
}

static void write_str(const char* s) {
    int n = str_len(s);
    if (n > 0) (void)write(1, s, (unsigned)n);
}

static int str_eq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++;
        b++;
    }
    return *a == *b;
}

static void copy_with_unescape(char* dst, int dst_cap, const char* src) {
    int di = 0;
    for (int si = 0; src[si] && di < dst_cap - 1; si++) {
        if (src[si] == '\\' && src[si + 1]) {
            si++;
            char esc = src[si];
            if (esc == 'e') dst[di++] = 27;
            else if (esc == 'n') dst[di++] = '\n';
            else if (esc == 'r') dst[di++] = '\r';
            else if (esc == '\\') dst[di++] = '\\';
            else {
                /* Preserve prompt tokens like \u, \h, \w, \$ */
                if (di < dst_cap - 2) {
                    dst[di++] = '\\';
                    dst[di++] = esc;
                } else {
                    dst[di++] = esc;
                }
            }
        } else {
            dst[di++] = src[si];
        }
    }
    dst[di] = '\0';
}

static void set_var(const char* name, const char* value) {
    if (str_eq(name, "USER")) copy_with_unescape(g_user, MAX_VAR, value);
    else if (str_eq(name, "HOST")) copy_with_unescape(g_host, MAX_VAR, value);
    else if (str_eq(name, "PWD")) copy_with_unescape(g_pwd, MAX_VAR, value);
    else if (str_eq(name, "PS1")) copy_with_unescape(g_ps1, MAX_VAR, value);
}

static void render_prompt(char* out, int out_cap) {
    int oi = 0;
    for (int i = 0; g_ps1[i] && oi < out_cap - 1; i++) {
        if (g_ps1[i] == '\\' && g_ps1[i + 1]) {
            char t = g_ps1[++i];
            const char* repl = 0;
            char one[2];
            if (t == 'u') repl = g_user;
            else if (t == 'h') repl = g_host;
            else if (t == 'w') repl = g_pwd;
            else if (t == '$') {
                one[0] = '$';
                one[1] = '\0';
                repl = one;
            } else if (t == 'e') {
                out[oi++] = 27;
                continue;
            } else {
                one[0] = t;
                one[1] = '\0';
                repl = one;
            }
            for (int k = 0; repl[k] && oi < out_cap - 1; k++) out[oi++] = repl[k];
            continue;
        }
        out[oi++] = g_ps1[i];
    }
    out[oi] = '\0';
}

static void apply_export_line(const char* line) {
    const char* p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (!starts_with(p, "export ")) return;
    p += 7;

    const char* eq = p;
    while (*eq && *eq != '=') eq++;
    if (*eq != '=') return;

    char name[16];
    int ni = 0;
    while (p < eq && ni < (int)sizeof(name) - 1) name[ni++] = *p++;
    name[ni] = '\0';

    const char* value = eq + 1;
    int len = str_len(value);
    if (len >= 2 && value[0] == '"' && value[len - 1] == '"') {
        static char tmp[MAX_VAR];
        int ti = 0;
        for (int i = 1; i < len - 1 && ti < MAX_VAR - 1; i++) tmp[ti++] = value[i];
        tmp[ti] = '\0';
        set_var(name, tmp);
        return;
    }
    set_var(name, value);
}

static void load_ditrc_defaults(void) {
    /* Fallback .ditrc profile until userspace file-open syscall exists */
    static const char* rc_lines[] = {
        "export USER=user",
        "export HOST=dit",
        "export PWD=~",
        "export PS1=\\e[1;32m\\u@\\h\\e[0m:\\e[1;34m\\w\\e[0m\\$ ",
        0
    };
    for (int i = 0; rc_lines[i]; i++) apply_export_line(rc_lines[i]);
}

void userland_shell_main(void) {
    char buf[MAX_LINE];
    char prompt[MAX_PROMPT];

    load_ditrc_defaults();
    render_prompt(prompt, MAX_PROMPT);
    write_str(prompt);

    for (;;) {
        int n = read(0, buf, sizeof(buf) - 1);
        if (n <= 0) continue;

        buf[n] = '\0';
        if (n > 0 && buf[n - 1] == '\n') buf[n - 1] = '\0';
        if (n > 1 && buf[n - 2] == '\r') buf[n - 2] = '\0';

        if (starts_with(buf, "exit")) {
            write_str("Bye.\r\n");
            poweroff_qemu();
        } else if (starts_with(buf, "echo")) {
            const char* msg = buf + 4;
            while (*msg == ' ' || *msg == '\t') msg++;
            int len = str_len(msg);
            if (len >= 2 && msg[0] == '"' && msg[len - 1] == '"') {
                msg++;
                len -= 2;
            }
            if (len > 0) (void)write(1, msg, (unsigned)len);
            write_str("\r\n");
        } else if (starts_with(buf, "export ")) {
            apply_export_line(buf);
        }

        render_prompt(prompt, MAX_PROMPT);
        write_str(prompt);
    }
}

