// DIT-OS Kernel - sys_execve
//
// Loads a static ELF32 executable: every PT_LOAD segment is read into fresh
// anonymous pages of a brand new address space, a stack with argc/argv/envp is
// built on top of it, and the calling task continues in the new image instead
// of returning to the old one (its interrupted user frame is rewritten to
// point at the entry point).

#include "syscall/syscall.hpp"
#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include "syscall/path.hpp"
#include "sched/sched.hpp"
#include "arch/x86_64/control.hpp"
#include "arch/x86_64/gdt.hpp"
#include "arch/x86_64/idt.hpp"
#include "arch/x86_64/irqflags.hpp"
#include "arch/x86_64/paging.hpp"
#include "arch/x86_64/tss.hpp"
#include "fs/vfs.hpp"
#include "mm/kmalloc.hpp"
#include "mm/uaccess.hpp"
#include "mm/vmm.hpp"
#include "errno_defs.hpp"
#include <stdint.h>
#include <stddef.h>

namespace syscall
{

extern SyscallResult sys_execve_impl(const SyscallArgs* args);

namespace
{

/* A file has to fit in the kernel heap, which bounds what can be exec'd. */
constexpr uint32_t MAX_IMAGE_BYTES = 64 * 1024;
/* Segments are confined to the image window: the stack, brk and mmap regions
 * start above it, so a segment can never overlap them. */
constexpr uint32_t IMAGE_END = mm::USER_IMAGE_BASE + mm::USER_IMAGE_SIZE;
/* Limits on the argument and environment vectors. */
constexpr uint32_t MAX_ARGS = 32;
constexpr uint32_t MAX_ARG_BYTES = 256;

/* ELF32 header and program header, as laid out in the file. */
struct Elf32Header
{
    uint8_t e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
};

struct Elf32ProgramHeader
{
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
};

static_assert(sizeof(Elf32Header) == 52, "unexpected ELF32 header layout");
static_assert(sizeof(Elf32ProgramHeader) == 32,
              "unexpected ELF32 program header layout");

constexpr uint16_t ET_EXEC = 2;
constexpr uint16_t EM_386 = 3;
constexpr uint32_t PT_LOAD = 1;
constexpr uint16_t ELFCLASS32 = 1;
constexpr uint8_t ELFDATA2LSB = 1;

int count_strings(uintptr_t vector)
{
    if (!vector)
        return 0;
    int count = 0;
    for (;;)
    {
        uintptr_t slot =
            vector + static_cast<uintptr_t>(count) * sizeof(uint32_t);
        if (slot < vector)
            return -EFAULT;
        uint32_t pointer = 0;
        if (mm::copy_from_user(&pointer, slot, sizeof(pointer)) < 0)
            return -EFAULT;
        if (!pointer)
            return count;
        if (static_cast<uint32_t>(count) == MAX_ARGS)
            return -E2BIG;
        count++;
    }
}

/* Copy the NULL terminated vector at @vector onto the user stack of address
 * space @directory, which ends at @stack_top, and return where each string
 * landed. */
int place_strings(uint32_t directory, uintptr_t vector, int count,
                  uint32_t* stack_top, uint32_t* out)
{
    char text[MAX_ARG_BYTES];
    for (int i = 0; i < count; i++)
    {
        uint32_t pointer = 0;
        if (mm::copy_from_user(
                &pointer, vector + static_cast<uintptr_t>(i) * sizeof(uint32_t),
                sizeof(pointer)) < 0)
            return -EFAULT;
        int length = mm::strncpy_from_user(text, pointer, sizeof(text));
        if (length < 0)
            return length;
        uint32_t at = (*stack_top - static_cast<uint32_t>(length) - 1) & ~3u;
        if (at < mm::USER_STACK_BASE)
            return -E2BIG;
        /* Through @directory, not copy_to_user: the caller is still running on
         * its own address space, and the new stack is not mapped there. */
        if (mm::vmm_write_user(directory, at, text,
                               static_cast<uint32_t>(length) + 1) !=
            static_cast<uint32_t>(length) + 1)
            return -EFAULT;
        out[i] = at;
        *stack_top = at;
    }
    return 0;
}

} // namespace

SyscallResult sys_execve_impl(const SyscallArgs* args)
{
    char path[PATH_MAX];
    int length = path_from_user(path, sizeof(path),
                                static_cast<uintptr_t>(args->args[0]));
    if (length < 0)
        return length;

    sched::Task* task = sched::current_task;
    if (!task || !task->page_directory)
        return -ESRCH;

    fs::file* file = fs::vfs_open(path, 0);
    if (!file)
        return -ENOENT;

    int64_t size = fs::vfs_lseek(file, 0, 2 /* SEEK_END */);
    if (size < static_cast<int64_t>(sizeof(Elf32Header)))
    {
        fs::vfs_close(file);
        return -ENOEXEC;
    }
    if (size > MAX_IMAGE_BYTES)
    {
        fs::vfs_close(file);
        return -EFBIG;
    }
    uint8_t* image =
        static_cast<uint8_t*>(mm::kmalloc(static_cast<uint32_t>(size)));
    if (!image)
    {
        fs::vfs_close(file);
        return -ENOMEM;
    }
    fs::vfs_lseek(file, 0, 0 /* SEEK_SET */);
    uint32_t got = 0;
    while (got < static_cast<uint32_t>(size))
    {
        int n =
            fs::vfs_read(file, image + got, static_cast<uint32_t>(size) - got);
        if (n <= 0)
            break;
        got += static_cast<uint32_t>(n);
    }
    fs::vfs_close(file);
    if (got != static_cast<uint32_t>(size))
    {
        mm::kfree(image);
        return -EIO;
    }

    SyscallResult result = -ENOEXEC;
    uint32_t directory = 0;
    const Elf32Header* header = reinterpret_cast<const Elf32Header*>(image);

    if (header->e_ident[0] != 0x7F || header->e_ident[1] != 'E' ||
        header->e_ident[2] != 'L' || header->e_ident[3] != 'F' ||
        header->e_ident[4] != ELFCLASS32 || header->e_ident[5] != ELFDATA2LSB ||
        header->e_type != ET_EXEC || header->e_machine != EM_386 ||
        header->e_phentsize != sizeof(Elf32ProgramHeader) ||
        header->e_phnum == 0)
    {
        goto out;
    }
    if (header->e_entry < mm::VMM_USER_START ||
        header->e_entry >= mm::VMM_USER_END)
    {
        result = -ENOEXEC;
        goto out;
    }
    if (header->e_phoff > static_cast<uint32_t>(size) ||
        header->e_phnum > (static_cast<uint32_t>(size) - header->e_phoff) /
                              sizeof(Elf32ProgramHeader))
    {
        result = -ENOEXEC;
        goto out;
    }

    directory = mm::vmm_create_user_directory(false);
    if (!directory)
    {
        result = -ENOMEM;
        goto out;
    }

    for (uint16_t i = 0; i < header->e_phnum; i++)
    {
        const Elf32ProgramHeader* ph =
            reinterpret_cast<const Elf32ProgramHeader*>(
                image + header->e_phoff + i * header->e_phentsize);
        if (ph->p_type != PT_LOAD)
            continue;
        /* A segment with no memory (an empty .data, for instance) maps
         * nothing and can carry any address. */
        if (ph->p_memsz == 0)
            continue;
        /* Otherwise the whole segment has to land inside the image window, so
         * it cannot overlap the stack, brk or mmap regions (CVE-2017-1000253
         * class), and its file part has to be inside the file. */
        if (ph->p_vaddr < mm::USER_IMAGE_BASE || ph->p_vaddr >= IMAGE_END ||
            ph->p_memsz > IMAGE_END - ph->p_vaddr ||
            ph->p_filesz > ph->p_memsz ||
            ph->p_offset > static_cast<uint32_t>(size) ||
            ph->p_filesz > static_cast<uint32_t>(size) - ph->p_offset)
        {
            result = -ENOEXEC;
            goto out;
        }

        /* Copy the file-backed part of the segment. */
        for (uint32_t done = 0; done < ph->p_filesz;)
        {
            uint32_t vaddr = ph->p_vaddr + done;
            uint32_t page = vaddr & ~(uint32_t)(arch::PAGE_SIZE - 1);
            uint32_t offset = vaddr - page;
            uint32_t chunk = arch::PAGE_SIZE - offset;
            if (chunk > ph->p_filesz - done)
                chunk = ph->p_filesz - done;
            uint32_t frame = mm::vmm_map_user_page(directory, page);
            if (!frame)
            {
                result = -ENOMEM;
                goto out;
            }
            const uint8_t* src = image + ph->p_offset + done;
            uint8_t* dst = reinterpret_cast<uint8_t*>(frame + offset);
            for (uint32_t i = 0; i < chunk; i++)
                dst[i] = src[i];
            done += chunk;
        }

        /* p_memsz may exceed p_filesz: the tail is .bss. Map those pages too
         * (frame_acquire zeroes new frames). Mapping a page that the file
         * loop already mapped returns its existing frame, so the whole
         * memory range can be walked without a special case. */
        for (uint32_t done = 0; done < ph->p_memsz;)
        {
            uint32_t vaddr = ph->p_vaddr + done;
            uint32_t page = vaddr & ~(uint32_t)(arch::PAGE_SIZE - 1);
            uint32_t offset = vaddr - page;
            uint32_t chunk = arch::PAGE_SIZE - offset;
            if (chunk > ph->p_memsz - done)
                chunk = ph->p_memsz - done;
            if (!mm::vmm_map_user_page(directory, page))
            {
                result = -ENOMEM;
                goto out;
            }
            done += chunk;
        }
    }

    {
        /* argc, argv[], NULL, envp[], NULL at the top of the user stack. */
        constexpr uint32_t stack_floor =
            mm::USER_STACK_TOP - mm::USER_STACK_SIZE;

        int argc = count_strings(static_cast<uintptr_t>(args->args[1]));
        if (argc < 0)
        {
            result = argc;
            goto out;
        }
        int envc = count_strings(static_cast<uintptr_t>(args->args[2]));
        if (envc < 0)
        {
            result = envc;
            goto out;
        }

        uint32_t stack_top = mm::USER_STACK_TOP;
        uint32_t argv_addr[MAX_ARGS];
        uint32_t envp_addr[MAX_ARGS];
        result = place_strings(directory, static_cast<uintptr_t>(args->args[1]),
                               argc, &stack_top, argv_addr);
        if (result < 0)
            goto out;
        result = place_strings(directory, static_cast<uintptr_t>(args->args[2]),
                               envc, &stack_top, envp_addr);
        if (result < 0)
            goto out;

        uint32_t words[2 * (MAX_ARGS + 1)];
        uint32_t count = 0;
        words[count++] = static_cast<uint32_t>(argc);
        for (int i = 0; i < argc; i++)
            words[count++] = argv_addr[i];
        words[count++] = 0;
        for (int i = 0; i < envc; i++)
            words[count++] = envp_addr[i];
        words[count++] = 0;

        /* The iret frame sits just below the arguments. */
        uint32_t frame[5];
        frame[0] = header->e_entry; // eip
        frame[1] = arch::GDT_SEL_USER_CODE; // cs
        frame[2] = arch::EFLAGS_IF | arch::EFLAGS_RESERVED;
        frame[3] = 0; /* esp, filled in below */
        frame[4] = arch::GDT_SEL_USER_DATA; // ss

        uint32_t esp = (stack_top - count * sizeof(uint32_t)) & ~3u;
        frame[1] = esp;
        if (esp - sizeof(frame) < stack_floor)
        {
            result = -E2BIG;
            goto out;
        }
        if (mm::vmm_write_user(directory, esp, words,
                               count * sizeof(uint32_t)) !=
            count * sizeof(uint32_t))
        {
            result = -EFAULT;
            goto out;
        }
        if (mm::vmm_write_user(directory, esp - sizeof(frame), frame,
                               sizeof(frame)) != sizeof(frame))
        {
            result = -EFAULT;
            goto out;
        }

        /* Commit. The task keeps running the interrupted syscall, but it will
         * return to the new image instead, so the old address space goes away
         * and the interrupted user frame is rewritten in place. The frame is
         * on the kernel stack, which every address space maps, so this is safe
         * after the page directory has been swapped. */
        uint32_t* regs = const_cast<uint32_t*>(syscall::current_regs());
        if (!regs)
        {
            result = -EFAULT;
            goto out;
        }
        uint32_t old_directory = task->page_directory;
        task->page_directory = directory;
        task->user_stack_top = esp;
        arch::paging_load(directory);
        arch::tss_set_kernel_stack(task->stack_top + task->stack_sz);
        mm::vmm_destroy_user_directory(old_directory);

        /* POSIX: caught signals revert to the default (their handlers are
         * gone with the old image), ignored ones stay ignored, and the
         * blocked mask and pending set are kept. */
        uint32_t irq = arch::irq_save();
        for (int sig = 0; sig < sched::NSIG; sig++)
        {
            sched::SignalAction& action = task->actions[sig];
            if (action.handler == sched::SIG_IGN)
                continue;
            action.handler = sched::SIG_DFL;
            action.mask = 0;
            action.flags = 0;
            action.restorer = 0;
        }
        arch::irq_restore(irq);

        regs[arch::FRAME_STACK_ESP] = esp;
        regs[arch::FRAME_EIP] = header->e_entry;
        regs[arch::FRAME_CS] = arch::GDT_SEL_USER_CODE;
        regs[arch::FRAME_EFLAGS] = arch::EFLAGS_IF | arch::EFLAGS_RESERVED;
        directory = 0; /* the task owns it now */
        result = 0;
    }

out:
    if (directory)
        mm::vmm_destroy_user_directory(directory);
    mm::kfree(image);
    return result;
}

} // namespace syscall
