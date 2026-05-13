#ifndef BOOTLOADER_FS_HPP
#define BOOTLOADER_FS_HPP

#include "types.hpp"
#include "disk.hpp"

namespace boot
{

// Generic file/dir entry (name + size + cluster/sector)
struct DirEntry {
    char name[32];
    u32 size;
    u32 cluster_or_inode;
    bool is_dir;
};

// Abstract filesystem reader
class FSReader {
public:
    virtual ~FSReader() = default;
    virtual bool init(u8 drive) = 0;
    virtual bool open(const char* path, DirEntry* out) = 0;
    virtual bool read(void* buffer, u32 size) = 0;
    virtual void close() = 0;
};

// Detect filesystem and return appropriate reader (caller owns)
FSReader* fs_detect(u8 drive);

} // namespace boot

#endif
