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

/*
Zero Delay Feedback Filters
Based on code by Will Pirkle, presented in:
http://www.willpirkle.com/Downloads/AN-4VirtualAnalogFilters.2.0.pdf
http://www.willpirkle.com/Downloads/AN-5Korg35_V3.pdf
http://www.willpirkle.com/Downloads/AN-6DiodeLadderFilter.pdf
http://www.willpirkle.com/Downloads/AN-7Korg35HPF_V2.pdf
and in his book "Designing software synthesizer plug-ins in C++ : for
RackAFX, VST3, and Audio Units"
ZDF using Trapezoidal integrator by Vadim Zavalishin, presented in "The Art
of VA Filter Design" (https://www.native-instruments.com/fileadmin/ni_media/
downloads/pdf/VAFilterDesign_1.1.1.pdf)
Adapted by R. Manzke from Csound diode_ladder C versions by Steven Yi
 https://github.com/csound/csound/blob/develop/Opcodes/wpfilters.c
 https://github.com/csound/csound/blob/develop/Opcodes/wpfilters.h
*/

#pragma once

#include "ctagFilterBase.hpp"

namespace tbd::sound_utils {
    class ctagDiodeLadderFilter : public ctagFilterBase {
    public:
        virtual void SetCutoff(float cutoff) override;

        virtual void SetResonance(float resonance) override;

        virtual void SetSampleRate(float fs) override;

        virtual float Process(float in) override;

        virtual void Init() override;

    private:
        float kval_ = 1.f;
        float a[4] = {0.f};
        float z[4] = {0.f};
        float G[4] = {0.f};
        float beta[4] = {0.f};
        float delta[3] = {0.f};
        float epsilon[3] = {0.f};
        float gamma[3] = {0.f};
        float SG[4] = {0.f};
        float SIGMA = 0.f, GAMMA = 0.f;
        float alpha = 0.f;
        float g, gp1;
    };
}



