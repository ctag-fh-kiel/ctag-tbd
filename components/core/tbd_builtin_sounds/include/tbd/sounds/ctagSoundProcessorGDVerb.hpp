/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/


#pragma once

#include <atomic>
#include <cstdint>
#include <tbd/sound_processor.hpp>
#include "freeverb3/strev.hpp"
#include "freeverb3/progenitor.hpp"

namespace tbd::sounds {

struct SoundProcessorGDVerb : audio::SoundProcessor {

    void Process(const audio::ProcessData&) override;

    virtual void Init(std::size_t blockSize, void *blockPtr) override;

protected:

    // stereo reverb object
    fv3::strev_f strev;

    float prefRevTime = 1.f;

    // sectionHpp
    std::atomic<int32_t> revtime, cv_revtime;
    std::atomic<int32_t> dccut, cv_dccut;
    std::atomic<int32_t> idiffusion1, cv_idiffusion1;
    std::atomic<int32_t> idiffusion2, cv_idiffusion2;
    std::atomic<int32_t> diffusion1, cv_diffusion1;
    std::atomic<int32_t> diffusion2, cv_diffusion2;
    std::atomic<int32_t> inputdamp, cv_inputdamp;
    std::atomic<int32_t> damp, cv_damp;
    std::atomic<int32_t> outputdamp, cv_outputdamp;
    std::atomic<int32_t> spin, cv_spin;
    std::atomic<int32_t> spindiff, cv_spindiff;
    std::atomic<int32_t> spinlimit, cv_spinlimit;
    std::atomic<int32_t> wander, cv_wander;
    std::atomic<int32_t> modnoise1, cv_modnoise1;
    std::atomic<int32_t> modnoise2, cv_modnoise2;
    std::atomic<int32_t> autodiff, trig_autodiff;
    std::atomic<int32_t> dry, cv_dry;
    std::atomic<int32_t> wet, cv_wet;
    std::atomic<int32_t> width, cv_width;
    std::atomic<int32_t> mono, trig_mono;
    // sectionHpp
};

}

