#!/usr/bin/env bash
# Create disk image: stage1 + stage2 + loader + kernel (stripped ELF).
# Called by bootloader Makefile or after build.sh.

set -e
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BLD="$ROOT/bootloader/build"

for f in "$BLD/stage1.bin" "$BLD/stage2.bin" "$BLD/loader.bin" "$BLD/kernel.img"; do
    if [[ ! -f "$f" ]]; then
        echo "create-disk: missing $f (run scripts/build.sh first)" >&2
        exit 1
    fi
done

# Layout: 0=stage1, 2-10=stage2, 11-36=loader, 37+=kernel (matches bootloader/Makefile)
dd if=/dev/zero of="$BLD/disk.img" bs=512 count=2880 2>/dev/null
dd if="$BLD/stage1.bin" of="$BLD/disk.img" conv=notrunc 2>/dev/null
dd if="$BLD/stage2.bin" of="$BLD/disk.img" bs=512 seek=2 conv=notrunc 2>/dev/null
dd if="$BLD/loader.bin" of="$BLD/disk.img" bs=512 seek=11 conv=notrunc 2>/dev/null
dd if="$BLD/kernel.img" of="$BLD/disk.img" bs=512 seek=37 conv=notrunc 2>/dev/null

echo "Disk image: $BLD/disk.img"
