#pragma once

#include <tbd/sound_processor.hpp>


namespace tbd::basic_sounds {

struct SineSource : sound_processor::MonoSoundProcessor {
    ~SineSource() override = default;

    void init() override;
    void process_mono(const sound_processor::StereoSampleView& input,
                      const sound_processor::MonoSampleView& output) override;
};


}
