#include <tbd/common/audio.hpp>

#include <tbd/drivers/common/dynamic_cv.hpp>


namespace {
    uint8_t cv_data[TBD_CHUNK_BUFFER_SIZE + N_TRIGS] = {};
}

namespace tbd::drivers {

void DynamicCV::init() {

}

uint8_t* DynamicCV::update() {
    return cv_data;
}

void DynamicCV::flush() {

}

}