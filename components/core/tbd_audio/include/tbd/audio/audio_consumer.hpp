#pragma once

#include <tbd/audio_device/audio_settings.hpp>
#include <tbd/audio/channels.hpp>
#include <tbd/sound_processor.hpp>
#include <tbd/system/locks.hpp>
#include <tbd/errors.hpp>

#include <freeverb3/efilter.hpp>
#include <stmlib/dsp/dsp.h>
#include <atomic>


namespace tbd::audio {
enum NoiseGateConfig {
    NG_NONE  = 0,
    NG_LEFT  = 1,
    NG_RIGHT = 2,
    NG_BOTH  = 3,
};

struct AudioProcessingParams {
    std::atomic<uint32_t> noiseGateCfg;
    std::atomic<uint32_t> ch01Daisy;
    std::atomic<uint32_t> toStereoCH0;
    std::atomic<uint32_t> toStereoCH1;

    std::atomic<uint32_t> ch0_outputSoftClip;
    std::atomic<uint32_t> ch1_outputSoftClip;
};

struct AudioMetrics {
    bool warning = false;
    float input_level = 0.f;
    float output_level = 0.f;
};

struct AudioConsumer {
    sound_processor::SoundProcessor* get_sound_processor(channels::ChannelID channel);
    Error set_sound_processor(channels::Channels, sound_processor::SoundProcessor* sound_processor);
    Error reset_sound_processor(channels::Channels);


    Error startup();
    Error consume(float* audio_slice, AudioMetrics& metrics);
    Error cleanup();

protected:
    void apply_bandwidth_filter(float* audio_slice);
    void determine_input_level(AudioMetrics& metrics);
    void apply_noise_gate(float* audio_slice);
    channels::Channels run_processors(float* audio_slice);
    void apply_output_mapping(float* audio_slice);
    void apply_softclip(float* audio_slice);
    void determine_output_level(float* audio_slice, AudioMetrics& metrics);

    AudioProcessingParams params;

    sound_processor::SoundProcessor* left = nullptr;
    sound_processor::SoundProcessor* right = nullptr;
    system::Lock sound_processing_lock;

    bool noise_gate_is_open = true;

    fv3::dccut_f in_dccutl, in_dccutr;
    float peakIn = 0.f, peakOut = 0.f;
    float peakL = 0.f, peakR = 0.f;
    float lramp[TBD_SAMPLES_PER_CHUNK];
    // bool isStereoCH0 = false;
    sound_processor::ProcessData pd;
};

}
