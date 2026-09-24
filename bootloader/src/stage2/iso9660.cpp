// =============================================================================
// ISO9660 (CD) reader (minimal)
// =============================================================================

#include "../../include/fs.hpp"
#include "../../include/disk.hpp"
#include "../../include/types.hpp"

namespace boot
{

// ISO9660 volume descriptor type
constexpr u8 ISO_VD_PRIMARY = 1;
constexpr u8 ISO_VD_TERMINATOR = 255;

class Iso9660Reader : public FSReader
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
