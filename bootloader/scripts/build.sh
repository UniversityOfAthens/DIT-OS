#!/usr/bin/env bash
# =============================================================================
# DIT-OS Bootloader build script
# Builds stage1 + stage2 and optionally the disk image.
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BOOTLOADER_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$BOOTLOADER_ROOT/build"

usage() {
    echo "Usage: $0 [all|stage1|stage2|disk|clean]"
    echo "  all    - build stage1, stage2, and disk image (default)"
    echo "  stage1 - build stage1.bin only"
    echo "  stage2 - build stage2.bin only"
    echo "  disk   - build disk image (requires stage1, stage2, kernel)"
    echo "  clean  - remove build directory"
}

main() {
    cd "$BOOTLOADER_ROOT"
    mkdir -p "$BUILD_DIR"

    case "${1:-all}" in
        all)
            make -f Makefile all
            ;;
        stage1)
            make -f Makefile "$BUILD_DIR/stage1.bin"
            ;;
        stage2)
            make -f Makefile "$BUILD_DIR/stage2.bin"
            ;;
        disk)
            make -f Makefile disk
            ;;
        clean)
            make -f Makefile clean
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown target: $1"
            usage
            exit 1
            ;;
    esac
}

main "$@"
