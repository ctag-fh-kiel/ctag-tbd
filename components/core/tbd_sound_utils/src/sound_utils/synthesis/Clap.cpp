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

#include <tbd/sound_utils/synthesis/Clap.hpp>
#include "stmlib/utils/random.h"

namespace tbd::sound_utils::synthesis {

void Clap::Init(){
    svf1.Init();
    svf2.Init();
    env1.SetSampleRate(44100.f);
    env1.SetModeExp();
    env1.SetAttack(0.f);
    env2.SetSampleRate(44100.f);
    env2.SetModeExp();
    env2.SetAttack(0.f);

    timers[0].SetTimeoutCallback([&](){a1 = amplitudes[params.transient][0]; env1.Trigger();});
    timers[1].SetTimeoutCallback([&](){a1 = amplitudes[params.transient][1]; env1.Trigger();});
    timers[2].SetTimeoutCallback([&](){a1 = amplitudes[params.transient][2]; env1.Trigger();});
    timers[3].SetTimeoutCallback([&](){a2 = amplitudes[params.transient][3]; env2.Trigger();});
}

void Clap::Trigger(){
    env1.Trigger();
    timers[0].SetTimeout(delays[params.transient][0] * params.scale);
    timers[1].SetTimeout(delays[params.transient][1] * params.scale);
    timers[2].SetTimeout(delays[params.transient][2] * params.scale);
    timers[3].SetTimeout(delays[params.transient][3] * params.scale);
}

void Clap::Process(float* out, uint32_t size){
    svf1.set_f_q<stmlib::FREQUENCY_DIRTY>(params.pitch1, params.reso1);
    svf2.set_f_q<stmlib::FREQUENCY_DIRTY>(params.pitch2, params.reso2);
    env1.SetDecay(params.decay1);
    env2.SetDecay(params.decay2);
    env2.SetAttack(params.attack);

    for(int i=0;i<size;i++){
        timers[0].Tick();
        timers[1].Tick();
        timers[2].Tick();
        timers[3].Tick();
        float noise = stmlib::Random::GetFloat();
        noise = noise > 0.5f ? 1.f : -1.f;
        float transient = svf1.Process<stmlib::FILTER_MODE_BAND_PASS>(noise);
        transient *= env1.Process() * a1;
        float tail = svf2.Process<stmlib::FILTER_MODE_BAND_PASS>(noise);
        tail *= env2.Process() * a2;
        *out++ = transient + tail;
    }
}

}
