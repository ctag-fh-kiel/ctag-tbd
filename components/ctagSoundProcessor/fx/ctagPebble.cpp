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

#include <stdlib.h>
#include <math.h>
#include "helpers/ctagFastMath.hpp"
#include "fx/ctagPebble.hpp"

using namespace CTAG::SP::HELPERS;

void ctagPebble::Init()           // For convenience Init() already will be called by the constructor
{
  // === Faust: classInit() ---
  // --- Faust: instanceInitmydspSIG0() ---
  for (int l9 = 0; (l9 < 2); l9 = (l9 + 1))
  {
    iRec14[l9] = 0;
  }
  // --- fillmydspSIG0() ---
  for (int i = 0; (i < sizeof(ftbl0mydspSIG0)/sizeof(float)); i = (i + 1))
  {
    iRec14[0] = (iRec14[1] + 1);
    float fTemp3 = (0.0078125f * float((iRec14[0] + -1)));
    float fTemp4 = float(int(fTemp3));
    float fTemp5 = (fTemp3 - fTemp4);
    ftbl0mydspSIG0[i] = (1.0f - (1.02564108f * std::sin((2.69344211f * ((fTemp5 < 0.5f) ? fTemp5 : (fTemp4 + (1.0f - fTemp3)))))));
    iRec14[1] = iRec14[0];
  }
  // === Faust: instanceInit() ---
  // --- Faust: instanceConstants(int sample_rate) ---
  fSampleRate = 44100.f;                        // Default, use SetSampleRate() to change!
  float fConst0 = std::min<float>(192000.0f, std::max<float>(1.0f, float(fSampleRate)));
  fConst1 = std::exp((0.0f - (10.0f / fConst0)));
  fConst2 = (1.0f - fConst1);
  fConst3 = std::exp((0.0f - (207.345108f / fConst0)));
  fConst4 = (0.5f * (fConst3 + 1.0f));
  fConst5 = (0.0f - fConst4);
  fConst6 = (0.00999999978f * fConst2);
  fConst7 = (1.0f / fConst0);
  fConst8 = (2764.60156f / fConst0);
  
  // --- Faust: instanceResetUserInterface() ---
  fBypass = FAUSTFLOAT(0.0f);                           // inactive/active (0/1)
  fDryWet = FAUSTFLOAT(50.0f);                          // 0...100
  fColor = FAUSTFLOAT(1.0f);                            // 0...1
  fFeedbackDepth = FAUSTFLOAT(75.0f);                   // 0...99
  fFeedbackBassCut = FAUSTFLOAT(500.0f);                // 10...5000
  fLFOfrequency = FAUSTFLOAT(0.20000000000000001f);     // 0.00999999978...5
  
  // --- Faust: instanceClear() ---
  for (int l0 = 0; (l0 < 2); l0 = (l0 + 1)) {
      fRec0[l0] = 0.0f;
  }
  for (int l1 = 0; (l1 < 2); l1 = (l1 + 1)) {
      fRec1[l1] = 0.0f;
  }
  for (int l2 = 0; (l2 < 2); l2 = (l2 + 1)) {
      fRec7[l2] = 0.0f;
  }
  for (int l3 = 0; (l3 < 2); l3 = (l3 + 1)) {
      fRec9[l3] = 0.0f;
  }
  for (int l4 = 0; (l4 < 2); l4 = (l4 + 1)) {
      fRec8[l4] = 0.0f;
  }
  for (int l5 = 0; (l5 < 2); l5 = (l5 + 1)) {
      fRec11[l5] = 0.0f;
  }
  for (int l6 = 0; (l6 < 2); l6 = (l6 + 1)) {
      fRec10[l6] = 0.0f;
  }
  for (int l7 = 0; (l7 < 2); l7 = (l7 + 1)) {
      fRec12[l7] = 0.0f;
  }
  for (int l8 = 0; (l8 < 2); l8 = (l8 + 1)) {
      fRec13[l8] = 0.0f;
  }
  for (int l10 = 0; (l10 < 2); l10 = (l10 + 1)) {
      fRec16[l10] = 0.0f;
  }
  for (int l11 = 0; (l11 < 2); l11 = (l11 + 1)) {
      fRec15[l11] = 0.0f;
  }
  for (int l12 = 0; (l12 < 2); l12 = (l12 + 1)) {
      fRec6[l12] = 0.0f;
  }
  for (int l13 = 0; (l13 < 2); l13 = (l13 + 1)) {
      fRec5[l13] = 0.0f;
  }
  for (int l14 = 0; (l14 < 2); l14 = (l14 + 1)) {
      fRec4[l14] = 0.0f;
  }
  for (int l15 = 0; (l15 < 2); l15 = (l15 + 1)) {
      fRec3[l15] = 0.0f;
  }
  for (int l16 = 0; (l16 < 2); l16 = (l16 + 1)) {
      fRec2[l16] = 0.0f;
  }
  for (int l17 = 0; (l17 < 2); l17 = (l17 + 1)) {
      fRec17[l17] = 0.0f;
  }
}

float ctagPebble::Process(float in)   // Mono
{
float f_result = 0.f;

  // --- Faust: compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) ---
  float fSlow0 = (fConst2 * float((1 - (float(fBypass) > 0.5f))));
  float fSlow1 = (0.0157079641f * float(fDryWet));
  float fSlow2 = (fConst2 * fastcos(fSlow1));
  int iSlow3 = int(float(fColor));
  float fSlow4 = (fConst6 * float(fFeedbackDepth));
  float fSlow5 = (fConst2 * float(fFeedbackBassCut));
  float fSlow6 = (fConst2 * (iSlow3 ? 39.4868202f : 62.3695068f));
  float fSlow7 = (fConst2 * (iSlow3 ? 96.8631363f : 114.232643f));
  float fSlow8 = (fConst2 * float(fLFOfrequency));
  float fSlow9 = (fConst2 * fastsin(fSlow1));

  fRec0[0] = (fSlow0 + (fConst1 * fRec0[1]));
  fBypassMeterSymbol = FAUSTFLOAT(fRec0[0]);
  float fTemp0 = float(in);
  fRec1[0] = (fSlow2 + (fConst1 * fRec1[1]));
  fRec7[0] = (fTemp0 + (fConst3 * fRec7[1]));
  fRec9[0] = (fSlow4 + (fConst1 * fRec9[1]));
  fRec8[0] = ((fConst1 * fRec8[1]) + (fConst2 * (iSlow3 ? fRec9[0] : (0.100000001f * fRec9[0]))));
  fRec11[0] = (fSlow5 + (fConst1 * fRec11[1]));
  float fTemp1 = fasterexp((fConst7 * (0.0f - (6.28318548f * fRec11[0]))));
  fRec10[0] = (fRec2[1] + (fRec10[1] * fTemp1));
  float fTemp2 = (fTemp1 + 1.0f);
  fRec12[0] = ((fConst1 * fRec12[1]) + fSlow6);
  fRec13[0] = ((fConst1 * fRec13[1]) + fSlow7);
  fRec16[0] = (fSlow8 + (fConst1 * fRec16[1]));
  float fTemp6 = (fRec15[1] + (fConst7 * fRec16[0]));
  fRec15[0] = (fTemp6 - std::floor(fTemp6));
  float fTemp7 = (128.0f * fRec15[0]);
  int iTemp8 = int(fTemp7);
  float fTemp9 = ftbl0mydspSIG0[iTemp8];
  float fTemp10 = ((fConst8 * fasterpow2((0.0833333358f * ((fRec12[0] + ((fRec13[0] - fRec12[0]) * (fTemp9 + ((fTemp7 - float(iTemp8)) * (ftbl0mydspSIG0[((iTemp8 + 1) % 128)] - fTemp9))))) + -69.0f)))) + -1.0f);
  fRec6[0] = (((fConst5 * fRec7[1]) + ((fRec8[0] * ((0.5f * (fRec10[0] * fTemp2)) + (fRec10[1] * (0.0f - (0.5f * fTemp2))))) + (fConst4 * fRec7[0]))) - (fRec6[1] * fTemp10));
  fRec5[0] = (fRec6[1] + (fTemp10 * (fRec6[0] - fRec5[1])));
  fRec4[0] = (fRec5[1] + (fTemp10 * (fRec5[0] - fRec4[1])));
  fRec3[0] = (fRec4[1] + (fTemp10 * (fRec4[0] - fRec3[1])));
  fRec2[0] = (fRec3[1] + (fRec3[0] * fTemp10));
  fRec17[0] = (fSlow9 + (fConst1 * fRec17[1]));
  f_result = FAUSTFLOAT(((((fTemp0 * fRec1[0]) + (fRec2[0] * fRec17[0])) * fRec0[0]) + (fTemp0 * (1.0f - fRec0[0]))));
  fRec0[1] = fRec0[0];
  fRec1[1] = fRec1[0];
  fRec7[1] = fRec7[0];
  fRec9[1] = fRec9[0];
  fRec8[1] = fRec8[0];
  fRec11[1] = fRec11[0];
  fRec10[1] = fRec10[0];
  fRec12[1] = fRec12[0];
  fRec13[1] = fRec13[0];
  fRec16[1] = fRec16[0];
  fRec15[1] = fRec15[0];
  fRec6[1] = fRec6[0];
  fRec5[1] = fRec5[0];
  fRec4[1] = fRec4[0];
  fRec3[1] = fRec3[0];
  fRec2[1] = fRec2[0];
  fRec17[1] = fRec17[0];
  
  return(f_result);
}

