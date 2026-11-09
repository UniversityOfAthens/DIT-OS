#!/usr/bin/env bash
# DIT-OS integration test: boot the disk image in QEMU and drive the shell
# over the serial port, waiting for expected output instead of sleeping.
#
# Usage: scripts/qemu-test.sh [disk.img]
# Env:   QEMU_TEST_LOG     where to keep the serial log (default: temp file)
#        QEMU_TEST_TIMEOUT seconds to wait for each expected output (30)
#        QEMU_TEST_STRESS  number of repeated commands in the stress step (100)
# Exit status is 0 only if every check passed; on failure the end of the
# serial log is printed.

set -euo pipefail
IFS=$'\n\t'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
readonly ROOT
readonly DISK="${1:-$ROOT/bootloader/build/disk.img}"
readonly TIMEOUT="${QEMU_TEST_TIMEOUT:-30}"
readonly STRESS="${QEMU_TEST_STRESS:-100}"
readonly HELLO_TEXT="Hello from DIT-OS ramfs."
readonly HELLO_BYTES=$(( ${#HELLO_TEXT} + 1 ))
readonly POLL_INTERVAL=0.05

WORK_DIR="$(mktemp -d)"
readonly WORK_DIR
readonly LOG="${QEMU_TEST_LOG:-$WORK_DIR/serial.log}"
readonly FIFO="$WORK_DIR/stdin"
QEMU_PID=""
STEP="startup"

cleanup() {
    exec 3>&- 2>/dev/null || true
    if [[ -n "$QEMU_PID" ]] && kill -0 "$QEMU_PID" 2>/dev/null; then
        kill "$QEMU_PID" 2>/dev/null || true
        wait "$QEMU_PID" 2>/dev/null || true
    fi
    rm -rf "$WORK_DIR"
}
trap cleanup EXIT

fail() {
    >&2 echo "FAIL [$STEP]: $*"
    >&2 echo "----- last 40 lines of serial output -----"
    tail -n 40 "$LOG" | cat -v >&2 || true
    exit 1
}

pass() {
    echo "PASS [$STEP]"
}

log_size() {
    wc -c < "$LOG"
}

# Send keystrokes (printf %b escapes: \r = Enter, \t = Tab, \003 = Ctrl+C).
send() {
    printf '%b' "$1" >&3
}

# Wait until the log, after byte offset $1, has a line matching regex $2.
# Anchor patterns with ^ so the echoed input line cannot satisfy them.
expect_after() {
    local offset="$1" pattern="$2" deadline=$((SECONDS + TIMEOUT))
    while ((SECONDS < deadline)); do
        if tail -c +"$((offset + 1))" "$LOG" | tr -d '\r' |
            grep -aqE -- "$pattern"; then
            return 0
        fi
        kill -0 "$QEMU_PID" 2>/dev/null || fail "QEMU exited early"
        sleep "$POLL_INTERVAL"
    done
    fail "no output matching '$pattern' within ${TIMEOUT}s"
}

# Send $1 and wait for a line matching $2 in the output it produces.
run_expect() {
    local offset
    offset="$(log_size)"
    send "$1"
    expect_after "$offset" "$2"
}

start_qemu() {
    [[ -f "$DISK" ]] || fail "disk image not found: $DISK"
    mkfifo "$FIFO"
    : > "$LOG"
    qemu-system-i386 -accel tcg \
        -drive format=raw,file="$DISK",index=0,if=floppy,snapshot=on \
        -display none -serial stdio -monitor none \
        -no-reboot -m 32M < "$FIFO" > "$LOG" 2>&1 &
    QEMU_PID=$!
    exec 3> "$FIFO"
}

main() {
    start_qemu

    STEP="boot menu"
    expect_after 0 "Boot DIT-OS"
    pass

    STEP="userland self-test: stat/fstat"
    # The init program exercises the process model before starting the shell,
    # so it only runs after the boot menu is dismissed. The stat line reports
    # the size of the seeded hello.txt, which stat() reads back from ramfs.
    run_expect '\r' "^\\[selftest\\] stat: /hello\\.txt ${HELLO_BYTES} bytes\$"
    pass

    STEP="userland self-test: kill"
    # A child kills itself with SIGKILL; waitpid() must decode the signal
    # status (9) through the POSIX encoding.
    expect_after 0 '^\[selftest\] kill: SIGKILL status 9$'
    pass

    STEP="userland self-test (brk, mmap, fork, execve, waitpid, stat, kill, security)"
    expect_after 0 '^\[selftest\] ok$'
    # The ok line alone is not trustworthy: the failures counter lives on a
    # page fork() makes copy-on-write, and a COW refcount bug once corrupted
    # it back to 0 while FAIL lines were already printed (M42, fixed). Keep
    # scanning the log so a failing self-test can never pass this gate.
    if tr -d '\r' < "$LOG" | grep -aq '^\[selftest\] FAIL'; then
        fail "self-test printed FAIL lines"
    fi
    pass

    STEP="kernel reaches shell"
    expect_after 0 'user@dit'
    pass

    STEP="echo"
    run_expect 'echo "ci-echo-ok"\r' '^ci-echo-ok$'
    pass

    STEP="cat"
    run_expect 'cat hello.txt\r' "^$HELLO_TEXT\$"
    pass

    STEP="ls"
    run_expect 'ls /\r' '^README\.txt$'
    pass

    STEP="missing file"
    run_expect 'cat no-such-file\r' '^cat: cannot open file$'
    pass

    STEP="read and getdents errors are reported"
    # getdents() on a regular file (-ENOTDIR) and read() on a directory
    # (-EISDIR) return errors; treating them as EOF would silently print
    # nothing. Both messages go to fd 2.
    run_expect 'ls hello.txt\r' '^ls: hello\.txt: cannot read directory$'
    run_expect 'cat /\r' '^cat: read error$'
    pass

    STEP="tab completes command and file"
    run_expect 'ca\thel\t\r' "^$HELLO_TEXT\$"
    pass

    STEP="tab lists ambiguous commands"
    run_expect 'e\t' 'echo  exit  export'
    send '\003'
    pass

    STEP="cd and pwd (kernel working directory)"
    run_expect 'cd home\r' 'user@dit'
    run_expect 'pwd\r' '^/home$'
    run_expect 'cat notes.txt\r' '^Notes live in /home\.$'
    run_expect 'cd ..\r' 'user@dit'
    run_expect 'pwd\r' '^/$'
    run_expect 'cd nowhere\r' '^cd: nowhere: No such file or directory$'
    pass

    STEP="shell parser limits"
    # Argument and pipeline counts are bounded (Baron Samedit class): an
    # over-long command line is refused, never split past its buffers.
    run_expect 'echo 1 2 3 4 5 6 7 8 9\r' '^sh: syntax error$'
    run_expect 'echo a | echo b | echo c | echo d | echo e\r' '^sh: syntax error$'
    run_expect 'echo "unterminated\r' '^sh: syntax error$'
    run_expect 'echo one & echo two\r' '^sh: syntax error$'
    pass

    STEP="run a program"
    run_expect 'greeter\r' '^\[selftest\] execve: greeter \(argc=1\)$'
    run_expect 'nosuch\r' '^nosuch: command not found$'
    pass

    STEP="pipeline (pipe + dup2)"
    run_expect 'cat hello.txt | greeter read\r' "^greeter got: $HELLO_TEXT\$"
    pass

    # "greeter read" prints its ready line just before it blocks in read(),
    # so input sent after that line reaches the job, not the shell.
    STEP="foreground job reads the console"
    run_expect 'greeter read\r' '^greeter: reading$'
    run_expect 'typed-line\r' '^greeter got: typed-line$'
    pass

    # A caught signal interrupts a canonical console read: the job's parent
    # signals the reader as soon as it blocks (before any input is typed),
    # read() returns -EINTR, and the retry completes a fresh line normally.
    # The TTY line buffer itself (L25) is not reachable from a serial-driven
    # signal: INTR/QUIT/SUSP flush it per POSIX (covered by review).
    STEP="signal interrupts a console read"
    run_expect 'greeter ttyline\r' '^greeter: ttyline-1$'
    local offset
    offset="$(log_size)"
    send 'ab'
    send 'cd\r'
    expect_after "$offset" 'greeter got: abcd'
    expect_after "$offset" 'greeter: ttyline-ok'
    pass

    STEP="Ctrl+C ends the foreground job"
    run_expect 'greeter read\r' '^greeter: reading$'
    run_expect '\003' '^\^C$'
    # If the job survived, it would read this line instead of the shell.
    run_expect 'echo after-intr\r' '^after-intr$'
    pass

    STEP="Ctrl+Z stops, jobs lists, fg resumes"
    run_expect 'greeter read\r' '^greeter: reading$'
    run_expect '\032' '^\[1\]\+ +Stopped +greeter read$'
    run_expect 'jobs\r' '^\[1\]\+ +Stopped +greeter read$'
    run_expect 'fg\r' '^greeter read$'
    run_expect 'resumed\r' '^greeter got: resumed$'
    pass

    STEP="background job reports its exit"
    run_expect 'greeter &\r' '^\[1\] [0-9]+$'
    run_expect '\r' '^\[1\]\+ +Exit 3 +greeter$'
    pass

    STEP="background read stops with SIGTTIN, fg brings it back"
    # A background job's output can land after the shell's prompt on the same
    # line, so this one match is not anchored at the start.
    run_expect 'greeter read &\r' 'greeter: reading$'
    run_expect '\r' '^\[1\]\+ +Stopped +greeter read$'
    run_expect 'fg %1\r' '^greeter read$'
    run_expect 'from-bg\r' '^greeter got: from-bg$'
    pass

    STEP="stress ($STRESS commands)"
    local offset count
    offset="$(log_size)"
    for ((i = 0; i < STRESS; i++)); do
        run_expect 'cat hello.txt\r' "^$HELLO_TEXT\$"
    done
    count="$(tail -c +"$((offset + 1))" "$LOG" | tr -d '\r' |
        grep -acx -- "$HELLO_TEXT" || true)"
    ((count == STRESS)) || fail "expected $STRESS outputs, got $count"
    ! grep -aq "KERNEL PANIC" "$LOG" || fail "kernel panic during stress"
    pass

    STEP="exit powers off"
    send 'exit\r'
    local deadline=$((SECONDS + TIMEOUT))
    while kill -0 "$QEMU_PID" 2>/dev/null; do
        ((SECONDS < deadline)) || fail "QEMU still running after exit"
        sleep "$POLL_INTERVAL"
    done
    wait "$QEMU_PID" || fail "QEMU exited with an error"
    QEMU_PID=""
    pass

    echo "All QEMU integration tests passed."
}

main "$@"
