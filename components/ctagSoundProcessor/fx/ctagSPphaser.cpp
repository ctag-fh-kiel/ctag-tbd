#include <stdlib.h>
#include <math.h>
#include "helpers/ctagFastMath.hpp"
#include "fx/ctagSPphaser.hpp"

#define phaser_max(a,b) ((a < b) ? b : a)
#define phaser_min(a,b) ((a < b) ? a : b)
#define phaser_faustpower2_f(value) (value * value)
#define phaser_faustpower3_f(value) (value * value * value)
#define phaser_faustpower4_f(value) (value * value * value * value)

/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020,2021 by Robert Manzke. All rights reserved.

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
Phaser effect, based on the soundpipe port by Paul Bachelor generated from Faust code taken from the Guitarix project, as to be found here:
https://github.com/PaulBatchelor/Soundpipe
https://paulbatchelor.github.io/res/soundpipe/docs/phaser.html
Adapted by M. Brüssel from the Soundpipe version.
*/
using namespace CTAG::SP::HELPERS;

namespace CTAG::SP{
    void ctagSPphaser::Init()           // For convenience Init() already will be called by the constructor
    {
        fSamplingFreq_ = 44100.f;     // Default is 44.1 kHz, use SetSampleRate() afterwards to change -
        level_ = (float) 0.;
        for (int i0 = 0; (i0 < 2); i0 = (i0 + 1))
            iVec0[i0] = 0;

        vibratoMode_ = (float) 0.;
        depth_ = (float) 1.;
        iConst0 = phaser_min(192000, phaser_max(1, fSamplingFreq_));
        fConst1 = (1.f / (float) iConst0);
        notchWidth_ = (float) 1000.;
        notchFreq_ = (float) 1.5;
        minNotch1Freq_ = (float) 100.;
        maxNotch1Freq_ = (float) 800.;
        fConst2 = (0.10472f / (float) iConst0);
        lfobpm_ = (float) 30.;

        for (int i1 = 0; (i1 < 2); i1 = (i1 + 1))
            fRec5[i1] = 0.f;
        for (int i2 = 0; (i2 < 2); i2 = (i2 + 1))
            fRec6[i2] = 0.f;
        feedbackGain_ = (float) 0.;
        for (int i3 = 0; (i3 < 3); i3 = (i3 + 1))
            fRec4[i3] = 0.f;
        for (int i4 = 0; (i4 < 3); i4 = (i4 + 1))
            fRec3[i4] = 0.f;
        for (int i5 = 0; (i5 < 3); i5 = (i5 + 1))
            fRec2[i5] = 0.f;
        for (int i6 = 0; (i6 < 3); i6 = (i6 + 1))
            fRec1[i6] = 0.f;
        for (int i7 = 0; (i7 < 2); i7 = (i7 + 1))
            fRec0[i7] = 0.f;
        for (int i8 = 0; (i8 < 3); i8 = (i8 + 1))
            fRec11[i8] = 0.f;
        for (int i9 = 0; (i9 < 3); i9 = (i9 + 1))
            fRec10[i9] = 0.f;
        for (int i10 = 0; (i10 < 3); i10 = (i10 + 1))
            fRec9[i10] = 0.f;
        for (int i11 = 0; (i11 < 3); i11 = (i11 + 1))
            fRec8[i11] = 0.f;
        for (int i12 = 0; (i12 < 2); i12 = (i12 + 1))
            fRec7[i12] = 0.f;
    }

    void ctagSPphaser::Process(float* in_l, float* in_r ,float* out_l,float* out_r)     // Stereo
    {
        float* input0 = in_l;
        float* input1 = in_r;
        float* output0 = out_l;
        float* output1 = out_r;
        float fSlow0 = fasterpow10(0.05f * (float)level_);
        float fSlow1 = (0.5f * ((int)(float) vibratoMode_?2.f:(float)depth_));
        float fSlow2 = (1.f - fSlow1);
        float fSlow3 = fasterexp((fConst1 * (0.f - (3.14159f * (float)notchWidth_))));
        float fSlow4 = phaser_faustpower2_f(fSlow3);
        float fSlow5 = (0.f - (2.f * fSlow3));
        float fSlow6 = (float)notchFreq_;
        float fSlow7 = (fConst1 * fSlow6);
        float fSlow8 = (float)minNotch1Freq_;
        float fSlow9 = (6.28319f * fSlow8);
        float fSlow10 = (0.5f * ((6.28319f * phaser_max(fSlow8, (float)maxNotch1Freq_)) - fSlow9));
        float fSlow11 = (fConst2 * (float)lfobpm_);
        float fSlow12 = fastsin(fSlow11);
        float fSlow13 = fastcos(fSlow11);
        float fSlow14 = (0.f - fSlow12);
        float fSlow15 = (float)feedbackGain_;
        float fSlow16 = (fConst1 * phaser_faustpower2_f(fSlow6));
        float fSlow17 = (fConst1 * phaser_faustpower3_f(fSlow6));
        float fSlow18 = (fConst1 * phaser_faustpower4_f(fSlow6));
        float fSlow19 = ((int)(float)invert_?(0.f - fSlow1):fSlow1);

        iVec0[0] = 1;
        float fTemp0 = (float)(*input0);
        fRec5[0] = ((fSlow12 * fRec6[1]) + (fSlow13 * fRec5[1]));
        fRec6[0] = ((1.f + ((fSlow13 * fRec6[1]) + (fSlow14 * fRec5[1]))) - (float)iVec0[1]);
        float fTemp1 = ((fSlow10 * (1.f - fRec5[0])) + fSlow9);
        float fTemp2 = (fRec4[1] * fastcos((fSlow7 * fTemp1)));
        fRec4[0] = (0.f - (((fSlow5 * fTemp2) + (fSlow4 * fRec4[2])) - ((fSlow0 * fTemp0) + (fSlow15 * fRec0[1]))));
        float fTemp3 = (fRec3[1] * fastcos((fSlow16 * fTemp1)));
        fRec3[0] = ((fSlow5 * (fTemp2 - fTemp3)) + (fRec4[2] + (fSlow4 * (fRec4[0] - fRec3[2]))));
        float fTemp4 = (fRec2[1] * fastcos((fSlow17 * fTemp1)));
        fRec2[0] = ((fSlow5 * (fTemp3 - fTemp4)) + (fRec3[2] + (fSlow4 * (fRec3[0] - fRec2[2]))));
        float fTemp5 = (fRec1[1] * fastcos((fSlow18 * fTemp1)));
        fRec1[0] = ((fSlow5 * (fTemp4 - fTemp5)) + (fRec2[2] + (fSlow4 * (fRec2[0] - fRec1[2]))));
        fRec0[0] = ((fSlow4 * fRec1[0]) + ((fSlow5 * fTemp5) + fRec1[2]));
        *output0 = (float)((fSlow0 * (fSlow2 * fTemp0)) + (fRec0[0] * fSlow19));
        float fTemp6 = (float)(*input1);
        float fTemp7 = ((fSlow10 * (1.f - fRec6[0])) + fSlow9);
        float fTemp8 = (fRec11[1] * fastcos((fSlow7 * fTemp7)));
        fRec11[0] = (0.f - (((fSlow5 * fTemp8) + (fSlow4 * fRec11[2])) - ((fSlow0 * fTemp6) + (fSlow15 * fRec7[1]))));
        float fTemp9 = (fRec10[1] * fastcos((fSlow16 * fTemp7)));
        fRec10[0] = ((fSlow5 * (fTemp8 - fTemp9)) + (fRec11[2] + (fSlow4 * (fRec11[0] - fRec10[2]))));
        float fTemp10 = (fRec9[1] * fastcos((fSlow17 * fTemp7)));
        fRec9[0] = ((fSlow5 * (fTemp9 - fTemp10)) + (fRec10[2] + (fSlow4 * (fRec10[0] - fRec9[2]))));
        float fTemp11 = (fRec8[1] * fastcos((fSlow18 * fTemp7)));
        fRec8[0] = ((fSlow5 * (fTemp10 - fTemp11)) + (fRec9[2] + (fSlow4 * (fRec9[0] - fRec8[2]))));
        fRec7[0] = ((fSlow4 * fRec8[0]) + ((fSlow5 * fTemp11) + fRec8[2]));
        *output1 = (float)((fSlow0 * (fSlow2 * fTemp6)) + (fRec7[0] * fSlow19));
        iVec0[1] = iVec0[0];
        fRec5[1] = fRec5[0];
        fRec6[1] = fRec6[0];
        fRec4[2] = fRec4[1];
        fRec4[1] = fRec4[0];
        fRec3[2] = fRec3[1];
        fRec3[1] = fRec3[0];
        fRec2[2] = fRec2[1];
        fRec2[1] = fRec2[0];
        fRec1[2] = fRec1[1];
        fRec1[1] = fRec1[0];
        fRec0[1] = fRec0[0];
        fRec11[2] = fRec11[1];
        fRec11[1] = fRec11[0];
        fRec10[2] = fRec10[1];
        fRec10[1] = fRec10[0];
        fRec9[2] = fRec9[1];
        fRec9[1] = fRec9[0];
        fRec8[2] = fRec8[1];
        fRec8[1] = fRec8[0];
        fRec7[1] = fRec7[0];
    }

    float ctagSPphaser::Process(float in)   // Mono
    {
        float result = 0.f;

        float fSlow0 = fasterpow10(0.05f * (float)level_);
        float fSlow1 = (0.5f * ((int)(float) vibratoMode_?2.f:(float)depth_));
        float fSlow2 = (1.f - fSlow1);
        float fSlow3 = fasterexp((fConst1 * (0.f - (3.14159f * (float)notchWidth_))));
        float fSlow4 = phaser_faustpower2_f(fSlow3);
        float fSlow5 = (0.f - (2.f * fSlow3));
        float fSlow6 = (float)notchFreq_;
        float fSlow7 = (fConst1 * fSlow6);
        float fSlow8 = (float)minNotch1Freq_;
        float fSlow9 = (6.28319f * fSlow8);
        float fSlow10 = (0.5f * ((6.28319f * phaser_max(fSlow8, (float)maxNotch1Freq_)) - fSlow9));
        float fSlow11 = (fConst2 * (float)lfobpm_);
        float fSlow12 = fastsin(fSlow11);
        float fSlow13 = fastcos(fSlow11);
        float fSlow14 = (0.f - fSlow12);
        float fSlow15 = (float)feedbackGain_;
        float fSlow16 = (fConst1 * phaser_faustpower2_f(fSlow6));
        float fSlow17 = (fConst1 * phaser_faustpower3_f(fSlow6));
        float fSlow18 = (fConst1 * phaser_faustpower4_f(fSlow6));
        float fSlow19 = ((int)(float)invert_?(0.f - fSlow1):fSlow1);

        iVec0[0] = 1;
        float fTemp0 = in;
        fRec5[0] = ((fSlow12 * fRec6[1]) + (fSlow13 * fRec5[1]));
        fRec6[0] = ((1.f + ((fSlow13 * fRec6[1]) + (fSlow14 * fRec5[1]))) - (float)iVec0[1]);
        float fTemp1 = ((fSlow10 * (1.f - fRec5[0])) + fSlow9);
        float fTemp2 = (fRec4[1] * fastcos((fSlow7 * fTemp1)));
        fRec4[0] = (0.f - (((fSlow5 * fTemp2) + (fSlow4 * fRec4[2])) - ((fSlow0 * fTemp0) + (fSlow15 * fRec0[1]))));
        float fTemp3 = (fRec3[1] * fastcos((fSlow16 * fTemp1)));
        fRec3[0] = ((fSlow5 * (fTemp2 - fTemp3)) + (fRec4[2] + (fSlow4 * (fRec4[0] - fRec3[2]))));
        float fTemp4 = (fRec2[1] * fastcos((fSlow17 * fTemp1)));
        fRec2[0] = ((fSlow5 * (fTemp3 - fTemp4)) + (fRec3[2] + (fSlow4 * (fRec3[0] - fRec2[2]))));
        float fTemp5 = (fRec1[1] * fastcos((fSlow18 * fTemp1)));
        fRec1[0] = ((fSlow5 * (fTemp4 - fTemp5)) + (fRec2[2] + (fSlow4 * (fRec2[0] - fRec1[2]))));
        fRec0[0] = ((fSlow4 * fRec1[0]) + ((fSlow5 * fTemp5) + fRec1[2]));
        result = (float)((fSlow0 * (fSlow2 * fTemp0)) + (fRec0[0] * fSlow19));
        iVec0[1] = iVec0[0];
        fRec5[1] = fRec5[0];
        fRec6[1] = fRec6[0];
        fRec4[2] = fRec4[1];
        fRec4[1] = fRec4[0];
        fRec3[2] = fRec3[1];
        fRec3[1] = fRec3[0];
        fRec2[2] = fRec2[1];
        fRec2[1] = fRec2[0];
        fRec1[2] = fRec1[1];
        fRec1[1] = fRec1[0];
        fRec0[1] = fRec0[0];
        fRec11[2] = fRec11[1];
        fRec11[1] = fRec11[0];
        fRec10[2] = fRec10[1];
        fRec10[1] = fRec10[0];
        fRec9[2] = fRec9[1];
        fRec9[1] = fRec9[0];
        fRec8[2] = fRec8[1];
        fRec8[1] = fRec8[0];
        fRec7[1] = fRec7[0];
        return(result);
    }
}



