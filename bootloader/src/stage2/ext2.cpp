// =============================================================================
// EXT2 filesystem reader (minimal)
// =============================================================================

#include "../../include/fs.hpp"
#include "../../include/disk.hpp"
#include "../../include/types.hpp"

namespace boot {

// EXT2 superblock magic
constexpr u16 EXT2_SUPER_MAGIC = 0xEF53;

class Ext2Reader : public FSReader {
public:
    bool init(u8 drive) override { drive_ = drive; return true; }
    bool open(const char *path, DirEntry *out) override { (void)path; (void)out; return false; }
    bool read(void *buffer, u32 size) override { (void)buffer; (void)size; return false; }
    void close() override {}
private:
    u8 drive_ = 0;
};

} // namespace boot
