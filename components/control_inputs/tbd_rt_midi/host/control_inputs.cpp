#include <tbd/control_inputs.hpp>

#include <tbd/audio_device/audio_settings.hpp>

namespace tbd {

namespace {
    float cv_buffer[N_CVS] = {};
    uint8_t trigger_buffer[N_TRIGS] = {};
}

void ControlInputs::SetCVChannelBiPolar(bool const &v0, bool const &v1, bool const &v2, bool const &v3) {

}

void ControlInputs::init() {

}

void ControlInputs::update(uint8_t **trigs, float **cvs) {
    *cvs = cv_buffer;
    *trigs = trigger_buffer;
}

void ControlInputs::flush() {

}

}
