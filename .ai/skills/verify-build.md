# Post-change verification (build + run)

After substantive changes to the kernel, bootloader, userland blobs linked into the kernel, or build scripts, **verify that everything still builds and the image boots**.

> Mirrors `.cursor/rules/verify-build.mdc` for agents that read `.ai/skills/` instead of Cursor rules.

Run this sequence from the **repository root** (requires toolchain and QEMU as for normal development):

```bash
rm -rf bootloader/build && rm -rf kernel/build

make -C kernel
make -C bootloader all
make -C bootloader run
```

- **Clean** removes stale objects so link errors and missing sources surface immediately.
- **`make -C kernel`** produces `bootloader/build/kernel.bin` (see `kernel/Makefile`).
- **`make -C bootloader all`** assembles `disk.img` and binaries.
- **`make -C bootloader run`** launches QEMU; confirm the menu loads and the kernel reaches the shell (or expected boot stage).

If a change is documentation-only or cannot affect the build, skipping this sequence is fine; when in doubt, run it.

**Note:** `make -C bootloader run` starts QEMU and may block until the VM exits; in headless automation, confirm at minimum that the kernel and bootloader `all` targets succeed.
