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

#include <cmath>
#include <cstdint>
#include "ctagWNoiseGen.hpp"

#ifndef TBD_SIM
#include "xtensa/core-macros.h"
#endif

// after supercollider dust

namespace CTAG::SP::HELPERS {
    class ctagDustGen {
    public:
        ctagDustGen() :
#ifndef TBD_SIM
                wnoise(XTHAL_GET_CCOUNT()) // seed noise generator with system tick count
#else
                wnoise(42)
#endif
        {
            wnoise.SetBipolar(false);
            isBipolar = false;
            smooth = 0.f;
            zLast = 0.f;
            fSample = 44100.f;
            wCnt = 0;
        }

        void SetParams(float rate, float mul, float add, float fs) {
            fSample = fs;
            m_density = rate;
            m_thresh = m_density / fSample;
            m_scale = m_thresh > 0.f ? 1.f / m_thresh : 0.f;
        }

        void SetRate(float rate) {
            m_density = rate;
            m_thresh = m_density / fSample;
            m_scale = m_thresh > 0.f ? 1.f / m_thresh : 0.f;
        }

        void SetSmooth(const float s) {
            if (s > 1.f)smooth = 1.f;
            else if (s < 0.f)smooth = 0.f;
            else smooth = s;
        }

        void SetWidth(uint32_t w) {
            if (w > 0.05f * fSample) w = (uint32_t) 0.05 * fSample;
            width = w;
        }

        void SetBipolar(bool yes) {
            wnoise.SetBipolar(yes);
        }

        float Process() {
            if (wCnt != 0) {
                //TBD_LOGE("Dust", "wcnt %d", wCnt);
                wCnt--;
                if (wCnt > width / 2)
                    return zLast;
                else
                    return -zLast;
            }

            float z = wnoise.Process();
            float val;
            if (fabs(z) < m_thresh)
                val = z * m_scale;
            else
                val = 0.f;
            if (val != 0.f) {
                wCnt = width;
            }
            val = (1.f - smooth) * val + smooth * zLast;
            zLast = val;
            return val;
        }

    private:
        ctagWNoiseGen wnoise;
        float m_density, m_thresh, m_scale;
        float fSample;
        float zLast, smooth;
        bool isBipolar;
        uint32_t width, wCnt;
    };
}

