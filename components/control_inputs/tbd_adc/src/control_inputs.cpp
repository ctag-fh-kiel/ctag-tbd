#include <tbd/control_inputs/module.hpp>
#include <tbd/control_inputs.hpp>

#include <tbd/logging.hpp>

#include <tbd/control_inputs/impl/adc.hpp>
#include <tbd/control_inputs/gpio.hpp>
#if TBD_CALIBRATION
    #include <tbd/control_inputs/impl/calibration.hpp>
#endif 

using CVInput = tbd::drivers::ADC;


namespace {

// buffers for converted uint16 CVs and triggers
float cv_data[N_CVS];
uint8_t trigger_data[N_TRIGS];

}

using tbd::control_inputs::tag;

namespace tbd {

void ControlInputs::init() {
    TBD_LOGI(tag, "initializing input manager");

    CVInput::init();
    drivers::GPIO::InitGPIO();


    #if TDB_CALLIBRARION
        CTAG::CAL::Calibration::Init();
    #endif

}

void TBD_IRAM ControlInputs::update(uint8_t **trigs, float **cvs) {

    /* for debug purposes
    uint16_t *magic_number = (uint16_t*) &data[98];
    if(*magic_number != 0xcafe){
        // Debug transmission
        printf("%5d ", *magic_number);
        for(int i=0;i<8;i++){
            printf("%d ", (*trigs)[i]);
        }
        for(int i=0;i<18;i++){
            printf("%.3f ", (*cvs)[i]);
        }
        printf("\n");
    }
     */

    auto data = CVInput::update();


// FIXME: are we ever going to have a generalised calibration flow and can we integrate
//        triggers into ADC to make this more consistent
    #if TBD_CALIBRATION
        // output only contains CVs as uint16

        // convert uint16 raw CVs to calibrated floats
        Calibration::MapCVData(reinterpret_cast<uint16_t*>(data), cv_data);
        auto _cvs = cv_data;

        // fetch trigger values from pins
        trigger_data[0] = drivers::GPIO::GetTrig0();
        trigger_data[1] = drivers::GPIO::GetTrig1();
        auto _trigs = trigger_data;
    #else
        // raw output contains both CVs and triggers in the correct format

        auto _cvs = reinterpret_cast<float*>(data);
        auto _trigs = &data[N_CVS * 4];
    #endif
    *cvs = _cvs;
    *trigs = _trigs;
}

void ControlInputs::SetCVChannelBiPolar(const bool &v0, const bool &v1, const bool &v2, const bool &v3) {
    #if TBD_CALIBRATION
        // ifdefs to exclude this from BBA and MK2 are in Calibration.hpp
        Calibration::ConfigCVChannels(v0 ? calibration::CVConfig::CVBipolar : calibration::CVConfig::CVUnipolar,
                                    v1 ? calibration::CVConfig::CVBipolar : calibration::CVConfig::CVUnipolar,
                                    v2 ? calibration::CVConfig::CVBipolar : calibration::CVConfig::CVUnipolar,
                                    v3 ? calibration::CVConfig::CVBipolar : calibration::CVConfig::CVUnipolar);
    #endif
}

void ControlInputs::flush() {
    CVInput::flush();
}

}
