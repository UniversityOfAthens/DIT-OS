#!/usr/bin/env bash
# DIT-OS Main build script: kernel (ELF) + bootloader + disk image for full boot.

set -e
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

echo "[build] Building kernel (ELF)..."
make -C kernel all

echo "[build] Building bootloader (stage1, stage2, loader, disk)..."
make -C bootloader all

echo "[build] Done. Disk image: bootloader/build/disk.img"
