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
#include "ctagSoundProcessor.hpp"
#include "freeverb3/strev.hpp"
#include "freeverb3/progenitor.hpp"

namespace CTAG::SP {
    class ctagSoundProcessorGDVerb : public ctagSoundProcessor {
    public:
        void Process(const ProcessData &) override;

        ctagSoundProcessorGDVerb();

    private:

        virtual void knowYourself() override;

        // stereo reverb object
        fv3::strev_f strev;

        // sectionHpp
        atomic<int32_t> revtime, cv_revtime;
        atomic<int32_t> dccut, cv_dccut;
        atomic<int32_t> idiffusion1, cv_idiffusion1;
        atomic<int32_t> idiffusion2, cv_idiffusion2;
        atomic<int32_t> diffusion1, cv_diffusion1;
        atomic<int32_t> diffusion2, cv_diffusion2;
        atomic<int32_t> inputdamp, cv_inputdamp;
        atomic<int32_t> damp, cv_damp;
        atomic<int32_t> outputdamp, cv_outputdamp;
        atomic<int32_t> spin, cv_spin;
        atomic<int32_t> spindiff, cv_spindiff;
        atomic<int32_t> spinlimit, cv_spinlimit;
        atomic<int32_t> wander, cv_wander;
        atomic<int32_t> modnoise1, cv_modnoise1;
        atomic<int32_t> modnoise2, cv_modnoise2;
        atomic<int32_t> autodiff, trig_autodiff;
        atomic<int32_t> dry, cv_dry;
        atomic<int32_t> wet, cv_wet;
        atomic<int32_t> width, cv_width;
        atomic<int32_t> mono, trig_mono;
        // sectionHpp
    };
}

