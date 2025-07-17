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

#pragma once
#include <cstdio>

// abstract filter superclass

namespace tbd::sound_utils {
    class ctagFilterBase {
    public:

        virtual ~ctagFilterBase() {};

        virtual void SetCutoff(float cutoff) {
            cutoff_ = cutoff;
        };

        virtual void SetResonance(float resonance) {
            resonance_ = resonance;
        };

        virtual void SetSampleRate(float fs) {
            fs_ = fs;
        };

        virtual float Process(float in) = 0;

        virtual void Init() = 0;

        virtual void SetGain(float gain) {
            gain_ = gain;
        }

        virtual void Debug(){
            printf("fs, cutoff, resonance, gain, %f, %f, %f, %f", fs_, cutoff_, resonance_, gain_);
        }

    protected:
        float fs_ = 44100.f;
        float cutoff_ = 0.5f;
        float resonance_ = 0.f;
        float gain_ = 1.f;
    };
}

