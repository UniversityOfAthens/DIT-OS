/* DIT-OS Userland - Shell line editor with tab completion */
#ifndef USER_SHELL_READLINE_H
#define USER_SHELL_READLINE_H

/** Switch the console to raw mode so the editor sees every key. If that
 * fails, readline falls back to the kernel's line editing (no completion). */
void readline_init(void);

/** Print @prompt and read one line into @buf (NUL-terminated, no newline).
 * Tab completes the first word from @commands (NULL-terminated) and later
 * words as file paths. Returns the line length, or -1 on read error. */
int readline(const char* prompt, char* buf, int cap,
             const char* const* commands);

#endif /* USER_SHELL_READLINE_H */
