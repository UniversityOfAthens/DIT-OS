#include "sched/sched.hpp"
#include "mm/vmm.hpp"
#include <stddef.h>
#include <stdint.h>

extern "C" const uint8_t user_image_start[];
extern "C" const uint8_t user_image_end[];

namespace sched
{

bool init_user_image()
{
    size_t image_size = static_cast<size_t>(user_image_end - user_image_start);
    if (image_size == 0 || image_size > mm::USER_IMAGE_SIZE)
        return false;

    uint8_t* destination = reinterpret_cast<uint8_t*>(mm::USER_IMAGE_BASE);
    for (size_t i = 0; i < image_size; i++)
        destination[i] = user_image_start[i];
    return true;
}

}
