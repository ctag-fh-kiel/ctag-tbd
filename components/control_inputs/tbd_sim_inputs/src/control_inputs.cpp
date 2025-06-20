#include <tbd/control_inputs/module.hpp>
#include <tbd/control_inputs.hpp>

#include <tbd/logging.hpp>


namespace {

// buffers for converted uint16 CVs and triggers
float cv_data[N_CVS];
uint8_t trigger_data[N_TRIGS];

}

using tbd::control_inputs::tag;

namespace tbd {

void ControlInputs::init() {
    TBD_LOGI(tag, "initializing sim input manager");
}

void TBD_IRAM ControlInputs::update(uint8_t **trigs, float **cvs) {
    *cvs = cv_data;
    *trigs = trigger_data;
}

void ControlInputs::SetCVChannelBiPolar(const bool &v0, const bool &v1, const bool &v2, const bool &v3) {

}

void ControlInputs::flush() {

}

}
