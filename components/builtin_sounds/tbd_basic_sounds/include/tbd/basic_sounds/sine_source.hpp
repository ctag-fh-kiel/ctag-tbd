#pragma once

#include <tbd/sound_processor.hpp>


namespace tbd::basic_sounds {

struct SineSource : sound_processor::MonoSoundProcessor {
    ~SineSource() override = default;

    void init() override;
    void process(const sound_processor::ProcessData&data) override;
};


}
