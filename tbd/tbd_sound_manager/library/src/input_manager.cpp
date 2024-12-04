/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.
(c) 2023 MIDI-Message-Parser aka 'bba_update()' by Mathias Brüssel. 

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/
#include "input_manager.hpp"

#include <tbd/logging.hpp>
#include <tbd/sound_manager/common/module.hpp>

#if TBD_CALIBRATION
    #include <tbd/calibration.hpp>
#endif

#if TBD_CV_ADC || TBD_CV_MCP_3208
    #if !TBD_CALIBRATION
        #error "calibration is mandatory for ADC and MCP3208"
    #endif

    #include <tbd/drivers/adc.hpp>
    #include <tbd/drivers/gpio.hpp>

    using CVInput = tbd::drivers::ADC;
#elif TBD_CV_STM32
    #include <tbd/drivers/adc_stm32.hpp>

    using CVInput = tbd::drivers::ADCStm32;
#elif TBD_CV_MIDI
    #include <tbd/drivers/midi.hpp>

    using CVInput = tbd::drivers::Midi;
#elif TBD_CV_SIM
    #include <tbd/drivers/cv_input.hpp>

    using CVInput = tbd::drivers::CVInput;
#else
    #error "no CV inputs configured"
#endif


namespace {

#if TBD_CV_ADC || TBD_CV_MCP_3208

    // buffers for converted uint16 CVs and triggers
    float cv_data[N_CVS];
    uint8_t trigger_data[N_TRIGS];

#endif

}


namespace tbd::audio {

void TBD_IRAM InputManager::update(uint8_t **trigs, float **cvs) {

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

void InputManager::SetCVChannelBiPolar(const bool &v0, const bool &v1, const bool &v2, const bool &v3) {
#if TBD_CALIBRATION
    // ifdefs to exclude this from BBA and MK2 are in Calibration.hpp
    Calibration::ConfigCVChannels(v0 ? calibration::CVConfig::CVBipolar : calibration::CVConfig::CVUnipolar,
                                  v1 ? calibration::CVConfig::CVBipolar : calibration::CVConfig::CVUnipolar,
                                  v2 ? calibration::CVConfig::CVBipolar : calibration::CVConfig::CVUnipolar,
                                  v3 ? calibration::CVConfig::CVBipolar : calibration::CVConfig::CVUnipolar);
#endif
}

void InputManager::init() {
    TBD_LOGI(tag, "Initializing control!");

    CVInput::init();
#if TBD_CV_ADC
    drivers::GPIO::InitGPIO();
#endif

#if TDB_CALLIBRARION
    CTAG::CAL::Calibration::Init();
#endif

}

void InputManager::flush() {
    CVInput::flush();
}

}
