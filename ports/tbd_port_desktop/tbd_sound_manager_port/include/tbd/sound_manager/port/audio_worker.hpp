#include <tbd/sound_manager/common/module.hpp>

#include <algorithm>
#include <RtAudio.h>

#include <tbd/system/cpu_cores.hpp>
#include <tbd/sound_manager/common/audio_consumer_type.hpp>
#include <tbd/portutils/file_audio_source.hpp>
#include <tbd/sound_manager/port/sound_params.hpp>


namespace tbd::audio {

template<AudioConsumerType AudioConsumerT>
struct AudioFeeder {
    AudioFeeder(const char* name) : _name(name) {}

    void set_output_levels(float left_level, float right_level) {

    }

    void let_signal_settle() {
    
    }

    uint32_t init(SoundParams&& sound_params) {
        _params = sound_params;
        return 0;
    }

    uint32_t begin(bool wait = false) {
        RtAudio::DeviceInfo info;
        info = _audio.getDeviceInfo(_params.device());
        if (!info.probed) {
            TBD_LOGE(tag, "sound card probing failed");
            return 1;
        }
    
        // Print, for example, the maximum number of output channels for each device
        TBD_LOGI(tag, "device %i %s", _params.device(), info.name.c_str());

        if (_params.use_live_input() && info.duplexChannels < 2) {
            TBD_LOGE(tag, "No duplex device found, enabling output only!");
            return 1;
        }

        auto sample_rates = info.sampleRates;
        if (std::count(sample_rates.begin(), sample_rates.end(), TBD_SAMPLE_RATE) == 0
            || (info.nativeFormats & RTAUDIO_FLOAT32) == 0)
        {

            TBD_LOGE(tag, "Sample rate of 44100Hz@float32 not supported!");
            return 1;
        }

        _consumer.startup();

        input_params.deviceId = _params.device();
        input_params.nChannels = 2;
        output_params.deviceId = _params.device();
        output_params.nChannels = 2;

        try {
            if (!_params.use_live_input()) {
                _file_input.open(_params.input_file());
                _audio.openStream(&output_params, nullptr, RTAUDIO_FLOAT32, TBD_SAMPLE_RATE, &samples_per_channel, &processing_main, this);
            } else {
                _audio.openStream(&output_params, &input_params, RTAUDIO_FLOAT32, TBD_SAMPLE_RATE, &samples_per_channel, &processing_main, this);
            }
        }
        catch (RtAudioError &e) {
            e.printMessage();
            return 1;
        }
        try {
            _audio.startStream();
        }
        catch (RtAudioError &e) {
            e.printMessage();
            return 1;
        }

        return 0;
    }

    uint32_t end(bool wait = false) {
        if (_audio.isStreamRunning() && _audio.isStreamOpen()) {
            _audio.stopStream();
            _audio.closeStream();
        }
        return 0;
    }

    void pause_processing() {
        _audio.stopStream();
    }

    void resume_processing(bool wait = false) {
        _audio.startStream();
    }

private:
    static int processing_main(
        void *output_buffer, void *input_buffer, unsigned int num_frames,
        double stream_time, RtAudioStreamStatus status, void* _self
    ) {
        if (_self == nullptr) {
            return 1;
        }
        auto& self = *reinterpret_cast<AudioFeeder*>(_self);

        if (num_frames != TBD_SAMPLES_PER_CHUNK) {
            // FIXME: maybe something else
            return 1;
        }

        // FIXME: should we clamp or reset these values?
        // reset out of range sound values
        auto fbuf = reinterpret_cast<float*>(output_buffer);

        if (!self._params.use_live_input()) {
            if (!self._file_input.read_chunk(fbuf, TBD_SAMPLES_PER_CHUNK)) {
                return 1;
            }
        } else {
            memcpy(fbuf, input_buffer, TBD_SAMPLES_PER_CHUNK * 2 * 4);
        }

        for(int i=0;i<32;i++){
            if(fbuf[i*2] > 1.f)fbuf[i*2] = 0.f;
            if(fbuf[i*2] < -1.f)fbuf[i*2] = -0.f;
            if(fbuf[i*2 + 1] > 1.f)fbuf[i*2 + 1] = 0.f;
            if(fbuf[i*2 + 1] < -1.f)fbuf[i*2 + 1] = -0.f;
        }

        // FIXME: the simulator does more at this point... do we need a custom buffer?

        self._consumer.consume(reinterpret_cast<float*>(output_buffer));
        
        // FIXME: looked like this
        // memcpy(outputBuffer, fbuf, BUF_SZ * 2 * 4);
        return 0;
    }

    SoundParams _params;
    const char* _name;
    RtAudio _audio;
    common::FileAudioSource _file_input;

    // TODO: look into the mechanics of rtaudio: why can't this be const
    uint samples_per_channel = TBD_SAMPLES_PER_CHUNK;
    RtAudio::StreamParameters input_params;
    RtAudio::StreamParameters output_params;
    float fbuf[TBD_SAMPLES_PER_CHUNK * 2];
    AudioConsumerT _consumer;
};


template<AudioConsumerType AudioConsumerT, system::CpuCore cpu_core>
using AudioWorker = AudioFeeder<AudioConsumerT>;

}
