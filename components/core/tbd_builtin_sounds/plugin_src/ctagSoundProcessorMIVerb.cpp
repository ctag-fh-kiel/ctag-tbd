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

#include <tbd/sounds/SoundProcessorMIVerb.hpp>
#include <iostream>
#include <tbd/sound_utils/ctagFastMath.hpp>
#include <tbd/logging.hpp>
#include <tbd/heaps.hpp>


namespace heaps = tbd::heaps;

using namespace tbd::sounds;

void SoundProcessorMIVerb::Init(std::size_t blockSize, void *blockPtr) {
    reverb_buffer = (float *) heaps::malloc(32768 * sizeof(float),
                                               TBD_HEAPS_SPIRAM);//MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (reverb_buffer == NULL) {
        TBD_LOGE("MIVerb", "Could not allocate shared buffer!");
    }

    reverb.Init(reverb_buffer);
    reverb.set_amount(0.5f);
    reverb.set_lp(0.5f);
    reverb.set_lfo2_freq(0.1f);
    reverb.set_lfo1_freq(0.2f);
    reverb.set_time(0.2f);
    reverb.set_diffusion(0.5f);
}

void SoundProcessorMIVerb::Process(const audio::ProcessData&data) {

    float fTime = time / 4095.f;
    float fDiffusion = diffusion / 4095.f;
    float fLp = lp / 4095.f;
    float fGain = in_gain / 4095.f;
    float fAmount = amount / 4095.f;
    float lfo1 = lfo1_f / 4095.f;
    float lfo2 = lfo2_f / 4095.f;

    if (cv_time != -1) {
        fTime = sound_utils::fastfabs(data.cv[cv_time]);
    }
    if (cv_diffusion != -1) {
        fDiffusion = sound_utils::fastfabs(data.cv[cv_diffusion]);
    }
    if (cv_lp != -1) {
        fLp = sound_utils::fastfabs(data.cv[cv_lp]);
    }
    if (cv_in_gain != -1) {
        fGain = sound_utils::fastfabs(data.cv[cv_in_gain]);
    }
    if (cv_amount != -1) {
        fAmount = sound_utils::fastfabs(data.cv[cv_amount]);
    }
    if (cv_lfo1_f != -1) {
        lfo1 = sound_utils::fastfabs(data.cv[cv_lfo1_f]);
    }
    if (cv_lfo2_f != -1) {
        lfo2 = sound_utils::fastfabs(data.cv[cv_lfo2_f]);
    }

    reverb.set_diffusion(fDiffusion);
    reverb.set_input_gain(fGain);
    reverb.set_amount(fAmount);
    reverb.set_lp(fLp);
    reverb.set_time(fTime);
    reverb.set_lfo1_freq(lfo1);
    reverb.set_lfo2_freq(lfo2);

    float left[bufSz], right[bufSz];
    for (int i = 0; i < bufSz; i++) {
        left[i] = data.buf[i * 2];
        right[i] = data.buf[i * 2 + 1];
    }

    reverb.Process(left, right, bufSz);

    for (int i = 0; i < bufSz; i++) {
        data.buf[i * 2] = left[i];
        data.buf[i * 2 + 1] = right[i];
    }
}

SoundProcessorMIVerb::~SoundProcessorMIVerb() {
    heaps::free(reverb_buffer);
}
