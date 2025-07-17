#ifdef USE_ESP32

#include <tbd/storage/flash_storage.hpp>

#include <esp_flash.h>
#include <tbd/logging.hpp>


namespace tbd {
namespace storage {

void FlashStorage::read(void* buffer, address_type address, address_type length) {
    esp_flash_read(nullptr, buffer, address, length);
}

FlashStorage default_flash;

}
}

#endif