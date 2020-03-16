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

namespace CTAG::SP{
    class ctagSoundProcessorGDVerb : public ctagSoundProcessor{
    public:
        void Process(const ProcessData &);
        ~ctagSoundProcessorGDVerb();
        ctagSoundProcessorGDVerb();
        const char * GetCStrID() const;
    private:
        void setParamValueInternal(const string id, const string key, const int val) override;
        void loadPresetInternal() override;
        const string id = "gdverb";
        // stereo reverb object
        fv3::strev_f strev;
        //fv3::progenitor_f strev;
        // inter thread variables are atomic
        atomic<int32_t> par_revtime;
        atomic<int32_t> par_dccut;
        atomic<int32_t> par_idiffusion1;
        atomic<int32_t> par_idiffusion2;
        atomic<int32_t> par_diffusion1;
        atomic<int32_t> par_diffusion2;
        atomic<int32_t> par_inputdamp;
        atomic<int32_t> par_damp;
        atomic<int32_t> par_outputdamp;
        atomic<int32_t> par_spin;
        atomic<int32_t> par_spindiff;
        atomic<int32_t> par_spinlimit;
        atomic<int32_t> par_wander;
        atomic<int32_t> par_modnoise1;
        atomic<int32_t> par_modnoise2;
        atomic<int32_t> par_autodiff;
        atomic<int32_t> par_wet;
        atomic<int32_t> par_dry;
        atomic<int32_t> par_width;
        atomic<int32_t> par_mono;
        atomic<int32_t> cv_par_revtime;
        atomic<int32_t> cv_par_dccut;
        atomic<int32_t> cv_par_idiffusion1;
        atomic<int32_t> cv_par_idiffusion2;
        atomic<int32_t> cv_par_diffusion1;
        atomic<int32_t> cv_par_diffusion2;
        atomic<int32_t> cv_par_inputdamp;
        atomic<int32_t> cv_par_damp;
        atomic<int32_t> cv_par_outputdamp;
        atomic<int32_t> cv_par_spin;
        atomic<int32_t> cv_par_spindiff;
        atomic<int32_t> cv_par_spinlimit;
        atomic<int32_t> cv_par_wander;
        atomic<int32_t> cv_par_modnoise1;
        atomic<int32_t> cv_par_modnoise2;
        atomic<int32_t> cv_par_autodiff;
        atomic<int32_t> cv_par_wet;
        atomic<int32_t> cv_par_dry;
        atomic<int32_t> cv_par_width;
        atomic<int32_t> updateParams;

    };
}

