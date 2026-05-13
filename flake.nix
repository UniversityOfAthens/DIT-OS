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
      in
      {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            # Core build tooling
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

          shellHook = ''
            echo ""
            echo "DIT-OS development environment"
            echo "  Build:  rm -rf kernel/build bootloader/build && make -C kernel && make -C bootloader all"
            echo "  Run:    make -C bootloader run"
            echo "  Debug:  make -C bootloader debug"
            echo ""
          '';
        };
      }
    );
}
