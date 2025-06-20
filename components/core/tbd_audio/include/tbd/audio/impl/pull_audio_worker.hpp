#include <tbd/audio/module.hpp>

#include <tbd/audio_device/audio_settings.hpp>
#include <tbd/audio_device.hpp>
#include <tbd/audio_device/concepts/audio_consumer_type.hpp>


namespace tbd::audio {

struct PullAudioParams {
    // no params
};

/** @brief module task implementation for pulling audio task
 *
 */
template<AudioConsumerType AudioConsumerT>
struct PullAudioFeeder {
    using params_type = PullAudioParams;

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
    
        _consumer.consume(fbuf);

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
    AudioConsumerT _consumer;
};

/** @brief audio worker pulling samples from codec
 *
 */
template<AudioConsumerType AudioConsumerT, system::CpuCore cpu_core>
using PullAudioWorker = system::TaskModule<PullAudioFeeder<AudioConsumerT>, cpu_core>;

}
