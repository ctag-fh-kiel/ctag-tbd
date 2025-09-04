#include <tbd/common/audio.hpp>
#include <tbd/drivers/cv_input.hpp>

namespace {
uint8_t cv_data[TBD_CHUNK_BUFFER_SIZE + N_TRIGS] = {};
}

namespace tbd::drivers {

void CVInput::init() {

}

uint8_t* CVInput::update() {
    return cv_data;
}

void ControlInputs::update_metrics(const sound_processor::ProcessingMetrics& metrics) {

}

void CVInput::flush() {

}

}