#!/usr/bin/env bash
# Run DIT-OS in QEMU (custom bootloader disk). Builds if disk.img missing.
#
# With -serial mon:stdio, the menu uses the terminal: use Up/Down arrows or 1/2/3, Enter to select.
# To exit QEMU: press Ctrl+A then X (QEMU intercepts Ctrl+A; the next key is the command).

set -euo pipefail
IFS=$'\n\t'

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly ROOT
readonly DISK="$ROOT/bootloader/build/disk.img"

if [[ ! -f "$DISK" ]]; then
    echo "Disk not found. Running build..."
    "$ROOT/scripts/build.sh"
fi

echo "Starting QEMU (menu: arrows or 1/2/3 + Enter; exit: Ctrl+A then X)..."
exec qemu-system-i386 -accel tcg,thread=single \
    -drive format=raw,file="$DISK",index=0,if=floppy,snapshot=on \
    -display none -serial mon:stdio \
    -no-reboot -no-shutdown -m 32M "$@"
