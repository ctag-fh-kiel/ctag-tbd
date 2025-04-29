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


#include <tbd/sounds/SoundProcessorDLoop.hpp>
#include <tbd/sound_utils/ctagFastMath.hpp>
#include <iostream>
#include <cmath>

using namespace tbd::sounds;

void SoundProcessorDLoop::Init(std::size_t blockSize, void *blockPtr) {
    // init params
    d.SetBipolar(false);
    d.ReSeed(seed);
    d_pan.SetBipolar(true);
    d_pan.ReSeed(seed + ofssspread);
    d_level.SetBipolar(false);
    d_level.ReSeed(seed + ofsvspread);
    s_l.SetBipolar(false);
    s_l.ReSeed(seed + 1234);
    s_r.SetBipolar(false);
    s_r.ReSeed(seed + 5678);

    loopCntr = 0;
    lastReset = 0;
    dccutl.setCutOnFreq(20, 44100.f);
    dccutr.setCutOnFreq(20, 44100.f);
    decL.SetSampleRate(44100.f);
    decL.SetCoeffSmoothing(false);
    decR.SetSampleRate(44100.f);
    decR.SetCoeffSmoothing(false);

}

void SoundProcessorDLoop::Process(const audio::ProcessData&data) {
    // params and CV / trig mods
    float thres = (float) density / 4095.f;
    if (cv_density != -1) thres = data.cv[cv_density];
    thres *= thres;
    thres = 1.f - thres;
    float flevel = (float) level / 4095.f;
    if (cv_level != -1) flevel = data.cv[cv_level];
    flevel *= flevel;
    flevel *= 2.f;
    float fsspread = (float) sspread / 4095.f; // stereo spread
    if (cv_sspread != -1) fsspread = data.cv[cv_sspread] * data.cv[cv_sspread];
    float fvspread = (float) vspread / 4095.f;
    if (cv_vspread != -1) fvspread = data.cv[cv_vspread] * data.cv[cv_vspread];
    fvspread *= -100.f; // volume variance
    uint32_t isLoop = loop;
    if (trig_loop != -1) isLoop = data.trig[trig_loop] == 1 ? 0 : 1;
    uint32_t sequenceLength = slen;
    if (cv_slen != -1) sequenceLength = (uint32_t) (data.cv[cv_slen] * data.cv[cv_slen] * 100000.f);
    uint32_t sequenceSeed = seed;
    if (cv_seed != -1)
        sequenceSeed = (uint32_t) (data.cv[cv_seed] * data.cv[cv_seed] *
                                   1024.f); // limit range for more stability due to adc noise
    uint32_t isReset = reset;
    if (trig_reset != -1) isReset = data.trig[trig_reset] == 1 ? 0 : 1;
    uint32_t isRandomShaping = s_enable;// == 1 ? 0 : 1;
    if (trig_s_enable != -1) isRandomShaping = data.trig[trig_s_enable] == 1 ? 0 : 1;
    float decay = (float) s_decay / 4095.f * 0.05f;
    if (cv_s_decay != -1) decay = data.cv[cv_s_decay] * data.cv[cv_s_decay] * 0.05f;
    decL.SetDecay60dB(decay);
    decR.SetCoeff(decL.GetCoeff());
    float rShape = (float) s_rlevel / 4095.f;
    if (cv_s_rlevel != -1) rShape = data.cv[cv_s_rlevel] * data.cv[cv_s_rlevel];

    // create audio buffer
    if (loopCntr < sequenceLength) {
        for (uint32_t i = 0; i < bufSz; i++) {
            float val = d.Process();
            val = val > thres ? 1.0f : 0.f; // dust grain
            float l = d_level.Process(); // random
            l *= fvspread;
            val *= fast_VdB(l);
            val *= flevel;
            float pan = d_pan.Process() * 0.5f;
            float t = val * (0.5f + fsspread * pan);
            if (t > 1.f) t = 1.f;
            if (isRandomShaping && t > 0.001f) {
                float a = 4.f * rShape * s_l.Process();
                if (a > 1.f)a = 0.99999999999999999f;
                decL.SetCoeff(a);
            }
            t = decL.Process(t);
            data.buf[i * 2] = dccutl.process(t); // left minus dc offset
            t = val * (0.5f - fsspread * pan);
            if (t > 1.f) t = 1.f;
            if (isRandomShaping && t > 0.001f) {
                float a = 4.f * rShape * s_r.Process();
                if (a > 1.f)a = 0.99999999999999999f;
                decR.SetCoeff(a);
            }
            t = decR.Process(t);

            data.buf[i * 2 + 1] = dccutr.process(t);// right minus dc offset
        }
    }

    // end of loop? restart
    uint32_t ofsStereoSpread = ofssspread;
    if (cv_ofssspread != -1) ofsStereoSpread = data.cv[cv_ofssspread] * 10000.f;
    uint32_t ofsVolumeSpread = ofsvspread;
    if (cv_ofsvspread != -1) ofsVolumeSpread = data.cv[cv_ofsvspread] * 10000.f;
    if ((loopCntr >= sequenceLength && isLoop == 1) || (isReset == 1 && lastReset == 0)) {
        loopCntr = 0;
        d.ReSeed(sequenceSeed);
        d_pan.ReSeed(sequenceSeed + ofsStereoSpread);
        d_level.ReSeed(sequenceSeed + ofsVolumeSpread);
        s_l.ReSeed(sequenceSeed + 1234);
        s_r.ReSeed(sequenceSeed + 5678);
    } else {
        loopCntr++;
    }
    lastReset = isReset;
}
