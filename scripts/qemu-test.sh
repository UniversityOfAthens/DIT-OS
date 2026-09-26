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

    STEP="userland self-test (brk, mmap, fork, execve, waitpid)"
    # The init program exercises the process model before starting the shell,
    # so it only runs after the boot menu is dismissed.
    run_expect '\r' '^\[selftest\] ok$'
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

    STEP="tab completes command and file"
    run_expect 'ca\thel\t\r' "^$HELLO_TEXT\$"
    pass

    STEP="tab lists ambiguous commands"
    run_expect 'e\t' 'echo  exit  export'
    send '\003'
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
