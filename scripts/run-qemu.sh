#!/usr/bin/env bash
# Run DIT-OS in QEMU (custom bootloader disk). Builds if disk.img missing.

set -e
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DISK="$ROOT/bootloader/build/disk.img"

if [[ ! -f "$DISK" ]]; then
    echo "Disk not found. Running build..."
    "$ROOT/scripts/build.sh"
fi

echo "Starting QEMU..."
exec qemu-system-i386 -accel tcg,thread=single \
    -drive format=raw,file="$DISK",index=0,if=floppy,snapshot=on \
    -display none -serial mon:stdio "$@"
