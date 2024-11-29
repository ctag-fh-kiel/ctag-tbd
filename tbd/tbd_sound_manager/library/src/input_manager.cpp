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
    #include <tbd/drivers/adc.hpp>
    #include <tbd/drivers/gpio.hpp>

    uint8_t tbd::audio::InputManager::trig_data[N_TRIGS];
    float tbd::audio::InputManager::cv_data[N_CVS];
#elif TBD_CV_STM32
#include <tbd/drivers/adc_stm32.hpp>
#elif TBD_CV_MIDI
#include <tbd/drivers/midi.hpp>
#else
    #error "no CV inputs configured"
#endif

namespace tbd::audio {

TBD_IRAM void InputManager::Update(uint8_t **trigs, float **cvs) {

#if TBD_ADC
    drivers::ADC::Update();
#elif TBD_CV_STM32
    uint8_t *data = (uint8_t *) drivers::ADCStm32::Update();
    *cvs = (float*) data;
    *trigs = &data[N_CVS*4];
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
#elif TBD_CV_MIDI
    uint8_t *data = drivers::Midi::Update();
    *cvs = (float *) data;
    *trigs = &data[N_CVS * 4];
#endif

#if TBD_CALIBRATION
        Calibration::MapCVData(drivers::ADC::data, cv_data);
        *cvs = cv_data;

    // update trig data
        trig_data[0] = drivers::GPIO::GetTrig0();
        trig_data[1] = drivers::GPIO::GetTrig1();
        *trigs = trig_data;
#endif
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

void InputManager::Init() {
    TBD_LOGI(tag, "Initializing control!");

#if TBD_CV_ADC
    drivers::ADC::InitADCSystem();
    drivers::GPIO::InitGPIO();
#elif TBD_CV_STM32
    drivers::ADCStm32::Init();
#elif TBD_CV_MIDI
    drivers::Midi::Init();
#endif

#if TDB_CALLIBRARION
    CTAG::CAL::Calibration::Init();
#endif

}

void InputManager::FlushBuffers() {
#if TBD_CV_MIDI
    drivers::Midi::Flush();
#endif
}

}
