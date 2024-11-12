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

/* based on ensemble chorus by Chris Johnson, see LICENSE file */

#pragma once

namespace airwindows {
    class EChorus {
    public:
        void Process(float *, int);

        void SetSpeed(const float &); // 0 ... 1
        void SetRange(const float &); // 0 ... 1
        void SetWet(const float &); // 0 ... 1
        void SetWidth(const float &); // 0 ... 0.5
        void SetMono(bool);

        void SetBypass(bool);

        void SetStages(const int &); // 1 ... 4
        EChorus();

        ~EChorus();

    private:
        const static int totalsamples = 16386;
        float *dL;
        float *dR;
        float sweep;
        int gcount;
        int stages;
        float airPrevL;
        float airEvenL;
        float airOddL;
        float airFactorL;
        float airPrevR;
        float airEvenR;
        float airOddR;
        float airFactorR;
        bool fpFlip;
        float fpNShapeL;
        float fpNShapeR;
        float width;
        float A, B, C;
        bool isMonoIn, isBypass;
    };
}
