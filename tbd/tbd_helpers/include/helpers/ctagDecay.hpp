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


//
// Created by Robert Manzke on 06.03.20.
//

#pragma once

namespace CTAG::SP::HELPERS {
    class ctagDecay {
    public:
        ctagDecay();

        void SetDecay60dB(float val);

        void SetSampleRate(float fs);

        float Process(float in);

        void SetCoeff(float c);

        void SetCoeffSmoothing(bool yes);

        void SetCoeffSmoothingLevel(float lev);

        float GetCoeff();

    private:
        float _c;
        float out;
        float _fs;
        float pre_c;
        bool smooth_c;
        float smooth_level;
    };
}


