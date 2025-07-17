#pragma once

#include <tbd/sound_processor.hpp>

#include <tbd/sounds/shared_header.hpp>

namespace tbd::test_sounds {

struct SineGen {
    void process_mono(const sound_processor::StereoSampleView& input,
                      const sound_processor::MonoSampleView& output);

    ufloat_par freq_hz;
};

struct LowPass {
    void process_mono(const sound_processor::StereoSampleView& input,
                      const sound_processor::MonoSampleView& output);

    ufloat_par freq_hz;
};

struct Test2 : sound_processor::MonoSoundProcessor {
    ~Test2() override = default;

    void init() override;
    void process_mono(const sound_processor::StereoSampleView& input,
                      const sound_processor::MonoSampleView& output) override;

    int_par p1;
    int_par p2;

    SineGen gen1;
    SineGen gen2;
    LowPass low_pass;
};

}
