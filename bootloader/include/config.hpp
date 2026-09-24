#ifndef BOOTLOADER_CONFIG_HPP
#define BOOTLOADER_CONFIG_HPP

#include <stdint.h>

namespace boot
{

constexpr int MAX_ENTRIES = 8;
constexpr int MAX_NAME_LEN = 32;
constexpr int MAX_PATH_LEN = 64;
constexpr int MAX_CMDLINE_LEN = 128;
constexpr int ENTRY_SIZE = MAX_NAME_LEN + MAX_PATH_LEN + MAX_CMDLINE_LEN + 2;

struct BootEntry
{
    char name[MAX_NAME_LEN];
    char kernel_path[MAX_PATH_LEN];
    char cmdline[MAX_CMDLINE_LEN];
};

struct Config
{
    uint8_t timeout;
    uint8_t default_entry;
    uint8_t entry_count;
    BootEntry entries[MAX_ENTRIES];
};

class ConfigParser
{
public:
    ConfigParser();
    bool parse(const char* buffer, uint32_t size);
    const Config& get_config() const
    {
        return config_;
    }

private:
    Config config_;

    void skip_whitespace(const char*& p);
    void skip_to_newline(const char*& p);
    uint8_t parse_number(const char*& p);
    bool parse_entry(const char*& p, BootEntry& entry);
};

} // namespace boot

#endif
