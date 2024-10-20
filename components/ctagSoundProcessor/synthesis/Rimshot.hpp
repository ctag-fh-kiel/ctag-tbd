/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2024 by Robert Manzke. All rights reserved.

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

#include "stmlib/dsp/filter.h"
#include "helpers/ctagADEnv.hpp"
#include "plaits/dsp/oscillator/sine_oscillator.h"

namespace CTAG::SYNTHESIS{
    class Rimshot {
		public:
			void Init();
    		void Trigger();
    		void Process(float* out, uint32_t size);

    		struct Params{
    			float f0; // 70/44100 .. 350/44100
    			float decay; // .1f .. .75f
				float accent; // 0.1f .. 1.f
    			float reso_hp; // 5.f .. 1.f
    			float base; // .35f, .65f
    			float noise_level; // 0.f, .2f
    		};
    		Params params;
        private:

    		float Diode(float x) {
    			if(x > 1.f) return 1.f;
    			if(x < -1.f) return -1.f;
    			return x;
    		}

            stmlib::Svf resonator[3];
        	stmlib::Svf hp;
        	plaits::SineOscillator osc;
        	SP::HELPERS::ctagADEnv env;

    		int pulse_remaining_samples_ {0};
        	float pulse_ {0.f};
        	float pulse_height_ {0.f};
        	float pulse_lp_ {0.f};
        	float noise_envelope_ {0.f};
        	float sustain_gain_ {0.f};

    };
};
