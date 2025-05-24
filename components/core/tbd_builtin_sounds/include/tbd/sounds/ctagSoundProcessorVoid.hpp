#include <atomic>
#include <tbd/parameter_types.h>
#include <tbd/sound_processor.hpp>

namespace tbd::sounds {
    


[[tbd(name="Void", description="Output input signal unaltered")]]
struct SoundProcessorVoid : sound_processor::SoundProcessor {

    virtual void Process(const sound_processor::ProcessData&) override;
    virtual void Init(std::size_t blockSize, void *blockPtr) override;
    virtual ~SoundProcessorVoid();

protected:

    [[tbd(name="Dummy Control Value", description="This input has no effect")]]
    uint_par dummy;

    [[tbd(name="Dummy Trigger", description="This trigger has no effect")]]
    trigger_par trig_dummy;
};

}