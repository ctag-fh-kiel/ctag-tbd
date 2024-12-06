#pragma once

#ifndef TBD_IS_AUDIO_LOOP_COMPILATION_UNIT
    #error "audio loop is performance critical, compile in a single compilation unit"
#endif

#include <tbd/sound_manager/audio_worker.hpp>

#include <tbd/sound_processor.hpp>
#include <tbd/system/cpu_cores.hpp>
#include <tbd/system/locks.hpp>
#include <helpers/ctagFastMath.hpp>
#include <tbd/common/audio.hpp>
#include <tbd/sound_registry/sound_processor_factory.hpp>
#include <tbd/sound_manager/common/module.hpp>
#include "input_manager.hpp"
#include "sound_level_worker.hpp"



#define NOISE_GATE_LEVEL_CLOSE 0.0001f
#define NOISE_GATE_LEVEL_OPEN 0.0003f
#define CPU_MAX_ALLOWED_CYCLES 174150 // is 32/44100kHz * 240MHz


namespace tbd::audio {

struct AudioProcessingParams {
    std::atomic<uint32_t> noiseGateCfg;
    std::atomic<uint32_t> ch01Daisy;
    std::atomic<uint32_t> toStereoCH0;
    std::atomic<uint32_t> toStereoCH1;

    std::atomic<uint32_t> ch0_outputSoftClip;
    std::atomic<uint32_t> ch1_outputSoftClip;
};


AudioProcessingParams params;
// static Lock audioTaskH, ledTaskH;
CTAG::SP::ctagSoundProcessor *sp[2];
system::Lock sound_processing_lock;


struct AudioConsumer {
    uint32_t startup();
    uint32_t consume(float* audio_slice);
    uint32_t cleanup();

private:
    fv3::dccut_f in_dccutl, in_dccutr;
    float peakIn = 0.f, peakOut = 0.f;
    float peakL = 0.f, peakR = 0.f;
    int ngState = NG_OPEN;
    float lramp[TBD_SAMPLES_PER_CHUNK];
    bool isStereoCH0 = false;
    CTAG::SP::ProcessData pd;
};


uint32_t AudioConsumer::startup() {
    //fv3::dccut_f out_dccutl, out_dccutr;
    in_dccutl.setCutOnFreq(3.7f, 44100.f);
    in_dccutr.setCutOnFreq(3.7f, 44100.f);
    /*
    out_dccutl.setCutOnFreq(3.7f, 44100.f);
    out_dccutr.setCutOnFreq(3.7f, 44100.f);
    */

    // generate linear ramp ]0,1[ squared
    for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
        lramp[i] = (float) (i + 1) / (float) (TBD_SAMPLES_PER_CHUNK + 1);
        lramp[i] *= lramp[i];
    }

    TBD_LOGI(tag, "initialized plugin audio worker");

    return 0;
}

uint32_t AudioConsumer::consume(float* audio_slice) {
    pd.buf = audio_slice;

    // update data from ADCs and GPIOs for real-time control
    InputManager::update(&pd.trig, &pd.cv);

    TBD_TIMEOUT_ENSURE_OPS_PER_SECOND(timeout, TBD_SAMPLE_RATE / TBD_SAMPLES_PER_CHUNK);
    SoundLevel sound_level(sound_level_worker);    

    // In peak detection
    // dc cut input
    float maxl = 0.f, maxr = 0.f;
    float max = 0.f;
    for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
        audio_slice[i * 2] = in_dccutl(audio_slice[i * 2]);
        float val = fabsf(audio_slice[i * 2]);
        if (val > maxl) maxl = val;
        audio_slice[i * 2 + 1] = in_dccutr(audio_slice[i * 2 + 1]);
        val = fabsf(audio_slice[i * 2 + 1]);
        if (val > maxr) maxr = val;
    }
    max = maxl >= maxr ? maxl : maxr;
    peakIn = 0.95f * peakIn + 0.05f * max;

    // noise gate
    if (params.noiseGateCfg == 1) { // both channels noise gate
        if (ngState == NG_OPEN && peakIn < NOISE_GATE_LEVEL_CLOSE) {
            ngState = NG_BOTH;
            for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) { // linearly ramp down buffer
                audio_slice[i * 2] *= lramp[TBD_SAMPLES_PER_CHUNK - 1 - i];
                audio_slice[i * 2 + 1] *= lramp[TBD_SAMPLES_PER_CHUNK - 1 - i];
            }
        } else if (ngState != NG_OPEN && peakIn > NOISE_GATE_LEVEL_OPEN) {
            ngState = NG_OPEN;
            for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) { // linearly ramp up buffer
                audio_slice[i * 2] *= lramp[i];
                audio_slice[i * 2 + 1] *= lramp[i];
            }
        } else if (ngState != NG_OPEN) {
            memset(audio_slice, 0, TBD_CHUNK_BUFFER_SIZE);
        }
    } else if (params.noiseGateCfg == 2) { // left channel
        peakL = 0.95f * peakL + 0.05f * maxl;
        if (ngState == NG_OPEN && peakL < NOISE_GATE_LEVEL_CLOSE) {
            ngState = NG_LEFT;
            for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {// linearly ramp down buffer
                audio_slice[i * 2] *= lramp[TBD_SAMPLES_PER_CHUNK - 1 - i];
            }
        } else if (ngState != NG_OPEN && peakL > NOISE_GATE_LEVEL_OPEN) {
            ngState = NG_OPEN;
            for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) { // linear ramp up
                audio_slice[i * 2] *= lramp[i];
            }
        } else if (ngState != NG_OPEN) {
            for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
                audio_slice[i * 2] = 0;
            }
        }
    } else if (params.noiseGateCfg == 3) { // right channel
        peakR = 0.95f * peakR + 0.05f * maxr;
        if (ngState == NG_OPEN && peakR < NOISE_GATE_LEVEL_CLOSE) {
            ngState = NG_RIGHT;
            for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {// linearly ramp down buffer
                audio_slice[i * 2 + 1] *= lramp[TBD_SAMPLES_PER_CHUNK - 1 - i];
            }
        } else if (ngState != NG_OPEN && peakR > NOISE_GATE_LEVEL_OPEN) {
            ngState = NG_OPEN;
            for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) { // linear ramp up
                audio_slice[i * 2 + 1] *= lramp[i];
            }
        } else if (ngState != NG_OPEN) {
            for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
                audio_slice[i * 2 + 1] = 0;
            }
        }
    }

    // led indicator, green for input
    max = 255.f + 3.2f * CTAG::SP::HELPERS::fast_dBV(peakIn); // cut away at approx -80dB
    //TBD_LOGI(tag, "Max %.9f %f", peakIn, max);
    if (max > 0 && ngState == NG_OPEN) {
        sound_level.add_input_level(max);
    }

    // FIXME: who owns the sound processors? Management of sound processors should 
    //        probably be done in the audio loop
    // sound processors
    {
        auto sound_processing_guard = sound_processing_lock.guard();
        if (sound_processing_guard) {
            // apply sound processors
            if (sp[0] != nullptr) {
                isStereoCH0 = sp[0]->GetIsStereo();
                sp[0]->Process(pd);
            }
            if (!isStereoCH0){
                // check if ch0 -> ch1 daisy chain, i.e. use output of ch0 as input for ch1
                if(params.ch01Daisy){
                    for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
                        audio_slice[i * 2 + 1] = audio_slice[i * 2];
                    }
                }
                if (sp[1] != nullptr) sp[1]->Process(pd); // 0 is not a stereo processor
            }
        } else {
            // mute audio
            memset(audio_slice, 0, TBD_SAMPLES_PER_CHUNK * 2 * sizeof(float));
        }
    }

    // to stereo conversion
    if (!isStereoCH0) {
        if (params.toStereoCH0 || params.toStereoCH1) {
            float sb[TBD_SAMPLES_PER_CHUNK * 2];
            memcpy(sb, audio_slice, TBD_SAMPLES_PER_CHUNK * 2 * sizeof(float));
            if (params.toStereoCH0 == 1 && params.toStereoCH1 == 0) { // spread CH0 to both channels
                for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
                    audio_slice[i * 2] = 0.5f * sb[i * 2];
                    audio_slice[i * 2 + 1] = 0.5f * sb[i * 2] + sb[i * 2 + 1];
                }
            } else if (params.toStereoCH1 == 1 && params.toStereoCH0 == 0) { // spread CH1 to both channels
                for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
                    audio_slice[i * 2] = 0.5f * sb[i * 2 + 1] + sb[i * 2];
                    audio_slice[i * 2 + 1] = 0.5f * sb[i * 2 + 1];
                }
            } else if (params.toStereoCH0 == 1 && params.toStereoCH1 == 1) { // spread CH0 + CH1 to both channels
                for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
                    audio_slice[i * 2] = audio_slice[i * 2 + 1] = 0.5f * (sb[i * 2] + sb[i * 2 + 1]);
                }
            } else if (params.toStereoCH0 == 2 && params.toStereoCH1 == 2) { // swap channels
                for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
                    audio_slice[i * 2] = sb[i * 2 + 1];
                    audio_slice[i * 2 + 1] = sb[i * 2];
                }
            } else if (params.toStereoCH0 == 2 && params.toStereoCH1 == 0) { // mix CH0 with CH1 on CH1
                for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
                    audio_slice[i * 2] = 0.f;
                    audio_slice[i * 2 + 1] += sb[i * 2];
                }
            } else if (params.toStereoCH0 == 0 && params.toStereoCH1 == 2) { // mix CH1 with CH0 on CH0
                for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
                    audio_slice[i * 2] += sb[i * 2 + 1];
                    audio_slice[i * 2 + 1] = 0.f;
                }
            } else if (params.toStereoCH0 == 2 && params.toStereoCH1 == 1) { // move CH0 to CH1, spread CH1 to both
                for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
                    audio_slice[i * 2] = 0.5f * sb[i * 2 + 1];
                    audio_slice[i * 2 + 1] = 0.5f * sb[i * 2 + 1] + sb[i * 2];
                }
            } else if (params.toStereoCH0 == 1 && params.toStereoCH1 == 2) { // move CH1 to CH0, spread CH0 to both
                for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
                    audio_slice[i * 2] = 0.5f * sb[i * 2] + sb[i * 2 + 1];
                    audio_slice[i * 2 + 1] = 0.5f * sb[i * 2];
                }
            }
        }
    }

    // Out peak detection, red for output
    // limiting output
    max = 0.f;
    for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
        // soft limiting
        if (params.ch0_outputSoftClip) {
            audio_slice[i * 2] = stmlib::SoftClip(audio_slice[i * 2]);
        }
        if (params.ch1_outputSoftClip) {
            audio_slice[i * 2 + 1] = stmlib::SoftClip(audio_slice[i * 2 + 1]);
        }
        //if (audio_slice[i * 2] > max) max = audio_slice[i * 2];
        //if (audio_slice[i * 2 + 1] > max) max = audio_slice[i * 2 + 1];
    }


    // just take first sample of block for level meter
    max = fabsf(audio_slice[0] + audio_slice[1]) / 2.f;
    peakOut = 0.9f * peakOut + 0.1f * max;
    //TBD_LOGW(tag, "max %.12f, peak %.12f", max, peakOut);
    max = 255.f + 3.2f * CTAG::SP::HELPERS::fast_dBV(peakOut);
    if (max > 0.f) {
        sound_level.add_output_level(max);
    }

    if (!timeout) {
        sound_level.set_warning();
    }
    return 0;
}


uint32_t AudioConsumer::cleanup() {
    TBD_LOGI(tag, "plugin audio worker shutting down");
    return 0;
}


AudioWorker<AudioConsumer, system::CpuCore::audio> audio_worker("audio_worker");

}
