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
#include <tbd/drivers/cv_inputs.hpp>
#include <tbd/logging.hpp>

#if TBD_CALIBRATION
#include "Calibration.hpp"
#endif

#if TBD_CV_ADC
    #include "gpio.hpp"

    uint8_t CTAG::AUDIO::InputManager::trig_data[N_TRIGS];
    float CTAG::AUDIO::InputManager::cv_data[N_CVS];
#elif TBD_CV_STM32
#include <tbd/drivers/cv_inputs_stm32.hpp>
#elif TBD_CV_MIDI
#include "Midi.hpp"
#else
    #error "no CV inputs configured"
#endif

namespace tbd::audio {

IRAM_ATTR void InputManager::Update(uint8_t **trigs, float **cvs) {

#if TBD_ACD
    CTAG::DRIVERS::ADC::Update();
#elif TBD_CV_STM32
    uint8_t *data = (uint8_t *) DRIVERS::mk2::Update();
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
    uint8_t *data = CTAG::CTRL::Midi::Update();
    *cvs = (float *) data;
    *trigs = &data[N_CVS * 4];
#endif

#if TBD_CALIBRATION
        CTAG::CTAG::CAL::Calibration::MapCVData(CTAG::DRIVERS::ADC::data, cv_data);
        *cvs = cv_data;

    // update trig data
        trig_data[0] = CTAG::DRIVERS::GPIO::GetTrig0();
        trig_data[1] = CTAG::DRIVERS::GPIO::GetTrig1();
        *trigs = trig_data;
#endif
}

void InputManager::SetCVChannelBiPolar(const bool &v0, const bool &v1, const bool &v2, const bool &v3) {
#if TBD_CALIBRATION
    // ifdefs to exclude this from BBA and MK2 are in Calibration.hpp
    CTAG::CAL::Calibration::ConfigCVChannels(v0 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar,
                                             v1 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar,
                                             v2 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar,
                                             v3 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar);
#endif
}

void InputManager::Init() {
    TBD_LOGI("Control", "Initializing control!");

#if TBD_CV_ADC
    DRIVERS::ADC::InitADCSystem();
    DRIVERS::GPIO::InitGPIO();
#elif TBD_CV_STM32
    DRIVERS::mk2::Init();
#elif TBD_CV_MIDI
    CTAG::CTRL::Midi::Init();
#endif

#if TDB_CALLIBRARION
    CTAG::CAL::Calibration::Init();
#endif

}

void InputManager::FlushBuffers() {
#if TBD_CV_MIDI
    CTAG::CTRL::Midi::Flush();
#endif
}

}
