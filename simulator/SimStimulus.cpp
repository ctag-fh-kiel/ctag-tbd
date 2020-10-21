/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "SimStimulus.hpp"
#include <string>


SimStimulus::SimStimulus() {

}

SimStimulus::~SimStimulus() {

}

void SimStimulus::Process(float *cvpot, uint8_t *trig) {
    if(modeMutex.try_lock()){
        for(int i=2;i<6;i++){
            cvpot[i-2] = s[i].Process();
        }
        trig[0] = s[0].Process() >= 0.5f ? 0 : 1;
        trig[1] = s[1].Process() >= 0.5f ? 0 : 1;
        modeMutex.unlock();
    }
}

void SimStimulus::UpdateStimulus(const int *mode, const int *value) {
    modeMutex.lock();
    for(int i=0;i<6;i++){
        s[i].SetMode(mode[i]);
        s[i].SetValue(value[i]);
    }
    modeMutex.unlock();
}
