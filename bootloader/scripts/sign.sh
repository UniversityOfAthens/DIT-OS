#!/usr/bin/env bash
# =============================================================================
# Boot sector signing (optional - for secure boot or checksum)
# Ensures bytes 510-511 are 0x55 0xAA.
# =============================================================================

set -euo pipefail
IFS=$'\n\t'

if [ $# -lt 1 ]; then
    >&2 echo "Usage: $0 <stage1.bin>"
    >&2 echo "Ensures the 512-byte image has boot signature at offset 510-511."
    exit 1
fi

readonly FILE="$1"
if [ ! -f "$FILE" ]; then
    >&2 echo "Error: $FILE not found"
    exit 1
fi

SIZE=$(stat -c%s "$FILE" 2>/dev/null || stat -f%z "$FILE" 2>/dev/null)
if [ "$SIZE" -ne 512 ]; then
    >&2 echo "Warning: $FILE size is $SIZE, expected 512"
fi

# Write 0x55 0xAA at offset 510 (bytes 511 and 512)
printf '\x55\xaa' | dd of="$FILE" bs=1 seek=510 conv=notrunc status=none
echo "Boot signature written to $FILE"
