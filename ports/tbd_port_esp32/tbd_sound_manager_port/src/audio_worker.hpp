#include <tbd/sound_manager/common/module.hpp>

#include <tbd/system/cpu_cores.hpp>
#include <tbd/sound_manager/common/audio_consumer_type.hpp>


namespace tbd::audio {

template<AudioConsumerType AudioConsumerT, class CodecT>
struct AudioFeeder {

    uint32_t do_begin() {
        return 0;
    }

    uint32_t do_work() {
        // get normalized raw data from CODEC
        CodecT::ReadBuffer(fbuf, BUF_SZ);
    
        _consumer.consume(fbuf);

        // write raw float data back to CODEC
        CodecT::WriteBuffer(fbuf, BUF_SZ);
        return 0;
    }

    uint32_t do_cleanup() {
        return 0;
    }

private:
    float fbuf[BUF_SZ * 2];
    AudioConsumerT _consumer;
};


template<AudioConsumerType AudioConsumerT, 
         class CodecT, 
         system::CpuCore cpu_core>
using AudioWorker = system::ModuleTask<
    AudioFeeder<
        AudioConsumerT,
        CodecT>, 
    cpu_core>;
}
