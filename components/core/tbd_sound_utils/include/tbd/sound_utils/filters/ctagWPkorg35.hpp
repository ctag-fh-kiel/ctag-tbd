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
Zero Delay Feedback Filters
Based on code by Will Pirkle, presented in:
http://www.willpirkle.com/Downloads/AN-4VirtualAnalogFilters.2.0.pdf
http://www.willpirkle.com/Downloads/AN-5Korg35_V3.pdf
http://www.willpirkle.com/Downloads/AN-6DiodeLadderFilter.pdf
http://www.willpirkle.com/Downloads/AN-7Korg35HPF_V2.pdf
and in his book "Designing software synthesizer plug-ins in C++ : for
RackAFX, VST3, and Audio Units"
ZDF using Trapezoidal integrator by Vadim Zavalishin, presented in "The Art
of VA Filter Design" (https://www.native-instruments.com/fileadmin/ni_media/
downloads/pdf/VAFilterDesign_1.1.1.pdf)
Adapted by M. Brüssel from a Soundpipe version which is based off of an implemenation of the Korg35 filter by Will
Pirkle. The Soundpipe version had been ported from the CCRMA chugin by the same name.

Recommended values:
SetSaturation(); 0-8.99, off when 0
SetCutoff(); 0-12000
SetResonance(); 0.0002-1.99, filter is closed when 0!
*/

#pragma once

#include "ctagFilterBase.hpp"

namespace tbd::sound_utils
{
  class ctagWPkorg35 : public ctagFilterBase
  {
    public:
      ctagWPkorg35() {Init();}

      void SetSaturation(float saturation);
      virtual void SetCutoff(float cutoff) override;
      virtual void SetResonance(float resonance) override;
      virtual void SetSampleRate(float sr) override;
      virtual float Process(float in) override;
      virtual void Init() override;

    private:
      /* LPF1 */
      float lpf1_a;
      float lpf1_z;

      /* LPF2 */
      float lpf2_a;
      float lpf2_b;
      float lpf2_z;

      /* HPF */
      float hpf_a;
      float hpf_b;
      float hpf_z;

      float alpha;
      float saturation_;

      /* variables for reuse */
      float y1 = 0.f;
      float S35 = 0.f;
      float  u = 0.f;
      float y = 0.f;
      float vn = 0.f;

      float wd = 0.f;
      float T = 0.f;
      float wa = 0.f;
      float g = 0.f;
      float G = 0.f;

      float pcutoff;      // Previous cutoff
      float pres;         // Previous resonance
      float psaturation;  // Previous saturation
  };
}



