#ifndef BOOTLOADER_ELF_HPP
#define BOOTLOADER_ELF_HPP

#include "types.hpp"

namespace boot
{

// ELF32 types
constexpr u32 ELF_MAGIC = 0x464C457F; // "\x7FELF"

struct Elf32_Ehdr
{
    u8 e_ident[16];
    u16 e_type;
    u16 e_machine;
    u32 e_version;
    u32 e_entry;
    u32 e_phoff;
    u32 e_shoff;
    u32 e_flags;
    u16 e_ehsize;
    u16 e_phentsize;
    u16 e_phnum;
    u16 e_shentsize;
    u16 e_shnum;
    u16 e_shstrndx;
};

struct Elf32_Phdr
{
    u32 p_type;
    u32 p_offset;
    u32 p_vaddr;
    u32 p_paddr;
    u32 p_filesz;
    u32 p_memsz;
    u32 p_flags;
    u32 p_align;
};

constexpr u32 PT_LOAD = 1;

// Validate ELF and load segments to their p_vaddr
bool elf_load(void* file, u32 size, u32* entry_out);

} // namespace boot

#endif
