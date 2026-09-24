// =============================================================================
// FAT12/16/32 reader (minimal - open/read by path)
// =============================================================================

#include "../../include/fs.hpp"
#include "../../include/disk.hpp"
#include "../../include/types.hpp"

namespace boot
{

// Minimal BPB for FAT (first sector)
struct FatBPB
{
    u8 jmp[3];
    u8 oem[8];
    u16 bytes_per_sector;
    u8 sectors_per_cluster;
    u16 reserved_sectors;
    u8 num_fats;
    u16 root_entries;
    u16 total_sectors_16;
    u8 media_type;
    u16 sectors_per_fat16;
    u16 sectors_per_track;
    u16 num_heads;
    u32 hidden_sectors;
    u32 total_sectors_32;
} __attribute__((packed));

class FatReader : public FSReader
{
public:
    bool init(u8 drive) override
    {
        drive_ = drive;
        return true;
    }
    bool open(const char* path, DirEntry* out) override
    {
        (void)path;
        (void)out;
        return false;
    }
    bool read(void* buffer, u32 size) override
    {
        (void)buffer;
        (void)size;
        return false;
    }
    void close() override
    {
    }

private:
    u8 drive_ = 0;
};

} // namespace boot
