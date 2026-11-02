#!/usr/bin/env bash
# DIT-OS static analysis gate: run cppcheck over the kernel (C++17) and the
# userland (C11) with the same configuration GCC builds them in.
#
# Usage: scripts/cppcheck.sh [--style]
#   --style  also report style findings (advisory: never fails)
# Exit status is 0 only if no error/warning/portability/performance finding
# remains after suppressions (config/cppcheck-suppressions.txt and inline
# `// cppcheck-suppress <id>` comments).

set -euo pipefail
IFS=$'\n\t'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
readonly ROOT
readonly SUPPRESSIONS="$ROOT/config/cppcheck-suppressions.txt"
readonly GATE_CHECKS="warning,portability,performance"

die() {
    >&2 echo "cppcheck.sh: $*"
    exit 1
}

# Run cppcheck on one tree. $1 = language standard, $2 = checks, rest = args.
run_cppcheck() {
    local std="$1" checks="$2"
    shift 2
    cppcheck \
        --std="$std" \
        --platform=unix32 \
        -D__GNUC__ -D__i386__ \
        --enable="$checks" \
        --inline-suppr \
        --suppressions-list="$SUPPRESSIONS" \
        --suppress=missingIncludeSystem \
        --error-exitcode=1 \
        --quiet \
        -j "$JOBS" \
        --template='{file}:{line}: {severity} [{id}] {message}' \
        "$@"
}

main() {
    local style=0
    case "${1:-}" in
        "") ;;
        --style) style=1 ;;
        *) die "unknown argument '$1' (usage: $0 [--style])" ;;
    esac

    command -v cppcheck >/dev/null || die "cppcheck not found on PATH"
    [[ -f "$SUPPRESSIONS" ]] || die "missing $SUPPRESSIONS"
    JOBS="$(nproc 2>/dev/null || echo 2)"
    readonly JOBS

    cd "$ROOT"
    cppcheck --version

    local checks="$GATE_CHECKS"
    ((style)) && checks="$checks,style"

    local status=0
    echo "== kernel (C++17)"
    run_cppcheck c++17 "$checks" --language=c++ \
        -I kernel/include kernel || status=1
    echo "== userland (C11)"
    run_cppcheck c11 "$checks" --language=c \
        -I userland/include -I userland/libc/include -I userland/shell \
        userland || status=1

    if ((style)); then
        echo "cppcheck.sh: style run is advisory; exiting 0"
        return 0
    fi
    if ((status)); then
        >&2 echo "cppcheck.sh: findings above must be fixed or justified"
        return 1
    fi
    echo "cppcheck.sh: clean"
}

main "$@"
