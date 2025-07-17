#include <tbd/audio/module.hpp>

#include <tbd/audio_device/audio_settings.hpp>
#include <tbd/audio_device.hpp>
#include <tbd/sound_processor/channels.hpp>
#include <tbd/sound_processor.hpp>
#include <tbd/errors.hpp>
#include <tbd/system/task_module.hpp>

#include <tbd/audio/audio_consumer.hpp>

namespace tbd::system {
enum class CpuCore : uint8_t;
}

namespace tbd::audio {

struct PullAudioParams {
    // no params
};

/** @brief module task implementation for pulling audio task
 *
 */
struct PullAudioFeeder {
    using params_type = PullAudioParams;

    sound_processor::SoundProcessor* get_sound_processor(const sound_processor::channels::ChannelID channel) {
        return _consumer.get_sound_processor(channel);
    }

    Error set_sound_processor(const sound_processor::channels::Channels channels, sound_processor::SoundProcessor* sound_processor) {
        return _consumer.set_sound_processor(channels, sound_processor);
    }

    Error reset_sound_processor(const sound_processor::channels::Channels channels) {
        return _consumer.reset_sound_processor(channels);
    }

    void set_output_levels(float left_level, float right_level) {
        AudioDevice::set_output_levels(left_level, right_level);
    }

    /** @brief settle input by short high pass filtering
     * 
     */
    void let_signal_settle() {
        AudioDevice::recalib_dc_offset();
    }

    uint32_t init(const PullAudioParams& sound_params) {
        return 0;
    }

    uint32_t do_begin() {
        AudioDevice::init();
        return 0;
    }

    uint32_t do_work() {
        // get normalized raw data from CODEC
        AudioDevice::read_buffer(fbuf, TBD_SAMPLES_PER_CHUNK);

        AudioMetrics metrics;
        _consumer.consume(fbuf, metrics);

        // write raw float data back to CODEC
        AudioDevice::write_buffer(fbuf, TBD_SAMPLES_PER_CHUNK);
        return 0;
    }

    uint32_t do_cleanup() {
        AudioDevice::set_output_levels(0, 0);
        AudioDevice::deinit();
        return 0;
    }

private:
    float fbuf[TBD_CHUNK_BUFFER_LENGTH];
    AudioConsumer _consumer;
};

/** @brief audio worker pulling samples from codec
 *
 */
template<system::CpuCore cpu_core>
using PullAudioWorker = system::TaskModule<PullAudioFeeder, cpu_core>;

}
