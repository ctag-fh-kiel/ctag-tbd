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

/* based on EveryTrim by Chris Johnson, see LICENSE file */
/* ported to CTAG TBD by Christian Feyer */

#include <tbd/sound_utils/airwindows/EveryTrim.hpp>
#include <tbd/helpers/ctagFastMath.hpp>

using namespace CTAG::SP::HELPERS;


namespace airwindows {
    void EveryTrim::Process(float *buf, int bufSz) {

        float leftgain = powf(10.0f,((A*24.0f)-12.0f)/20.0f);
        float rightgain = powf(10.0f,((B*24.0f)-12.0f)/20.0f);
        float midgain = powf(10.0f,((C*24.0f)-12.0f)/20.0f);
        float sidegain = powf(10.0f,((D*24.0f)-12.0f)/20.0f);
        float mastergain = powf(10.0f,((E*24.0f)-12.0f)/20.0f) * 0.5f;

        float inputSampleL;
        float inputSampleR;
        float mid;
        float side;

        leftgain *= mastergain;
        rightgain *= mastergain;

        for(int i=0;i<bufSz;i++)
        {
            inputSampleL = buf[i*2];   //Channel 0
            inputSampleR = buf[i*2 + 1];   //Channel 1
            if (inputSampleL<1.2e-38f && -inputSampleL<1.2e-38f) {
                static int noisesource = 0;
                //this declares a variable before anything else is compiled. It won't keep assigning
                //it to 0 for every sample, it's as if the declaration doesn't exist in this context,
                //but it lets me add this denormalization fix in a single place rather than updating
                //it in three different locations. The variable isn't thread-safe but this is only
                //a random seed and we can share it with whatever.
                noisesource = noisesource % 1700021; noisesource++;
                int residue = noisesource * noisesource;
                residue = residue % 170003; residue *= residue;
                residue = residue % 17011; residue *= residue;
                residue = residue % 1709; residue *= residue;
                residue = residue % 173; residue *= residue;
                residue = residue % 17;
                float applyresidue = residue;
                applyresidue *= 0.00000001f;
                applyresidue *= 0.00000001f;
                inputSampleL = applyresidue;
            }
            if (inputSampleR<1.2e-38f && -inputSampleR<1.2e-38f) {
                static int noisesource = 0;
                noisesource = noisesource % 1700021; noisesource++;
                int residue = noisesource * noisesource;
                residue = residue % 170003; residue *= residue;
                residue = residue % 17011; residue *= residue;
                residue = residue % 1709; residue *= residue;
                residue = residue % 173; residue *= residue;
                residue = residue % 17;
                float applyresidue = residue;
                applyresidue *= 0.00000001f;
                applyresidue *= 0.00000001f;
                inputSampleR = applyresidue;
                //this denormalization routine produces a white noise at -300 dB which the noise
                //shaping will interact with to produce a bipolar output, but the noise is actually
                //all positive. That should stop any variables from going denormal, and the routine
                //only kicks in if digital black is input. As a final touch, if you save to 24-bit
                //the silence will return to being digital black again.
            }

            mid = inputSampleL + inputSampleR;
            side = inputSampleL - inputSampleR;
            mid *= midgain;
            side *= sidegain;
            inputSampleL = (mid+side) * leftgain;
            inputSampleR = (mid-side) * rightgain;
            //contains mastergain and the gain trim fixing the mid/side
/*
            //stereo 32 bit dither, made small and tidy.
            int expon; frexpf((float)inputSampleL, &expon);
            float dither = (rand()/(RAND_MAX*7.737125245533627e+25))*pow(2,expon+62);
            inputSampleL += (dither-fpNShapeL); fpNShapeL = dither;
            frexpf((float)inputSampleR, &expon);
            dither = (rand()/(RAND_MAX*7.737125245533627e+25))*pow(2,expon+62);
            inputSampleR += (dither-fpNShapeR); fpNShapeR = dither;
            //end 32 bit dither
*/
            buf[i*2] = inputSampleL;
            buf[i*2 + 1] = inputSampleR;

        }
    }

    EveryTrim::EveryTrim() {
        A = 0.5f;
        B = 0.5f;
        C = 0.5f;
        D = 0.5f;
        E = 0.5f;
        fpNShapeL = 0.0f;
        fpNShapeR = 0.0f;
        //this is reset: values being initialized only once. Startup values, whatever they are.
    }

    EveryTrim::~EveryTrim() {
    }

    void EveryTrim::SetLeft(const float &v) {
        A = v;
        //printf("A: %f\n", A);
    }

    void EveryTrim::SetRight(const float &v) {
        B = v;
        //printf("B: %f\n", B);
    }

    void EveryTrim::SetMid(const float &v) {
        C = v;
        //printf("C: %f\n", C);
    }

    void EveryTrim::SetSide(const float &v) {
        D = v;
        //printf("C: %f\n", C);
    }

    void EveryTrim::SetMaster(const float &v) {
        E = v;
        //printf("C: %f\n", C);
    }

}

