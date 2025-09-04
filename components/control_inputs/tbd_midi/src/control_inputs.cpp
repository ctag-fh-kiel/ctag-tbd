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
#include <tbd/control_inputs/module.hpp>
#include <tbd/control_inputs.hpp>

#include <tbd/logging.hpp>
#include <tbd/control_inputs/midi.hpp>

using CVInput = tbd::drivers::Midi;


using tbd::control_inputs::tag;

namespace tbd {

void ControlInputs::init() {
    TBD_LOGI(tag, "initializing input manager");

    CVInput::init();
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

    // raw output contains both CVs and triggers in the correct format
    *cvs = reinterpret_cast<float*>(data);
    *trigs = &data[N_CVS * 4];
}

void ControlInputs::update_metrics(const sound_processor::ProcessingMetrics& metrics) {

}

void ControlInputs::SetCVChannelBiPolar(const bool &v0, const bool &v1, const bool &v2, const bool &v3) {

}


void ControlInputs::flush() {
    CVInput::flush();
}

}
