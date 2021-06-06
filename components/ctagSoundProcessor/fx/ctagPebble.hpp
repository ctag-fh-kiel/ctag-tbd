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
Phaser effect, based on the Faust-Code for a "Electro Harmonix Small Stone" type of phaser by Jean Pierre Cimalando at https://github.com/jpcima/stone-phaser
Référence :
     Kiiski, R., Esqueda, F., & Välimäki, V. (2016).
     Time-variant gray-box modeling of a phaser pedal.
     In 19th International Conference on Digital Audio Effects (DAFx-16).
Adapted by M. Brüssel from the C++ code derived from the "stone_phaser.dsp" there via https://faustide.grame.fr
*/

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

class ctagPebble
{
  public:
    ctagPebble() {Init();}
    void Init();
    float Process(float in);  // Mono
    inline void SetSampleRate(float sampleRate) { fSampleRate = sampleRate; }
    inline void SetDryWet(float dryWet) { fDryWet = dryWet; }
    inline void SetColor(float color) { fColor = color; }
    inline void SetBypass(bool bypass) { fBypass = (float)bypass; }
    inline void SetFeedbackDepth(float feedbackDepth) { fFeedbackDepth = feedbackDepth; }
    inline void SetFeedbackBassCut(float feedbackBassCut) { fFeedbackBassCut = feedbackBassCut; }
    inline void SetLFOfrequency(float lfoFrequency) { fLFOfrequency = lfoFrequency; }

  private:
    int iRec14[2];                            // Faust class variable, used only during initialistation
    float ftbl0mydspSIG0[128];                // Internal table
  
    int fSampleRate;
    float fConst1;
    float fConst2;
    FAUSTFLOAT fBypass;                       // inactive/active (0/1)
    float fRec0[2];
    FAUSTFLOAT fBypassMeterSymbol;            // Unused for now
    FAUSTFLOAT fDryWet;                       // 0...100
    float fRec1[2];
    float fConst3;
    float fConst4;
    float fConst5;
    float fRec7[2];
    FAUSTFLOAT fColor;                        // 0...1
    float fConst6;
    FAUSTFLOAT fFeedbackDepth;                // 0...99
    float fRec9[2];
    float fRec8[2];
    float fConst7;
    FAUSTFLOAT fFeedbackBassCut;              // 10...5000
    float fRec11[2];
    float fRec10[2];
    float fConst8;
    float fRec12[2];
    float fRec13[2];
    FAUSTFLOAT fLFOfrequency;                 // 0.00999999978...5
    float fRec16[2];
    float fRec15[2];
    float fRec6[2];
    float fRec5[2];
    float fRec4[2];
    float fRec3[2];
    float fRec2[2];
    float fRec17[2];
};
