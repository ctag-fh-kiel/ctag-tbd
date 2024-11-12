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

// https://www.musicdsp.org/en/latest/Synthesis/10-fast-sine-and-cosine-calculation.html

namespace CTAG::SP::HELPERS {
    class ctagSineSource {
    public:
        ctagSineSource();

        void SetSampleRate(float f_hz);

        void SetFrequency(float f_hz);

        void SetFrequencyPhase(float f_hz, float phase_rad);

        float Process();

        float GetCos();

        float GetSin();

    protected:
        float a, s[2];
        float fSample;
    };
}
