#include <tbd/sounds/SoundProcessorMISVF.hpp>
#include <iostream>
#include <cmath>

using namespace tbd::sounds;

void SoundProcessorMISVF::Init(std::size_t blockSize, void *blockPtr) {
    adEnv.SetSampleRate(44100.f / bufSz);
    adEnv.SetModeExp();
    svf.Init();
}

void SoundProcessorMISVF::Process(const audio::ProcessData&data) {
    int mode = flt_mode;
    if (cv_flt_mode != -1) {
        mode = fabsf(data.cv[cv_flt_mode]) * 3;
    }
    if (mode > 2) mode = 2;
    if (mode < 0) mode = 0;

    // eg fm
    float fAttack, fDecay;
    float fAD = 0.f;
    if (enableEG == 1 && trig_enableEG != -1) {
        if (data.trig[trig_enableEG] != prevTrigState) {
            prevTrigState = data.trig[trig_enableEG];
            if (prevTrigState == 0) adEnv.Trigger();
        }
        adEnv.SetLoop(loopEG);
        fAttack = static_cast<float>(attack) / 4095.f;
        if (cv_attack != -1) {
            fAttack = fabsf(data.cv[cv_attack]);
        }
        fDecay = static_cast<float>(decay) / 4095.f;
        if (cv_decay != -1) {
            fDecay = fabsf(data.cv[cv_decay]);
        }
        adEnv.SetAttack(fAttack);
        adEnv.SetDecay(fDecay);
        fAD = adEnv.Process();
    }

    // modulation
    // gain is punch
    float fGain = static_cast<float>(gain) / 4095.f;
    float fCutoff = static_cast<float>(cutoff) / 4095.f;
    float fResonance = static_cast<float>(resonance) / 4095.f;
    float fFM = static_cast<float>(fm_amt) / 4095.f;
    if (cv_cutoff != -1) {
        fCutoff = data.cv[cv_cutoff] * data.cv[cv_cutoff];
    }
    if (cv_resonance != -1) {
        fResonance = fabsf(data.cv[cv_resonance]);
    }
    if (cv_gain != -1) {
        fGain = fabsf(data.cv[cv_gain]);
    }
    if (cv_fm_amt != -1) {
        // external CV
        fCutoff += fFM * data.cv[cv_fm_amt];
    } else if (enableEG == 1) {
        // internal AD env
        fCutoff += fFM * fAD;
    }

    int16_t f = static_cast<int16_t>(fCutoff * 16384.f);
    int16_t r = static_cast<int16_t>(fResonance * 32767.f);
    uint16_t p = static_cast<uint16_t>(fGain * 65535.f);
    if (r < 80) r = 80;
    if (f < 0) f = 0;
    svf.set_frequency(f);
    svf.set_resonance(r);
    svf.set_punch(p);
    svf.set_mode(static_cast<braids::SvfMode>(mode));

    for (int i = 0; i < bufSz; i++) {
        int32_t v = static_cast<int32_t>(data.buf[i * 2 + processCh] * 32767.f);
        data.buf[i * 2 + processCh] = static_cast<float>(svf.Process(v)) / 32767.f;
    }
}
