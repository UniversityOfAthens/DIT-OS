// DIT-OS Kernel - Anonymous pipes
//
// A pipe is a ring buffer shared by two open files. Readers block while it
// is empty and see EOF once no write end is open; writers block while it is
// full and get SIGPIPE/-EPIPE once no read end is open. Both ends live only
// in task context, but sleeping needs IRQs off (see sched::sleep_on).

#include "fs/pipe.hpp"
#include "fs/file.hpp"
#include "fs/ops.hpp"
#include "mm/kmalloc.hpp"
#include "sched/sched.hpp"
#include "sched/signal.hpp"
#include "arch/x86_64/irqflags.hpp"
#include "core/kassert.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace fs
{

namespace
{

static_assert(PIPE_BUF <= PIPE_CAPACITY, "atomic writes must fit");

/* A pipe is the largest variable kernel-heap user: the ring buffer alone is
 * PIPE_CAPACITY bytes out of a 256 KiB heap. Without a cap one process can
 * allocate pipes (and fork to get more fd tables) until fork/open/pipe fail
 * for every process (M46). Eight pipes keep at most ~32 KiB tied up, leaving
 * room for kernel stacks (8 KiB each) and execve images (up to 64 KiB). */
constexpr uint32_t MAX_PIPES = 8;

/* Pipes with at least one end open. Uniprocessor and cooperative, so a
 * simple counter is enough; release runs with IRQs already off. */
static uint32_t s_pipe_count;

struct Pipe
{
    uint8_t data[PIPE_CAPACITY];
    uint32_t head; /* next byte to read */
    uint32_t count; /* bytes buffered */
    bool reader_open;
    bool writer_open;
    sched::WaitQueue readers;
    sched::WaitQueue writers;
};

Pipe* pipe_of(file* f)
{
    return static_cast<Pipe*>(f->f_private);
}

void free_if_unused(Pipe* p)
{
    if (!p->reader_open && !p->writer_open)
    {
        KASSERT(s_pipe_count > 0);
        mm::kfree(p);
        s_pipe_count--;
    }
}

int pipe_read(file* f, void* buf, uint32_t size)
{
    Pipe* p = pipe_of(f);
    if (size == 0)
        return 0;
    uint8_t* out = static_cast<uint8_t*>(buf);
    for (;;)
    {
        uint32_t flags = arch::irq_save();
        if (p->count > 0)
        {
            uint32_t n = size < p->count ? size : p->count;
            for (uint32_t i = 0; i < n; i++)
                out[i] = p->data[(p->head + i) % PIPE_CAPACITY];
            p->head = (p->head + n) % PIPE_CAPACITY;
            p->count -= n;
            sched::wake_up_all(&p->writers);
            arch::irq_restore(flags);
            return static_cast<int>(n);
        }
        if (!p->writer_open)
        {
            arch::irq_restore(flags);
            return 0; /* EOF */
        }
        bool slept = sched::sleep_on(&p->readers);
        arch::irq_restore(flags);
        if (!slept)
            return -EINTR;
    }
}

int pipe_write(file* f, const void* buf, uint32_t size)
{
    Pipe* p = pipe_of(f);
    const uint8_t* in = static_cast<const uint8_t*>(buf);
    uint32_t done = 0;
    while (done < size)
    {
        uint32_t flags = arch::irq_save();
        if (!p->reader_open)
        {
            arch::irq_restore(flags);
            /* Nobody can ever read this: SIGPIPE (default: terminate). */
            sched::signal_send_task(sched::current_task, sched::SIGPIPE);
            return done ? static_cast<int>(done) : -EPIPE;
        }
        uint32_t want = size - done;
        uint32_t space = PIPE_CAPACITY - p->count;
        /* A write of at most PIPE_BUF bytes goes in at once or not at all,
         * so concurrent writers never interleave inside it. */
        bool fits = want <= PIPE_BUF ? space >= want : space > 0;
        if (fits)
        {
            uint32_t n = want < space ? want : space;
            uint32_t tail = (p->head + p->count) % PIPE_CAPACITY;
            for (uint32_t i = 0; i < n; i++)
                p->data[(tail + i) % PIPE_CAPACITY] = in[done + i];
            p->count += n;
            done += n;
            sched::wake_up_all(&p->readers);
            arch::irq_restore(flags);
            continue;
        }
        bool slept = sched::sleep_on(&p->writers);
        arch::irq_restore(flags);
        if (!slept)
            return done ? static_cast<int>(done) : -EINTR;
    }
    return static_cast<int>(done);
}

/* The wrong direction on an end is a bad descriptor for that operation. */
int pipe_no_read(file* f, void* buf, uint32_t size)
{
    (void)f;
    (void)buf;
    (void)size;
    return -EBADF;
}

int pipe_no_write(file* f, const void* buf, uint32_t size)
{
    (void)f;
    (void)buf;
    (void)size;
    return -EBADF;
}

int release_read_end(inode* ino, file* f)
{
    (void)ino;
    Pipe* p = pipe_of(f);
    uint32_t flags = arch::irq_save();
    p->reader_open = false;
    /* Blocked writers must notice and get EPIPE. */
    sched::wake_up_all(&p->writers);
    free_if_unused(p);
    arch::irq_restore(flags);
    return 0;
}

int release_write_end(inode* ino, file* f)
{
    (void)ino;
    Pipe* p = pipe_of(f);
    uint32_t flags = arch::irq_save();
    p->writer_open = false;
    /* Blocked readers must notice and see EOF. */
    sched::wake_up_all(&p->readers);
    free_if_unused(p);
    arch::irq_restore(flags);
    return 0;
}

FileOps s_read_ops = {
    nullptr, pipe_read, pipe_no_write, release_read_end, nullptr,
};

FileOps s_write_ops = {
    nullptr, pipe_no_read, pipe_write, release_write_end, nullptr,
};

file* new_end(Pipe* p, FileOps* ops)
{
    file* f = static_cast<file*>(mm::kmalloc(sizeof(file)));
    if (!f)
        return nullptr;
    uint8_t* raw = reinterpret_cast<uint8_t*>(f);
    for (uint32_t i = 0; i < sizeof(file); i++)
        raw[i] = 0;
    f->f_refcount = 1;
    f->f_private = p;
    f->f_ops = ops;
    return f;
}

} // namespace

int pipe_create(file** read_end, file** write_end)
{
    if (!read_end || !write_end)
        return -EINVAL;
    if (s_pipe_count >= MAX_PIPES)
        return -ENOMEM;
    Pipe* p = static_cast<Pipe*>(mm::kmalloc(sizeof(Pipe)));
    if (!p)
        return -ENOMEM;
    uint8_t* raw = reinterpret_cast<uint8_t*>(p);
    for (uint32_t i = 0; i < sizeof(Pipe); i++)
        raw[i] = 0;
    p->reader_open = true;
    p->writer_open = true;

    file* r = new_end(p, &s_read_ops);
    file* w = new_end(p, &s_write_ops);
    if (!r || !w)
    {
        mm::kfree(r);
        mm::kfree(w);
        mm::kfree(p);
        return -ENOMEM;
    }
    *read_end = r;
    *write_end = w;
    s_pipe_count++;
    return 0;
}

bool pipe_is_pipe(const file* f)
{
    return f && (f->f_ops == &s_read_ops || f->f_ops == &s_write_ops);
}

} // namespace fs
