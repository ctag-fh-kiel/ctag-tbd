#pragma once

#include <tbd/sound_processor.hpp>

#include <tbd/sounds/shared_header.hpp>

namespace tbd::test_sounds {

struct Test3 : sound_processor::MonoSoundProcessor {
    ~Test3() override = default;

    void init() override;
    void process_mono(const sound_processor::StereoSampleView& input,
                      const sound_processor::MonoSampleView& output) override;

    int_par p1;
    int_par p2;
    int_par p3;
    int_par p4;

    struct {
        uint_par g1_p1;
        uint_par g1_p2;
    } group1;

    struct {
        uint_par g2_p1;
        float_par g2_p2;
        trigger_par g2_p3;
    } group2;

    SharedGroupType1 group3;
    SharedGroupType2 group4;
    SharedGroupType2 group5;

    ufloat_par p5;
    float_par p6;
    int_par p7;
};

}