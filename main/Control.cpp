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

#include "Control.hpp"
#include "Calibration.hpp"
#include "gpio.hpp"
#include "adc.hpp"


#if CONFIG_TBD_PLATFORM_MK2
#include "mk2.hpp"
#elif CONFIG_TBD_PLATFORM_BBA
#include "Midi.hpp"
#else
    uint8_t CTAG::CTRL::Control::trig_data[N_TRIGS];
    float CTAG::CTRL::Control::cv_data[N_CVS];
#endif

IRAM_ATTR void CTAG::CTRL::Control::Update(uint8_t **trigs, float **cvs) {
#if defined(CONFIG_TBD_PLATFORM_MK2)
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
#elif defined(CONFIG_TBD_PLATFORM_BBA)
    uint8_t *data = Midi::Update();
    *cvs = (float *) data;
    *trigs = &data[N_CVS * 4];
#else
    // update CVs
        CTAG::DRIVERS::ADC::Update();
        CTAG::CAL::Calibration::MapCVData(CTAG::DRIVERS::ADC::data, cv_data);
        *cvs = cv_data;

    // update trig data
        trig_data[0] = CTAG::DRIVERS::GPIO::GetTrig0();
        trig_data[1] = CTAG::DRIVERS::GPIO::GetTrig1();
        *trigs = trig_data;
#endif
}

void CTAG::CTRL::Control::SetCVChannelBiPolar(const bool &v0, const bool &v1, const bool &v2, const bool &v3) {
    // ifdefs to exclude this from BBA and MK2 are in Calibration.hpp
    CTAG::CAL::Calibration::ConfigCVChannels(v0 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar,
                                             v1 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar,
                                             v2 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar,
                                             v3 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar);
}

void CTAG::CTRL::Control::Init() {
    ESP_LOGI("Control", "Initializing control!");
#if CONFIG_TBD_PLATFORM_MK2
    DRIVERS::mk2::Init();
#elif CONFIG_TBD_PLATFORM_BBA
    Midi::Init();
#else
    DRIVERS::ADC::InitADCSystem();
    DRIVERS::GPIO::InitGPIO();
    CAL::Calibration::Init();
#endif
}

void CTAG::CTRL::Control::FlushBuffers() {
#ifdef CONFIG_TBD_PLATFORM_BBA
    Midi::Flush();
#endif
}