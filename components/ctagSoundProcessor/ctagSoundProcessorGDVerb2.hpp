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
    class ctagSoundProcessorGDVerb2 : public ctagSoundProcessor {
    public:
        void Process(const ProcessData &) override;

       virtual void Init() override;

    private:

        virtual void knowYourself() override;

        // stereo reverb object
        fv3::progenitor_f prog_rev;

        float prefSpinToWander = 0.f;
        float prefRevTime = 1.f;

        // sectionHpp
	atomic<int32_t> exprtdecay, trig_exprtdecay;
	atomic<int32_t> revtime, cv_revtime;
	atomic<int32_t> decay0, cv_decay0;
	atomic<int32_t> decay1, cv_decay1;
	atomic<int32_t> decay2, cv_decay2;
	atomic<int32_t> decay3, cv_decay3;
	atomic<int32_t> decayf, cv_decayf;
	atomic<int32_t> exprtdiffu, trig_exprtdiffu;
	atomic<int32_t> diffusion1, cv_diffusion1;
	atomic<int32_t> diffusion2, cv_diffusion2;
	atomic<int32_t> diffusion3, cv_diffusion3;
	atomic<int32_t> diffusion4, cv_diffusion4;
	atomic<int32_t> exprtdamp, trig_exprtdamp;
	atomic<int32_t> dccut, cv_dccut;
	atomic<int32_t> inputdamp, cv_inputdamp;
	atomic<int32_t> damp, cv_damp;
	atomic<int32_t> outputdamp, cv_outputdamp;
	atomic<int32_t> outputdampbw, cv_outputdampbw;
	atomic<int32_t> bassboost, cv_bassboost;
	atomic<int32_t> damp2, cv_damp2;
	atomic<int32_t> bassbw, cv_bassbw;
	atomic<int32_t> exprtmod, trig_exprtmod;
	atomic<int32_t> spin, cv_spin;
	atomic<int32_t> spinlimit, cv_spinlimit;
	atomic<int32_t> wander, cv_wander;
	atomic<int32_t> spin2, cv_spin2;
	atomic<int32_t> spinlimit2, cv_spinlimit2;
	atomic<int32_t> wander2, cv_wander2;
	atomic<int32_t> spin2wander, cv_spin2wander;
	atomic<int32_t> dry, cv_dry;
	atomic<int32_t> wet, cv_wet;
	atomic<int32_t> width, cv_width;
	atomic<int32_t> mono, trig_mono;
	// sectionHpp
    };
}

