#include <tbd/basic_sounds/sine_source.hpp>
#include <tbd/sound_utils/ctagFastMath.hpp>
#include <tbd/audio_device/audio_settings.hpp>

namespace tbd::basic_sounds {
void SoundProcessorSineSrc::init() {
    // init params
    sineSource.SetSampleRate(TBD_SAMPLE_RATE);
    sineSource.SetFrequency(1000.f);
    // ad envs
    adEnv.SetSampleRate(TBD_SAMPLE_RATE);
    adEnv.SetModeExp();
    pitchEnv.SetSampleRate(TBD_SAMPLE_RATE);
    pitchEnv.SetModeExp();
}

void SoundProcessorSineSrc::process(const sound_processor::ProcessData&data) {
    float deltaCVLoud = (loudness - preCVLoudness) / (float) TBD_SAMPLES_PER_CHUNK; // for linear CV interpolation
    // cv frequency
    if (cv_frequency != -1) {
        preCVFrequency = 0.3f * data.cv[cv_frequency] + 0.7f * preCVFrequency; // smooth CV
        float fMod = preCVFrequency * 5.f;
        fMod = tbd::sound_utils::fastpow2(fMod);
        freq *= fMod;
    }
    // eg pitch
    if (enable_eg) {
        if (enable_eg != prevTrigState_p) {
            prevTrigState = enable_eg;
            if (prevTrigState_p == 0) pitchEnv.Trigger();
        }
        pitchEnv.SetLoop(loopEG_p);
        attackVal_p = (float) attack_p / 4095.f * 5.f;
        if (cv_attack_p != -1) {
            attackVal_p = data.cv[cv_attack_p] * data.cv[cv_attack_p];
        }
        decayVal_p = (float) decay_p / 4095.f * 5.f;
        if (cv_decay_p != -1) {
            decayVal_p = data.cv[cv_decay_p] * data.cv[cv_decay_p];
        }
        pitchEnv.SetAttack(attackVal_p);
        pitchEnv.SetDecay(decayVal_p);
    }
    // parameter control
    loud = (float) loudness / 4095.f;
    //  eg loud
    if (enableEG == 1 && trig_enableEG != -1) {
        if (data.trig[trig_enableEG] != prevTrigState) {
            prevTrigState = data.trig[trig_enableEG];
            if (prevTrigState == 0) adEnv.Trigger();
        }
        adEnv.SetLoop(loopEG);
        attackVal = (float) attack / 4095.f * 5.f;
        if (cv_attack != -1) {
            attackVal = data.cv[cv_attack] * data.cv[cv_attack] * 2.f;
        }
        decayVal = (float) decay / 4095.f * 10.f;
        if (cv_decay != -1) {
            decayVal = data.cv[cv_decay] * data.cv[cv_decay] * 4.f;
        }
        adEnv.SetAttack(attackVal);
        adEnv.SetDecay(decayVal);
    }
    // here is the oscillator
    float freqb;
    for (int i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) { // iterate all channel samples
        freqb = frequency;
        // pitch fm
        if (enableEG_p == 1 && trig_enableEG_p != -1) {
            float val;
            if (cv_amount_p == -1)
                val = tbd::sound_utils::fasterpow2(pitchEnv.Process() * (float) amount_p / 4095.f * 8.f);
            else
                val = tbd::sound_utils::fasterpow2(pitchEnv.Process() * data.cv[cv_amount_p] * 8.f);
            freqb *= val;
            if (freqb > 10000.f) freqb = 1000.f;
            if (freqb < 15.f) freqb = 15.f;
        }
        // freq
        sineSource.SetFrequency(freqb);
        // get samples
        data.buf[i * 2 + this->processCh] = sineSource.Process() * loud;
        // apply loud EG
        if (enableEG == 1) {
            data.buf[i * 2 + this->processCh] *= adEnv.Process();
        }
        // apply CV loud control
        if (cv_loudness != -1) {
            data.buf[i * 2 + this->processCh] *=
                    preCVLoudness * preCVLoudness; // linearly interpolate CV data, square for loudness perception
            preCVLoudness += deltaCVLoud;
        }
    }
}

}
