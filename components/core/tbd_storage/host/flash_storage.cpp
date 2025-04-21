#ifdef USE_HOST

#include <tbd/storage/common/flash_storage.hpp>

#include <tbd/logging.hpp>


namespace tbd::storage {

void FlashStorage::read(void* buffer, address_type address, address_type length) {
    // FIXME: implement
}

FlashStorage default_flash;

}

#endif
