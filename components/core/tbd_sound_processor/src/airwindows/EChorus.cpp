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

#include <tbd/sound_utils/airwindows/EChorus.hpp>
#include <tbd/helpers/ctagFastMath.hpp>
#include <tbd/heaps.hpp>

namespace heaps = tbd::heaps;


using namespace CTAG::SP::HELPERS;


namespace airwindows {
    void EChorus::Process(float *buf, int bufSz) {

        float speed = powf(A, 3.f) * 0.001f;
        int loopLimit = (int) (totalsamples * 0.499f);
        int count;
        float rangeL = powf(B, 3.f) * loopLimit * 0.12f;
        float rangeR = powf(B, 3.f) * loopLimit * 0.12f * width;
        float wet = C;
        float modulationL = rangeL * wet;
        float modulationR = rangeR * wet;
        float dry = 1.0f - wet;
        const float tupi = 3.141592653589793238f * 2.0f;
        float offset;

        float inputSampleL;
        float inputSampleR;
        float drySampleL;
        float drySampleR;

        for (int i = 0; i < bufSz; i++) {
            inputSampleL = buf[i * 2];
            if (isMonoIn)
                inputSampleR = buf[i * 2];
            else
                inputSampleR = buf[i * 2 + 1];
            if (!isBypass) {
                drySampleL = inputSampleL;
                drySampleR = inputSampleR;

                airFactorL = airPrevL - inputSampleL;
                if (fpFlip) {
                    airEvenL += airFactorL;
                    airOddL -= airFactorL;
                    airFactorL = airEvenL;
                }
                else {
                    airOddL += airFactorL;
                    airEvenL -= airFactorL;
                    airFactorL = airOddL;
                }
                airOddL = (airOddL - ((airOddL - airEvenL) / 256.0f)) / 1.0001f;
                airEvenL = (airEvenL - ((airEvenL - airOddL) / 256.0f)) / 1.0001f;
                airPrevL = inputSampleL;
                inputSampleL += (airFactorL * wet);
                //air, compensates for loss of highs in flanger's interpolation

                airFactorR = airPrevR - inputSampleR;
                if (fpFlip) {
                    airEvenR += airFactorR;
                    airOddR -= airFactorR;
                    airFactorR = airEvenR;
                }
                else {
                    airOddR += airFactorR;
                    airEvenR -= airFactorR;
                    airFactorR = airOddR;
                }
                airOddR = (airOddR - ((airOddR - airEvenR) / 256.0f)) / 1.0001f;
                airEvenR = (airEvenR - ((airEvenR - airOddR) / 256.0f)) / 1.0001f;
                airPrevR = inputSampleR;
                inputSampleR += (airFactorR * wet);
                //air, compensates for loss of highs in flanger's interpolation

                if (gcount < 1 || gcount > loopLimit) { gcount = loopLimit; }
                count = gcount;
                dL[count + loopLimit] = dL[count] = inputSampleL;
                dR[count + loopLimit] = dR[count] = inputSampleR;
                gcount--;
                //float buffer

                inputSampleL = inputSampleR = 0.f;
                for (int j = 0; j < stages; j++) {
                    float mpar = sinf(sweep * static_cast<float>(j + 1));
                    offset = rangeL * static_cast<float>(j + 1) + modulationL * mpar;
                    count = gcount + (int) floorf(offset);
                    inputSampleL += dL[count] * (1 - (offset - floorf(offset))); //less as value moves away from .0
                    inputSampleL += dL[count + 1]; //we can assume always using this in one way or another?
                    inputSampleL += (dL[count + 2] * (offset - floorf(offset))); //greater as value moves away from .0
                    inputSampleL -= (((dL[count] - dL[count + 1]) - (dL[count + 1] - dL[count + 2])) /
                                     50); //interpolation hacks 'r us

                    offset = rangeR * static_cast<float>(j + 1) + modulationR * mpar;
                    count = gcount + (int) floorf(offset);
                    inputSampleR += dR[count] * (1 - (offset - floorf(offset))); //less as value moves away from .0
                    inputSampleR += dR[count + 1]; //we can assume always using this in one way or another?
                    inputSampleR += (dR[count + 2] * (offset - floorf(offset))); //greater as value moves away from .0
                    inputSampleR -= (((dR[count] - dR[count + 1]) - (dR[count + 1] - dR[count + 2])) /
                                     50); //interpolation hacks 'r us
                }


                float fac = static_cast<float>(stages) * 2.f;
                inputSampleL /= fac; //to get a comparable level
                inputSampleR /= fac; //to get a comparable level

                sweep += speed;
                if (sweep > tupi) { sweep -= tupi; }
                //still scrolling through the samples, remember

                if (wet != 1.0) {
                    inputSampleL = (inputSampleL * wet) + (drySampleL * dry);
                    inputSampleR = (inputSampleR * wet) + (drySampleR * dry);
                }
                fpFlip = !fpFlip;
            }
            buf[i * 2] = inputSampleL;
            buf[i * 2 + 1] = inputSampleR;
        }
    }

    EChorus::EChorus() {
        dL = (float *) heaps::malloc(totalsamples * sizeof(float), TBD_HEAPS_SPIRAM);
        dR = (float *) heaps::malloc(totalsamples * sizeof(float), TBD_HEAPS_SPIRAM);
        for (int count = 0; count < totalsamples - 1; count++) {
            dL[count] = 0;
            dR[count] = 0;
        }
        sweep = 3.141592653589793238 / 2.0;
        gcount = 0;
        airPrevL = 0.0;
        airEvenL = 0.0;
        airOddL = 0.0;
        airFactorL = 0.0;
        airPrevR = 0.0;
        airEvenR = 0.0;
        airOddR = 0.0;
        airFactorR = 0.0;
        fpFlip = true;
        fpNShapeL = 0.0;
        fpNShapeR = 0.0;
        width = 0.f;
        isMonoIn = false;
        isBypass = false;
        A = B = C = 0.f;
    }

    EChorus::~EChorus() {
        heaps::free(dL);
        heaps::free(dR);
    }

    void EChorus::SetSpeed(const float &v) {
        A = v;
        //printf("A: %f\n", A);
    }

    void EChorus::SetRange(const float &v) {
        B = v;
        //printf("B: %f\n", B);
    }

    void EChorus::SetWet(const float &v) {
        C = v;
        //printf("C: %f\n", C);
    }

    void EChorus::SetWidth(const float &v) {
        width = v;
    }

    void EChorus::SetMono(bool v) {
        isMonoIn = v;
    }

    void EChorus::SetStages(const int &v) {
        stages = v;
    }

    void EChorus::SetBypass(bool v) {
        isBypass = v;
    }
}

