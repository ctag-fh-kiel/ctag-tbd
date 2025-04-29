#include <atomic>
#include <tbd/sound_processor/parameter_types.h>
#include <tbd/sound_processor.hpp>

namespace tbd::sounds {
    
using namespace tbd::audio::parameters;

[[tbd(name="Void", description="Output input signal unaltered")]]
struct SoundProcessorVoid : audio::SoundProcessor {

    virtual void Process(const audio::ProcessData&) override;
    virtual void Init(std::size_t blockSize, void *blockPtr) override;
    virtual ~SoundProcessorVoid();

protected:

    [[tbd(name="Dummy Control Value", description="This input has no effect")]]
    uint_par dummy;

    [[tbd(name="Dummy Trigger", description="This trigger has no effect")]]
    trigger_par trig_dummy;
};

}