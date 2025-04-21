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

#include "tbd/helpers/ctagNumUtil.hpp"
#include <cstdint>
#include <cmath>

namespace CTAG::SP::HELPERS{
    float calcMeanOfFloatArray(const float *farr, const int size) {
        float accum = 0.f;
        for(uint32_t i=0;i<size;i++){
            accum += farr[i];
        }
        return accum / float(size);
    }

    void removeMeanOfFloatArray(float *farr, const int size){
        float accum = 0.f;
        for(uint32_t i=0;i<size;i++){
            accum += farr[i];
        }
        float mean = accum / float(size);
        for(uint32_t i=0;i<size;i++){
            farr[i] -= mean;
        }
    }

    void scaleFloatArrayToAbsMax(float *farr, const int size){
        // find absolute max in array
        float max = fabsf(farr[0]);
        for(uint32_t i=1;i<size;i++){
            if(fabsf(farr[i]) > max) max = fabsf(farr[i]);
        }
        for(uint32_t i=0;i<size;i++){
            farr[i] /= max;
        }
    }

    void accumulateFloatArray(float *farr, const int size){
        float accum = farr[0];
        for(uint32_t i=1;i<size;i++){
            accum += farr[i];
            farr[i] = accum;
        }
    }
}