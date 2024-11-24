#include <tbd/storage/common/flash_storage.hpp>

#include <esp_flash.h>
#include <tbd/logging.hpp>
#include "sdkconfig.h"

namespace tbd {
namespace storage {

void FlashStorage::read(void* buffer, address_type address, address_type length) {
    esp_flash_read(nullptr, buffer, address, length);
}

FlashStorage default_flash;

}
}
