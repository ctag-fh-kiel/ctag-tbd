#pragma once

#include <tbd/sound_processor.hpp>

#include <tbd/sounds/shared_header.hpp>

namespace tbd::test_sounds {

struct Test1 : sound_processor::MonoSoundProcessor {
    ~Test1() override = default;

    void init() override;
    void process_mono(const sound_processor::StereoSampleView& input,
                      const sound_processor::MonoSampleView& output) override;


    [[tbd(name="Param 1", description="Some Parameter")]]
    int_par p1;
    mint_par p2;

    [[tbd(min=-12, max=22)]]
    mint_par p3;
    mint_par p4;

    struct {
        [[tbd(min=-12, max=22)]]
        uint_par g1_p1;
        uint_par g1_p2;
    } group1;

    struct {
        muint_par g2_p1;
        mfloat_par g2_p2;
        mtrigger_par g2_p3;
    } group2;

    SharedGroupType2 group3;
    SharedGroupType2 group4;
    SharedGroupType2 group5;

    mufloat_par p5;
    mfloat_par p6;
    int_par p7;
};

}
