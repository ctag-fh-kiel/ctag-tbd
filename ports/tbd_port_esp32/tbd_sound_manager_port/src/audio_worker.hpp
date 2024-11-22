#include <tbd/sound_manager/common/module.hpp>

#include <tbd/drivers/codec.hpp>
#include <tbd/system/cpu_cores.hpp>
#include <tbd/sound_manager/common/audio_consumer_type.hpp>


namespace tbd::audio {

template<AudioConsumerType AudioConsumerT>
struct AudioFeeder {

    void set_output_levels(float left_level, float right_level) {
        drivers::Codec::SetOutputLevels(left_level, right_level);
    }

    /** @brief settle input by short high pass filtering
     * 
     */
    void let_signal_settle() {
        drivers::Codec::RecalibDCOffset();
    }


    uint32_t do_begin() {
        drivers::Codec::InitCodec();
        return 0;
    }

    uint32_t do_work() {
        // get normalized raw data from CODEC
        drivers::Codec::ReadBuffer(fbuf, BUF_SZ);
    
        _consumer.consume(fbuf);

        // write raw float data back to CODEC
        drivers::Codec::WriteBuffer(fbuf, BUF_SZ);
        return 0;
    }

    uint32_t do_cleanup() {
        drivers::Codec::SetOutputLevels(0, 0);
        return 0;
    }

private:
    float fbuf[BUF_SZ * 2];
    AudioConsumerT _consumer;
};

template<AudioConsumerType AudioConsumerT, system::CpuCore cpu_core>
using AudioWorker = system::ModuleTask<AudioFeeder<AudioConsumerT>, cpu_core>;

}
