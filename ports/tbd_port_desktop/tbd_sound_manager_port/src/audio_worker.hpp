#include <tbd/sound_manager/common/module.hpp>

#include <algorithm>
#include <RtAudio.h>

#include <tbd/system/cpu_cores.hpp>
#include <tbd/sound_manager/common/audio_consumer_type.hpp>


namespace {
    const char* tag = "tbd_sound_manager_port";
}


namespace tbd::audio {

template<AudioConsumerType AudioConsumerT>
struct AudioFeeder {
    AudioFeeder(const char* name) : _name(name) {}

    void set_output_levels(float left_level, float right_level) {

    }

    void let_signal_settle() {
    
    }

    uint32_t begin(bool wait = false) {
        RtAudio::DeviceInfo info;
        info = _audio.getDeviceInfo(sound_card_id);
        if (!info.probed) {
            TBD_LOGE(tag, "sound card probing failed");
            return 1;
        }
    
        // Print, for example, the maximum number of output channels for each device
        TBD_LOGI(tag, "device %i %s", sound_card_id, info.name.c_str());

        if (info.duplexChannels < 2) {
            TBD_LOGI(tag, "No duplex device found, enabling output only!");
            output_only = true;
        }

        auto sample_rates = info.sampleRates;
        if (std::count(sample_rates.begin(), sample_rates.end(), 44100) == 0
            || (info.nativeFormats & RTAUDIO_FLOAT32) == 0)
        {

            TBD_LOGE(tag, "Sample rate of 44100Hz@float32 not supported!");
            return 1;
        }

        _consumer.startup();

        try {
            if (output_only) {
                _audio.openStream(&output_params, nullptr, RTAUDIO_FLOAT32, 44100, &samples_per_channel, &processing_main, this);
            } else {
                _audio.openStream(&output_params, &input_params, RTAUDIO_FLOAT32, 44100, &samples_per_channel, &processing_main, this);
            }
            // configure channels
            // model = std::make_unique<SPManagerDataModel>();
            // SetSoundProcessorChannel(0, model->GetActiveProcessorID(0));
            // SetSoundProcessorChannel(1, model->GetActiveProcessorID(1));
        }
        catch (RtAudioError &e) {
            e.printMessage();
            exit(0);
        }
        try {
            _audio.startStream();
        }
        catch (RtAudioError &e) {
            e.printMessage();
        }

        return 0;
    }

    // uint32_t do_work() {
    //     // get normalized raw data from CODEC
    //     CodecT::ReadBuffer(fbuf, BUF_SZ);
    
    //     _consumer.consume(fbuf);

    //     // write raw float data back to CODEC
    //     CodecT::WriteBuffer(fbuf, BUF_SZ);
    //     return 0;
    // }

    uint32_t end(bool wait = false) {
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
            return 0;
        }
        auto self = reinterpret_cast<AudioFeeder*>(_self);

        if (num_frames != BUF_SZ) {
            // FIXME: maybe something else
            return 0;
        }

        // FIXME: the simulator does more at this point... do we need a custom buffer?

        self->_consumer.consume(reinterpret_cast<float*>(input_buffer));
        
        // FIXME: looked like this
        // memcpy(outputBuffer, fbuf, BUF_SZ * 2 * 4);
        memcpy(output_buffer, input_buffer, BUF_SZ * 2 * 4);
        return 0;
    }

    const char* _name;
    RtAudio _audio;
    uint samples_per_channel = BUF_SZ;
    int sound_card_id;
    bool output_only;
    RtAudio::StreamParameters input_params;
    RtAudio::StreamParameters output_params;
    float fbuf[BUF_SZ * 2];
    AudioConsumerT _consumer;
};


template<AudioConsumerType AudioConsumerT, system::CpuCore cpu_core>
using AudioWorker = AudioFeeder<AudioConsumerT>;

}
