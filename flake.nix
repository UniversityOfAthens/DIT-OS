{
  description = "DIT-OS development environment";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };

        commonPackages = with pkgs; [
          # Core build tooling (kernel/Makefile uses host GCC with -m32
          # unless an i686-elf-* toolchain is on PATH)
          gnumake
          gcc13
          binutils
          nasm

          # Emulator and debugging
          qemu
          gdb

          # Disk image verification (CI)
          xxd

          # Code formatting (CI lint)
          clang-tools

          # ISO / initrd creation
          xorriso
          mtools
        ];

        mkDevShell =
          extraPackages:
          pkgs.mkShell {
            packages = commonPackages ++ extraPackages;

            # Nix's cc-wrapper adds userspace hardening flags (fortify,
            # stack protector, zero-call-used-regs, ...) that a freestanding
            # kernel must not get.
            hardeningDisable = [ "all" ];

            shellHook = ''
              echo ""
              echo "DIT-OS development environment"
              echo "  Build:  rm -rf kernel/build bootloader/build && make -C kernel && make -C bootloader all"
              echo "  Run:    make -C bootloader run"
              echo "  Debug:  make -C bootloader debug"
              echo ""
            '';
          };
      in
      {
        # nix develop: host GCC (fast, fully cached)
        devShells.default = mkDevShell [ ];

        # nix develop .#cross: adds the i686-elf cross toolchain, which
        # kernel/Makefile then uses automatically. Not in the binary cache,
        # so the first entry builds GCC from source.
        devShells.cross = mkDevShell [
          pkgs.pkgsCross.i686-embedded.buildPackages.gcc
          pkgs.pkgsCross.i686-embedded.buildPackages.binutils
        ];
      }
    );
}
