#pragma once
#include <tbd/audio/audio_consumer.hpp>

#ifndef TBD_IS_AUDIO_LOOP_COMPILATION_UNIT
    #error "audio loop is performance critical, compile in a single compilation unit"
#endif

#include <future>
#include <tbd/audio/module.hpp>
#include <tbd/audio_device/audio_settings.hpp>
#include <tbd/control_inputs.hpp>
#include <tbd/sound_processor.hpp>
#include <tbd/system/locks.hpp>
#include <tbd/errors.hpp>

#include <freeverb3/efilter.hpp>
#include <stmlib/dsp/dsp.h>

#define NOISE_GATE_LEVEL_CLOSE 0.0001f
#define NOISE_GATE_LEVEL_OPEN 0.0003f
#define CPU_MAX_ALLOWED_CYCLES 174150 // is 32/44100kHz * 240MHz


TBD_NEW_ERR(SOUND_FAILED_TO_ACQUIRE_LOCK, "failed to acquire audio lock");
TBD_NEW_ERR(SOUND_INVALID_SOUND_PROCESSOR, "sound processor is null");
TBD_NEW_ERR(SOUND_BAD_CHANNEL_ASSIGNMENT, "bad channel selection for plugin");

namespace tbd::audio {

inline sound_processor::SoundProcessor* AudioConsumer::get_sound_processor(sound_processor::channels::ChannelID channel) {
    using namespace sound_processor::channels;

    const auto sound_processing_guard = sound_processing_lock.guard();
    if (!sound_processing_guard) {
        return nullptr;
    }
    if (channel == CH_0) {
        return left;
    }
    if (channel == CH_1) {
        if (left != nullptr && left->is_stereo()) {
            return left;
        }
        return right;
    }
    return nullptr;
}

inline Error AudioConsumer::set_sound_processor(const sound_processor::channels::Channels channels, sound_processor::SoundProcessor* sound_processor) {
    using namespace tbd::sound_processor::channels;

    const auto sound_processing_guard = sound_processing_lock.guard();
    if (!sound_processing_guard) {
        return TBD_ERR(SOUND_FAILED_TO_ACQUIRE_LOCK);
    }
    if (sound_processor == nullptr) {
        return TBD_ERR(SOUND_INVALID_SOUND_PROCESSOR);
    }
    if (sound_processor->is_stereo()) {
        if (channels != CM_BOTH) {
            return TBD_ERR(SOUND_BAD_CHANNEL_ASSIGNMENT);
        }
        left = sound_processor;
        right = nullptr;
        return TBD_OK;
    }
    if (channels == CM_LEFT) {
        left = sound_processor;
        return TBD_OK;
    }
    if (channels == CM_RIGHT) {
        right = sound_processor;
        return TBD_OK;
    }
    return TBD_ERR(SOUND_BAD_CHANNEL_ASSIGNMENT);
}

inline Error AudioConsumer::reset_sound_processor(const sound_processor::channels::Channels channels) {
    using namespace sound_processor::channels;

    const auto sound_processing_guard = sound_processing_lock.guard();
    if (!sound_processing_guard) {
        return TBD_ERR(SOUND_FAILED_TO_ACQUIRE_LOCK);
    }

    if (!(channels & CM_BOTH)) {
        return TBD_ERR(SOUND_BAD_CHANNEL_ASSIGNMENT);
    }

    if (channels & CM_LEFT) {
        left = nullptr;
    }
    if (channels & CM_RIGHT) {
        right = nullptr;
    }
    return TBD_OK;
}


Error AudioConsumer::startup() {
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

    return TBD_OK;
}

inline void AudioConsumer::apply_bandwidth_filter(float* audio_slice) {
    // In peak detection
    // dc cut input
    float maxl = 0.f;
    float maxr = 0.f;
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
    peakL = 0.95f * peakL + 0.05f * maxl;
    peakR = 0.95f * peakR + 0.05f * maxr;
}

inline void AudioConsumer::determine_input_level(AudioMetrics& metrics) {
    float max = 255.f + 3.2f * sound_utils::fast_dBV(peakIn); // cut away at approx -80dB
    //TBD_LOGI(tag, "Max %.9f %f", peakIn, max);
    if (max > 0 && noise_gate_is_open) {
        metrics.input_level = max;
    }
}

inline void AudioConsumer::apply_noise_gate(float* audio_slice) {
    if (params.noiseGateCfg == NG_BOTH) { // both channels noise gate
        if (noise_gate_is_open && peakIn < NOISE_GATE_LEVEL_CLOSE) {
            noise_gate_is_open = false;
            for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) { // linearly ramp down buffer
                audio_slice[i * 2] *= lramp[TBD_SAMPLES_PER_CHUNK - 1 - i];
                audio_slice[i * 2 + 1] *= lramp[TBD_SAMPLES_PER_CHUNK - 1 - i];
            }
        } else if (!noise_gate_is_open && peakIn > NOISE_GATE_LEVEL_OPEN) {
            noise_gate_is_open = true;
            for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) { // linearly ramp up buffer
                audio_slice[i * 2] *= lramp[i];
                audio_slice[i * 2 + 1] *= lramp[i];
            }
        } else if (!noise_gate_is_open) {
            memset(audio_slice, 0, TBD_CHUNK_BUFFER_SIZE);
        }
    } else if (params.noiseGateCfg == 2) { // left channel
        if (noise_gate_is_open && peakL < NOISE_GATE_LEVEL_CLOSE) {
            noise_gate_is_open = false;
            for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {// linearly ramp down buffer
                audio_slice[i * 2] *= lramp[TBD_SAMPLES_PER_CHUNK - 1 - i];
            }
        } else if (!noise_gate_is_open && peakL > NOISE_GATE_LEVEL_OPEN) {
            noise_gate_is_open = true;
            for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) { // linear ramp up
                audio_slice[i * 2] *= lramp[i];
            }
        } else if (!noise_gate_is_open) {
            for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
                audio_slice[i * 2] = 0;
            }
        }
    } else if (params.noiseGateCfg == 3) { // right channel
        if (noise_gate_is_open && peakR < NOISE_GATE_LEVEL_CLOSE) {
            noise_gate_is_open = false;
            for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {// linearly ramp down buffer
                audio_slice[i * 2 + 1] *= lramp[TBD_SAMPLES_PER_CHUNK - 1 - i];
            }
        } else if (!noise_gate_is_open && peakR > NOISE_GATE_LEVEL_OPEN) {
            noise_gate_is_open = true;
            for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) { // linear ramp up
                audio_slice[i * 2 + 1] *= lramp[i];
            }
        } else if (!noise_gate_is_open) {
            for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
                audio_slice[i * 2 + 1] = 0;
            }
        }
    }
}

inline sound_processor::channels::Channels AudioConsumer::run_processors(float* audio_slice) {
    using namespace sound_processor::channels;

    const auto sound_processing_guard = sound_processing_lock.guard();
    Channels channels = CM_NONE;
    if (sound_processing_guard) {
        if (left != nullptr) {
            channels = left->is_stereo() ? CM_BOTH : CM_LEFT;
            left->process(channels, pd);
        }
        if (!(channels & CM_RIGHT)){
            // check if ch0 -> ch1 daisy chain, i.e. use output of ch0 as input for ch1
            if(params.ch01Daisy){
                for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
                    audio_slice[i * 2 + 1] = audio_slice[i * 2];
                }
            }
            if (right != nullptr) {
                right->process(CM_RIGHT, pd);
                channels = static_cast<Channels>(channels | CM_RIGHT);
            }
        }
    }

    // FIXME: mute or pass through, when no processors present
    // if (channels == CM_NONE) {
    //     // mute audio
    //     memset(audio_slice, 0, TBD_SAMPLES_PER_CHUNK * 2 * sizeof(float));
    // }
    return channels;
}

inline void AudioConsumer::apply_output_mapping(float* audio_slice) {
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

inline void AudioConsumer::apply_softclip(float* audio_slice) {
    for (uint32_t i = 0; i < TBD_SAMPLES_PER_CHUNK; i++) {
        if (params.ch0_outputSoftClip) {
            audio_slice[i * 2] = stmlib::SoftClip(audio_slice[i * 2]);
        }
        if (params.ch1_outputSoftClip) {
            audio_slice[i * 2 + 1] = stmlib::SoftClip(audio_slice[i * 2 + 1]);
        }
    }
}

inline void AudioConsumer::determine_output_level(float* audio_slice, AudioMetrics& metrics) {
    // just take first sample of block for level meter
    float max = fabsf(audio_slice[0] + audio_slice[1]) / 2.f;
    peakOut = 0.9f * peakOut + 0.1f * max;
    //TBD_LOGW(tag, "max %.12f, peak %.12f", max, peakOut);
    max = 255.f + 3.2f * tbd::sound_utils::fast_dBV(peakOut);
    if (max > 0.f) {
        metrics.output_level = max;
    }
}

Error AudioConsumer::consume(float* audio_slice, AudioMetrics& metrics) {
    using namespace sound_processor::channels;
    pd.buf = audio_slice;

    // update data from ADCs and GPIOs for real-time control
    ControlInputs::update(&pd.trig, &pd.cv);

    TBD_TIMEOUT_ENSURE_OPS_PER_SECOND(timeout, TBD_SAMPLE_RATE / TBD_SAMPLES_PER_CHUNK);

    apply_bandwidth_filter(audio_slice);
    determine_input_level(metrics);
    apply_noise_gate(audio_slice);
    const auto channels = run_processors(audio_slice);
    if (channels == CM_NONE) {
        return TBD_OK;
    }

    if (channels == CM_BOTH) {
        apply_output_mapping(audio_slice);
    }
    apply_softclip(audio_slice);
    determine_output_level(audio_slice, metrics);

    if (!timeout) {
        metrics.warning = true;
    }
    return TBD_OK;
}


Error AudioConsumer::cleanup() {
    TBD_LOGI(tag, "plugin audio worker shutting down");
    return TBD_OK;
}


}
