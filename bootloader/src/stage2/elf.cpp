// =============================================================================
// ELF loader - load PT_LOAD segments to their vaddr
// =============================================================================

#include "../../include/elf.hpp"
#include "../../include/types.hpp"
#include "../../include/disk.hpp"

namespace boot
{

bool elf_load(void* file, u32 size, u32* entry_out)
{
    if (size < sizeof(Elf32_Ehdr))
        return false;

    Elf32_Ehdr* eh = (Elf32_Ehdr*)file;
    if (*(u32*)eh->e_ident != ELF_MAGIC)
        return false;
    if (eh->e_ident[4] != 1) // ELFCLASS32
        return false;
    if (eh->e_ident[5] != 1) // ELFDATA2LSB
        return false;
    if (eh->e_type != 2) // ET_EXEC
        return false;
    if (eh->e_machine != 3) // EM_386
        return false;

    Elf32_Phdr* ph = (Elf32_Phdr*)((u8*)file + eh->e_phoff);
    for (u16 i = 0; i < eh->e_phnum; i++)
    {
        if (ph[i].p_type != PT_LOAD)
            continue;
        u32 filesz = ph[i].p_filesz;
        u32 memsz = ph[i].p_memsz;
        u32 vaddr = ph[i].p_vaddr;
        u32 offset = ph[i].p_offset;
        if (offset + filesz > size)
            return false;
        u8* src = (u8*)file + offset;
        u8* dst = (u8*)vaddr;
        for (u32 j = 0; j < filesz; j++)
            dst[j] = src[j];
        for (u32 j = filesz; j < memsz; j++)
            dst[j] = 0;
    }

    if (entry_out)
        *entry_out = eh->e_entry;
    return true;
}

} // namespace boot
