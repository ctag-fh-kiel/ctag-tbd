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

/* based on CStrip by Chris Johnson, see LICENSE file */
/* ported to CTAG TBD by Christian Feyer */

#pragma once

namespace airwindows {
    class CStripM {
    public:
        void Process(float *buf, int bufSz, int ch);

        void SetTreble(const float &); // 0 ... 1
        void SetMid(const float &); // 0 ... 1
        void SetBass(const float &); // 0 ... 1
        void SetLowpass(const float &); // 0 ... 1
        void SetTrebfreq(const float &); // 0 ... 1
        void SetBassfreq(const float &); // 0 ... 1
        void SetHipass(const float &); // 0 ... 1
        void SetGate(const float &); // 0 ... 1
        void SetComp(const float &); // 0 ... 1
        void SetCompspd(const float &); // 0 ... 1
        void SetTimelag(const float &); // 0 ... 1
        void SetOutgain(const float &); // 0 ... 1

        CStripM();

        ~CStripM();

    private:
        float fpNShapeL;
        //default stuff

        float lastSampleL;
        float last2SampleL;


        //begin EQ
        float iirHighSampleLA;
        float iirHighSampleLB;
        float iirHighSampleLC;
        float iirHighSampleLD;
        float iirHighSampleLE;
        float iirLowSampleLA;
        float iirLowSampleLB;
        float iirLowSampleLC;
        float iirLowSampleLD;
        float iirLowSampleLE;
        float iirHighSampleL;
        float iirLowSampleL;

        float tripletLA;
        float tripletLB;
        float tripletLC;
        float tripletFactorL;

        float lowpassSampleLAA;
        float lowpassSampleLAB;
        float lowpassSampleLBA;
        float lowpassSampleLBB;
        float lowpassSampleLCA;
        float lowpassSampleLCB;
        float lowpassSampleLDA;
        float lowpassSampleLDB;
        float lowpassSampleLE;
        float lowpassSampleLF;
        float lowpassSampleLG;


        float highpassSampleLAA;
        float highpassSampleLAB;
        float highpassSampleLBA;
        float highpassSampleLBB;
        float highpassSampleLCA;
        float highpassSampleLCB;
        float highpassSampleLDA;
        float highpassSampleLDB;
        float highpassSampleLE;
        float highpassSampleLF;


        bool flip;
        int flipthree;
        //end EQ

        //begin Gate
        bool WasNegativeL;
        int ZeroCrossL;
        float gaterollerL;
        float gateL;

        //end Gate

        //begin Timing
        float pL[4099];
        int count;
        //end Timing

        //begin ButterComp
        float controlAposL;
        float controlAnegL;
        float controlBposL;
        float controlBnegL;
        float targetposL;
        float targetnegL;
        float avgLA;
        float avgLB;
        float nvgLA;
        float nvgLB;

        //end ButterComp
        //flip is already covered in EQ

        float A;
        float B;
        float C;
        float D;
        float E;
        float F;
        float G;
        float H;
        float I;
        float J;
        float K;
        float L;
    };
}
