#ifndef KERNEL_DEV_CHARDEV_HPP
#define KERNEL_DEV_CHARDEV_HPP

#include <stdint.h>

namespace dev
{

/** Character device interface: byte stream, optional poll */
struct CharDev
{
    /** Read up to size bytes into buf. Return bytes read, 0 on EOF, negative on
     * error. */
    int (*read)(CharDev* cdev, void* buf, uint32_t size);
    /** Write up to size bytes from buf. Return bytes written, negative on
     * error. */
    int (*write)(CharDev* cdev, const void* buf, uint32_t size);
    /** Non-zero if input is available (for poll/select). */
    int (*has_input)(CharDev* cdev);
    void* driver_data;
};

} // namespace dev

#endif
