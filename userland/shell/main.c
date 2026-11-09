// DIT-OS Userland - Shell: builtins, programs, pipelines and job control
//
// The shell is PID 1's interactive half. Programs run as jobs: each job is
// a process group (one process per pipeline stage) that owns the console
// while it is in the foreground. A SIGCHLD handler records every state
// change, and also reaps orphans the kernel reparents to PID 1 (M41).

#include "readline.h"
#include "signal.h"
#include "stdio.h"
#include "syscall.h"

enum
{
    MAX_VAR = 64,
    MAX_LINE = 128,
    MAX_PROMPT = 256,
    MAX_PATH = 128,
    MAX_ARGS = 8, /* per pipeline stage, not counting the NULL */
    MAX_STAGES = 4,
    MAX_JOBS = 8,
    JOB_CMD_LEN = 48,
    TOKEN_STORE = 2 * MAX_LINE,
    EXIT_NOT_FOUND = 127
};

static char g_user[MAX_VAR] = "user";
static char g_host[MAX_VAR] = "dit";
static char g_ps1[MAX_VAR] = "\\u@\\h:\\w\\$ ";

/* Builtins offered by tab completion (keep in sync with run_builtin and
 * run_child_builtin). */
static const char* const s_commands[] = {
    "cat", "cd", "echo", "exit", "export", "bg", "fg", "jobs", "ls", "pwd", 0,
};

static void poweroff_qemu(void)
{
    (void)ioctl(1, TTY_IOC_POWER_OFF, 0);
    for (;;)
        __asm__ volatile("hlt");
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

static void write_str(const char* s)
{
    int n = str_len(s);
    if (n > 0)
        (void)write(1, s, (unsigned)n);
}

/* Diagnostics go to fd 2 so a redirection of stdout (planned) cannot hide
 * them; fd 2 is the console like 0 and 1. */
static void write_err(const char* s)
{
    int n = str_len(s);
    if (n > 0)
        (void)write(2, s, (unsigned)n);
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
            {
                /* The kernel's working directory, read for every prompt. */
                static char cwd[MAX_PATH];
                if (getcwd(cwd, sizeof(cwd)) < 0)
                {
                    cwd[0] = '?';
                    cwd[1] = '\0';
                }
                repl = cwd;
            }
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

/* ---- Parsing ------------------------------------------------------------ */

/* One command line: up to MAX_STAGES commands joined by '|', optionally
 * ending in '&'. Token strings live in @store. */
struct pipeline
{
    int nstages;
    int background;
    char* argv[MAX_STAGES][MAX_ARGS + 1];
    char store[TOKEN_STORE];
};

static int is_space(char c)
{
    return c == ' ' || c == '\t';
}

/* Split @line into a pipeline. Double quotes group words (and are
 * removed). Returns 0, or -1 on a syntax error or a line that does not fit
 * the limits. An empty line is 0 stages. */
static int parse_line(const char* line, struct pipeline* pl)
{
    int used = 0;
    int argc = 0;
    pl->nstages = 0;
    pl->background = 0;
    const char* p = line;
    for (;;)
    {
        while (is_space(*p))
            p++;
        char c = *p;
        if (c == '\0' || c == '|' || c == '&')
        {
            if (argc == 0)
                return (c == '\0' && pl->nstages == 0) ? 0 : -1;
            pl->argv[pl->nstages][argc] = 0;
            pl->nstages++;
            argc = 0;
            if (c == '\0')
                return 0;
            p++;
            if (c == '&')
            {
                while (is_space(*p))
                    p++;
                if (*p)
                    return -1; /* '&' only at the end */
                pl->background = 1;
                return 0;
            }
            if (pl->nstages == MAX_STAGES)
                return -1;
            continue;
        }
        if (argc == MAX_ARGS)
            return -1;
        char* tok = pl->store + used;
        if (c == '"')
        {
            p++;
            while (*p && *p != '"')
            {
                if (used >= TOKEN_STORE - 1)
                    return -1;
                pl->store[used++] = *p++;
            }
            if (*p != '"')
                return -1; /* unterminated quote */
            p++;
        }
        else
        {
            while (*p && !is_space(*p) && *p != '|' && *p != '&' && *p != '"')
            {
                if (used >= TOKEN_STORE - 1)
                    return -1;
                pl->store[used++] = *p++;
            }
        }
        if (used >= TOKEN_STORE)
            return -1;
        pl->store[used++] = '\0';
        pl->argv[pl->nstages][argc++] = tok;
    }
}

/* ---- Jobs --------------------------------------------------------------- */

enum proc_state
{
    PROC_RUNNING,
    PROC_STOPPED,
    PROC_DONE
};

struct job
{
    int used;
    int id;
    int pgid;
    int nprocs;
    int pids[MAX_STAGES];
    int states[MAX_STAGES];
    int status; /* wait status of the last stage */
    int stop_reported;
    int background;
    char cmd[JOB_CMD_LEN];
};

/* Written by the SIGCHLD handler; the main code blocks SIGCHLD while it
 * reads or changes the table. */
static struct job g_jobs[MAX_JOBS];
static sigset_t g_base_mask; /* the shell's normal mask (SIGCHLD open) */

static void block_sigchld(sigset_t* old)
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    (void)sigprocmask(SIG_BLOCK, &set, old);
}

static void restore_mask(const sigset_t* old)
{
    (void)sigprocmask(SIG_SETMASK, old, 0);
}

static int job_state(const struct job* j)
{
    int running = 0;
    int stopped = 0;
    for (int i = 0; i < j->nprocs; i++)
    {
        if (j->states[i] == PROC_RUNNING)
            running = 1;
        else if (j->states[i] == PROC_STOPPED)
            stopped = 1;
    }
    if (running)
        return PROC_RUNNING;
    return stopped ? PROC_STOPPED : PROC_DONE;
}

/* Record a waitpid() result. Children in no job are orphans the kernel
 * reparented to PID 1: reaping them is all that is needed (M41). */
static void record_status(int pid, int status)
{
    for (int j = 0; j < MAX_JOBS; j++)
    {
        struct job* job = &g_jobs[j];
        if (!job->used)
            continue;
        for (int k = 0; k < job->nprocs; k++)
        {
            if (job->pids[k] != pid)
                continue;
            if (WIFSTOPPED(status))
            {
                job->states[k] = PROC_STOPPED;
                job->stop_reported = 0;
            }
            else if (WIFCONTINUED(status))
            {
                job->states[k] = PROC_RUNNING;
            }
            else
            {
                job->states[k] = PROC_DONE;
                if (k == job->nprocs - 1)
                    job->status = status;
            }
            return;
        }
    }
}

static void on_sigchld(int sig)
{
    (void)sig;
    for (;;)
    {
        int status = 0;
        int pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
        if (pid <= 0)
            break;
        record_status(pid, status);
    }
}

static struct job* job_alloc(void)
{
    int next_id = 1;
    struct job* free_slot = 0;
    for (int j = 0; j < MAX_JOBS; j++)
    {
        if (g_jobs[j].used && g_jobs[j].id >= next_id)
            next_id = g_jobs[j].id + 1;
        if (!g_jobs[j].used && !free_slot)
            free_slot = &g_jobs[j];
    }
    if (!free_slot)
        return 0;
    for (unsigned i = 0; i < sizeof(*free_slot); i++)
        ((char*)free_slot)[i] = 0;
    free_slot->used = 1;
    free_slot->id = next_id;
    return free_slot;
}

/* The job named by "%N" or "N", or the most recent one for NULL. */
static struct job* job_find(const char* spec)
{
    int want = 0;
    if (spec)
    {
        if (*spec == '%')
            spec++;
        for (; *spec >= '0' && *spec <= '9'; spec++)
            want = want * 10 + (*spec - '0');
        if (*spec || want <= 0)
            return 0;
    }
    struct job* best = 0;
    for (int j = 0; j < MAX_JOBS; j++)
    {
        struct job* job = &g_jobs[j];
        if (!job->used)
            continue;
        if (want ? job->id == want : (!best || job->id > best->id))
            best = job;
    }
    return best;
}

static void print_status_word(int status)
{
    if (WIFEXITED(status))
    {
        if (WEXITSTATUS(status) == 0)
            printf("Done");
        else
            printf("Exit %d", WEXITSTATUS(status));
        return;
    }
    int sig = WTERMSIG(status);
    if (sig == SIGSEGV)
        printf("Segmentation fault");
    else if (sig == SIGKILL)
        printf("Killed");
    else if (sig == SIGTERM)
        printf("Terminated");
    else if (sig == SIGINT)
        printf("Interrupt");
    else
        printf("Signal %d", sig);
}

/* Report and free finished jobs, report new stops. SIGCHLD blocked. */
static void notify_jobs(void)
{
    for (int j = 0; j < MAX_JOBS; j++)
    {
        struct job* job = &g_jobs[j];
        if (!job->used)
            continue;
        int state = job_state(job);
        if (state == PROC_DONE)
        {
            printf("[%d]+  ", job->id);
            print_status_word(job->status);
            printf("  %s\r\n", job->cmd);
            job->used = 0;
        }
        else if (state == PROC_STOPPED && !job->stop_reported)
        {
            printf("[%d]+  Stopped  %s\r\n", job->id, job->cmd);
            job->stop_reported = 1;
        }
    }
}

/* Give the console to @job and wait until it stops or finishes. With
 * @resume, continue a stopped job first. SIGCHLD must be blocked. */
static void wait_foreground(struct job* job, int resume)
{
    job->background = 0;
    /* The job gets a normal terminal: line editing, echo and ^C/^Z. */
    (void)ioctl(0, TTY_IOC_SETMODE, TTY_MODE_DEFAULT);
    (void)tcsetpgrp(0, job->pgid);
    if (resume)
    {
        for (int k = 0; k < job->nprocs; k++)
            if (job->states[k] == PROC_STOPPED)
                job->states[k] = PROC_RUNNING;
        (void)kill(-job->pgid, SIGCONT);
    }
    while (job_state(job) == PROC_RUNNING)
        (void)sigsuspend(&g_base_mask);
    (void)tcsetpgrp(0, getpgrp());
    readline_init();

    if (job_state(job) == PROC_STOPPED)
    {
        printf("\r\n[%d]+  Stopped  %s\r\n", job->id, job->cmd);
        job->stop_reported = 1;
        job->background = 1;
        return;
    }
    /* A finished foreground job is not announced, except for deaths the
     * user did not ask for (^C already echoed itself). */
    if (WIFSIGNALED(job->status) && WTERMSIG(job->status) != SIGINT)
    {
        print_status_word(job->status);
        printf("\r\n");
    }
    job->used = 0;
}

/* ---- Builtins ----------------------------------------------------------- */

static void cmd_ls(const char* path)
{
    const char* name = path ? path : ".";
    int fd = open(name, 0);
    if (fd < 0)
    {
        write_str("ls: cannot open directory\r\n");
        return;
    }
    char buf[512];
    for (;;)
    {
        int n = getdents(fd, buf, sizeof(buf));
        if (n < 0)
        {
            /* -ENOTDIR when the operand is a file: an error, not EOF. */
            write_err("ls: ");
            write_err(name);
            write_err(": cannot read directory\r\n");
            break;
        }
        if (n == 0)
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

/* Copy @fd to stdout until EOF; @prog prefixes a read error on stderr. */
static void copy_fd(int fd, const char* prog)
{
    char chunk[256];
    for (;;)
    {
        int n = read(fd, chunk, sizeof(chunk));
        if (n == -EINTR)
            continue;
        if (n < 0)
        {
            write_err(prog);
            write_err(": read error\r\n");
            break;
        }
        if (n == 0)
            break;
        (void)write(1, chunk, (unsigned)n);
    }
}

static void cmd_cat(const char* path)
{
    int fd = open(path, 0);
    if (fd < 0)
    {
        write_str("cat: cannot open file\r\n");
        return;
    }
    copy_fd(fd, "cat");
    (void)close(fd);
}

static void cmd_echo(char* const* argv)
{
    for (int i = 1; argv[i]; i++)
    {
        if (i > 1)
            write_str(" ");
        write_str(argv[i]);
    }
    write_str("\r\n");
}

static void cmd_pwd(void)
{
    char cwd[MAX_PATH];
    if (getcwd(cwd, sizeof(cwd)) < 0)
    {
        write_str("pwd: cannot read the working directory\r\n");
        return;
    }
    write_str(cwd);
    write_str("\r\n");
}

static void cmd_cd(const char* path)
{
    int r = chdir(path ? path : "/");
    if (r == 0)
        return;
    printf("cd: %s: %s\r\n", path ? path : "/",
           r == -ENOTDIR ? "Not a directory" : "No such file or directory");
}

static void cmd_jobs(void)
{
    for (int j = 0; j < MAX_JOBS; j++)
    {
        struct job* job = &g_jobs[j];
        if (!job->used)
            continue;
        int state = job_state(job);
        const char* word = state == PROC_RUNNING ? "Running" :
                           state == PROC_STOPPED ? "Stopped" :
                                                   "Done";
        printf("[%d]+  %s  %s\r\n", job->id, word, job->cmd);
        if (state == PROC_STOPPED)
            job->stop_reported = 1;
    }
}

static void cmd_fg_bg(const char* spec, int foreground)
{
    struct job* job = job_find(spec);
    if (!job)
    {
        printf("%s: no such job\r\n", foreground ? "fg" : "bg");
        return;
    }
    if (foreground)
    {
        printf("%s\r\n", job->cmd);
        wait_foreground(job, 1);
        return;
    }
    for (int k = 0; k < job->nprocs; k++)
        if (job->states[k] == PROC_STOPPED)
            job->states[k] = PROC_RUNNING;
    job->background = 1;
    (void)kill(-job->pgid, SIGCONT);
    printf("[%d]+ %s &\r\n", job->id, job->cmd);
}

/* Builtins that can also run in a pipeline child. Returns 1 if @argv was
 * one. In a child, cat without operands copies stdin. */
static int run_child_builtin(char* const* argv, int in_child)
{
    const char* cmd = argv[0];
    if (str_eq(cmd, "echo"))
        cmd_echo(argv);
    else if (str_eq(cmd, "pwd"))
        cmd_pwd();
    else if (str_eq(cmd, "ls"))
        cmd_ls(argv[1]);
    else if (str_eq(cmd, "cat"))
    {
        if (argv[1])
            cmd_cat(argv[1]);
        else if (in_child)
            copy_fd(0, "cat");
        else
            write_str("cat: missing operand\r\n");
    }
    else
        return 0;
    return 1;
}

/* Builtins that change the shell itself. Returns 1 if @argv was one.
 * @raw is the whole line (export parses it itself). */
static int run_builtin(char* const* argv, const char* raw)
{
    const char* cmd = argv[0];
    if (str_eq(cmd, "exit"))
    {
        write_str("Bye.\r\n");
        poweroff_qemu();
    }
    else if (str_eq(cmd, "export"))
        apply_export_line(raw);
    else if (str_eq(cmd, "cd"))
        cmd_cd(argv[1]);
    else if (str_eq(cmd, "jobs"))
        cmd_jobs();
    else if (str_eq(cmd, "fg"))
        cmd_fg_bg(argv[1], 1);
    else if (str_eq(cmd, "bg"))
        cmd_fg_bg(argv[1], 0);
    else
        return run_child_builtin(argv, 0);
    return 1;
}

/* ---- Running programs --------------------------------------------------- */

/* Signals the shell ignores; a job gets them back at their defaults. */
static const int s_job_signals[] = {
    SIGINT, SIGQUIT, SIGTSTP, SIGTTIN, SIGTTOU, 0,
};

static void set_disposition(int sig, sighandler_t handler, unsigned flags)
{
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_mask = 0;
    act.sa_flags = flags;
    act.sa_restorer = 0;
    (void)sigaction(sig, &act, 0);
}

/* In the child of a pipeline stage: join the job's group, wire stdin and
 * stdout, then run the builtin or program. Never returns. */
static void exec_stage(char* const* argv, int pgid, int in_fd, int out_fd,
                       int unused_fd)
{
    (void)setpgid(0, pgid);
    for (int i = 0; s_job_signals[i]; i++)
        set_disposition(s_job_signals[i], SIG_DFL, 0);
    set_disposition(SIGCHLD, SIG_DFL, 0);
    (void)sigprocmask(SIG_SETMASK, &g_base_mask, 0);

    if (in_fd >= 0)
    {
        (void)dup2(in_fd, 0);
        (void)close(in_fd);
    }
    if (out_fd >= 0)
    {
        (void)dup2(out_fd, 1);
        (void)close(out_fd);
    }
    if (unused_fd >= 0)
        (void)close(unused_fd);

    if (run_child_builtin(argv, 1))
        exit(0);

    /* No PATH: a bare name is a program in the root directory. */
    char path[MAX_PATH];
    int n = 0;
    const char* name = argv[0];
    int has_slash = 0;
    for (int i = 0; name[i]; i++)
        if (name[i] == '/')
            has_slash = 1;
    if (!has_slash)
        path[n++] = '/';
    for (int i = 0; name[i] && n < MAX_PATH - 1; i++)
        path[n++] = name[i];
    path[n] = '\0';

    char* envp[] = { 0 };
    (void)execve(path, argv, envp);
    printf("%s: command not found\r\n", name);
    exit(EXIT_NOT_FOUND);
}

/* Job title: the line without its trailing '&'. */
static void job_title(char* out, const char* raw)
{
    int n = str_len(raw);
    while (n > 0 && (raw[n - 1] == '&' || is_space(raw[n - 1])))
        n--;
    int i = 0;
    for (; i < n && i < JOB_CMD_LEN - 1; i++)
        out[i] = raw[i];
    out[i] = '\0';
}

static void launch(struct pipeline* pl, const char* raw)
{
    sigset_t old;
    block_sigchld(&old);
    struct job* job = job_alloc();
    if (!job)
    {
        write_str("sh: too many jobs\r\n");
        restore_mask(&old);
        return;
    }
    job_title(job->cmd, raw);
    job->background = pl->background;

    int in_fd = -1;
    for (int k = 0; k < pl->nstages; k++)
    {
        int p[2] = { -1, -1 };
        if (k < pl->nstages - 1 && pipe(p) < 0)
        {
            write_str("sh: cannot create a pipe\r\n");
            break;
        }
        int pid = fork();
        if (pid == 0)
            exec_stage(pl->argv[k], job->pgid, in_fd, p[1], p[0]);
        if (in_fd >= 0)
            (void)close(in_fd);
        if (p[1] >= 0)
            (void)close(p[1]);
        in_fd = p[0];
        if (pid < 0)
        {
            write_str("sh: fork failed\r\n");
            break;
        }
        if (job->pgid == 0)
            job->pgid = pid;
        /* Also done in the child; whichever runs first wins the race. */
        (void)setpgid(pid, job->pgid);
        job->pids[job->nprocs] = pid;
        job->states[job->nprocs] = PROC_RUNNING;
        job->nprocs++;
    }
    if (in_fd >= 0)
        (void)close(in_fd);

    if (job->nprocs == 0)
        job->used = 0;
    else if (pl->background)
        printf("[%d] %d\r\n", job->id, job->pgid);
    else
        wait_foreground(job, 0);
    restore_mask(&old);
}

static void load_ditrc_defaults(void)
{
    /* Fallback .ditrc profile until userspace file-open syscall exists */
    static const char* rc_lines[] = {
        "export USER=user", "export HOST=dit",
        "export PS1=\\e[1;32m\\u@\\h\\e[0m:\\e[1;34m\\w\\e[0m\\$ ", 0
    };
    for (int i = 0; rc_lines[i]; i++)
        apply_export_line(rc_lines[i]);
}

static void shell_signals_init(void)
{
    /* Like an interactive shell: job-control signals are for the jobs. */
    for (int i = 0; s_job_signals[i]; i++)
        set_disposition(s_job_signals[i], SIG_IGN, 0);
    /* SA_RESTART: the reaper must not make the prompt's read() fail. */
    set_disposition(SIGCHLD, on_sigchld, SA_RESTART);
    (void)sigprocmask(SIG_BLOCK, 0, &g_base_mask);
    (void)sigdelset(&g_base_mask, SIGCHLD);
    (void)tcsetpgrp(0, getpgrp());
    /* Anything that exited before the handler existed. */
    on_sigchld(SIGCHLD);
}

void userland_shell_main(void)
{
    char buf[MAX_LINE];
    char prompt[MAX_PROMPT];
    static struct pipeline pl;

    load_ditrc_defaults();
    shell_signals_init();
    readline_init();

    for (;;)
    {
        sigset_t old;
        block_sigchld(&old);
        notify_jobs();
        restore_mask(&old);

        render_prompt(prompt, MAX_PROMPT);
        if (readline(prompt, buf, sizeof(buf), s_commands) < 0)
            continue;
        trim_input_line(buf);

        if (parse_line(buf, &pl) < 0)
        {
            write_str("sh: syntax error\r\n");
            continue;
        }
        if (pl.nstages == 0)
            continue;
        if (pl.nstages == 1 && !pl.background)
        {
            block_sigchld(&old);
            int handled = run_builtin(pl.argv[0], buf);
            restore_mask(&old);
            if (handled)
                continue;
        }
        launch(&pl, buf);
    }
}
