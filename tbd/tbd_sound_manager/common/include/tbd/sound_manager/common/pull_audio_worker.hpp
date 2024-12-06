#include <tbd/sound_manager/common/module.hpp>

#include <tbd/drivers/codec.hpp>
#include <tbd/common/audio.hpp>
#include <tbd/sound_manager/common/audio_consumer_type.hpp>


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
        drivers::Codec::SetOutputLevels(left_level, right_level);
    }

    /** @brief settle input by short high pass filtering
     * 
     */
    void let_signal_settle() {
        drivers::Codec::RecalibDCOffset();
    }

    uint32_t init(PullAudioParams&& sound_params) {
        return 0;
    }

    uint32_t do_begin() {
        drivers::Codec::init();
        return 0;
    }

    uint32_t do_work() {
        // get normalized raw data from CODEC
        drivers::Codec::ReadBuffer(fbuf, TBD_SAMPLES_PER_CHUNK);
    
        _consumer.consume(fbuf);

        // write raw float data back to CODEC
        drivers::Codec::WriteBuffer(fbuf, TBD_SAMPLES_PER_CHUNK);
        return 0;
    }

    uint32_t do_cleanup() {
        drivers::Codec::SetOutputLevels(0, 0);
        drivers::Codec::deinit();
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