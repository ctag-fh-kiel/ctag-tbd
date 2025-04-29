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

#include <tbd/sounds/SoundProcessorMIVerb2.hpp>
#include <iostream>
#include "clouds/dsp/frame.h"
#include <tbd/sound_utils/ctagFastMath.hpp>
#include <tbd/logging.hpp>
#include <tbd/heaps.hpp>


namespace heaps = tbd::heaps;

using namespace tbd::sounds;

void SoundProcessorMIVerb2::Init(std::size_t blockSize, void *blockPtr) {
    reverb_buffer = (float *) heaps::malloc(32768 * sizeof(float),
                                               TBD_HEAPS_SPIRAM);//MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (reverb_buffer == NULL) {
        TBD_LOGE("MIVerb", "Could not allocate shared buffer!");
    }

    reverb.Init(reverb_buffer);
}

void SoundProcessorMIVerb2::Process(const audio::ProcessData&data) {
    float fDecay = decay / 4095.f;
    if (cv_decay != -1) {
        fDecay = sound_utils::fastfabs(data.cv[cv_decay]);
    }
    float fSize = size / 4095.f;
    if (cv_size != -1) {
        fSize = sound_utils::fastfabs(data.cv[cv_size]);
    }
    float fGain = in_gain / 4095.f;
    if (cv_in_gain != -1) {
        fGain = sound_utils::fastfabs(data.cv[cv_in_gain]);
    }
    float fDiffusion = diffusion / 4095.f;
    if (cv_diffusion != -1) {
        fDiffusion = sound_utils::fastfabs(data.cv[cv_diffusion]);
    }
    float fLp = lp / 4095.f;
    if (cv_lp != -1) {
        fLp = sound_utils::fastfabs(data.cv[cv_lp]);
    }
    float fHp = hp / 4095.f;
    if (cv_hp != -1) {
        fHp = sound_utils::fastfabs(data.cv[cv_hp]);
    }
    float fModAmt = mod_amount / 4095.f;
    if (cv_mod_amount != -1) {
        fModAmt = sound_utils::fastfabs(data.cv[cv_mod_amount]);
    }
    float fModRate = mod_rate / 4095.f;
    if (cv_mod_rate != -1) {
        fModRate = sound_utils::fastfabs(data.cv[cv_mod_rate]);
    }
    float fRatio = ratio / 4095.f;
    if (cv_ratio != -1) {
        fRatio = sound_utils::fastfabs(data.cv[cv_ratio]);
    }
    float fPShftAmt = p_shift_amt / 4095.f;
    if (cv_p_shift_amt != -1) {
        fPShftAmt = sound_utils::fastfabs(data.cv[cv_p_shift_amt]);
    }
    float fAmount = amount / 4095.f;
    if (cv_amount != -1) {
        fAmount = sound_utils::fastfabs(data.cv[cv_amount]);
    }

    reverb.set_decay(fDecay);
    reverb.set_ratio(fRatio);
    reverb.set_size(fSize);
    reverb.set_diffusion(fDiffusion);
    reverb.set_lp(fLp);
    reverb.set_input_gain(fGain);
    reverb.set_hp(fHp);
    reverb.set_mod_amount(fModAmt);
    reverb.set_mod_rate(fModRate);
    reverb.set_pitch_shift_amount(fPShftAmt);
    reverb.set_amount(fAmount);

    clouds::FloatFrame frames[bufSz];
    for (int i = 0; i < bufSz; i++) {
        frames[i].l = data.buf[i * 2];
        frames[i].r = data.buf[i * 2 + 1];
    }

    reverb.Process(frames, bufSz);

    for (int i = 0; i < bufSz; i++) {
        data.buf[i * 2] = frames[i].l;
        data.buf[i * 2 + 1] = frames[i].r;
    }

}

SoundProcessorMIVerb2::~SoundProcessorMIVerb2() {
    heaps::free(reverb_buffer);
}
