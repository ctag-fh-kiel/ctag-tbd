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

#include <tbd/sound_utils/synthesis/Rimshot.hpp>
#include "stmlib/dsp/units.h"
#include "stmlib/utils/random.h"

void CTAG::SYNTHESIS::Rimshot::Init(){
    pulse_remaining_samples_ = 0;
    pulse_ = 0.0f;
    pulse_height_ = 0.0f;
    pulse_lp_ = 0.0f;
    noise_envelope_ = 0.0f;
    sustain_gain_ = 0.0f;
    resonator[0].Init();
    resonator[1].Init();
    resonator[2].Init();
    hp.Init();
    osc.Init();
    env.SetSampleRate(44100.f);
    env.SetAttack(0.f);
    env.SetModeExp();
}

void CTAG::SYNTHESIS::Rimshot::Trigger(){
    const int kTriggerPulseDuration = .25e-3f * 44100.f;
    pulse_remaining_samples_ = kTriggerPulseDuration;
    pulse_height_ = 3.0f + 7.0f * params.accent;
    noise_envelope_ = 2.0f;
    env.Trigger();
}

void CTAG::SYNTHESIS::Rimshot::Process(float* out, uint32_t size){
    float f[3];
    const float decay_xt = params.decay * (1.0f + params.decay  * (params.decay  - 1.0f));
    const float kPulseDecayTime = 0.05e-3f * 44100.f;
    const float q = 250.0f * stmlib::SemitonesToRatio(decay_xt * 84.0f);

    f[0] = params.f0;
    resonator[0].set_f_q<stmlib::FREQUENCY_DIRTY>(f[0], 1.f + f[0] * q * 0.5f);
    f[1] = params.f0 * 2.2727f;
    resonator[1].set_f_q<stmlib::FREQUENCY_DIRTY>(f[1], 1.f + f[1] * q * 0.25f);
    f[2] = params.f0 * 4.5454f;
    resonator[2].set_f_q<stmlib::FREQUENCY_DIRTY>(f[2], 1.f + f[2] * q * 0.175f);


    float gain[3] = {1.0f, .5f, 1.f};
    hp.set_f_q<stmlib::FREQUENCY_DIRTY>(params.f0*2.f, params.reso_hp);

    env.SetDecay(params.decay);
    for(int i=0;i<32;i++){
        float pulse = 0.0f;
        if (pulse_remaining_samples_) {
            --pulse_remaining_samples_;
            pulse = pulse_remaining_samples_ ? pulse_height_ : pulse_height_ - 1.0f;
            pulse_ = pulse;
        } else {
            pulse_ *= 1.0f - 1.0f / kPulseDecayTime;
            pulse = pulse_;
        }
        ONE_POLE(pulse_lp_, pulse, params.base);
        float shell = 0.0f;
        for (int i = 0; i < 3; ++i) {
            float excitation = i == 0
                ? (pulse - pulse_lp_) + 0.006f * pulse
                : 0.025f * pulse;
            float val = resonator[i].Process<stmlib::FILTER_MODE_BAND_PASS>(excitation);
            shell += gain[i] * val;
        }
        float envelope = env.Process();
        float noise_base = envelope * (osc.Next(f[0]) + 0.25f + stmlib::Random::GetFloat());
        *out++ = hp.Process<stmlib::FILTER_MODE_HIGH_PASS>(Diode(50.f * shell) + params.noise_level * noise_base);
    }
}
