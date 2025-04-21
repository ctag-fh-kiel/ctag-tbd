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
Talkbox
This module is ported from the mdaTalkbox plugin by Paul Kellet (maxim digital audio).
More information on his plugins and the original code can be found here:
http://mda.smartelectronix.com
Licence is GNU General Public License version 2.0 (GPLv2), MIT License - according to: https://sourceforge.net/projects/mda-vst
Adapted by M. Brüssel from the Soundpipe version of the plugin at: https://github.com/PaulBatchelor/Soundpipe
 */

#pragma once

#include <stdlib.h>
#include <math.h>
#include <tbd/helpers/ctagFastMath.hpp>
#include <tbd/sound_processor.hpp>

#ifndef SP_TALKBOX_BUFMAX
#define SP_TALKBOX_BUFMAX 1600
#endif

class ctagMDAtalkbox
{
  public:
    ctagMDAtalkbox();     // Please note: for your convenience this constructor will call Init(44100.f) automatically as well.
    ~ctagMDAtalkbox();
    void Init( float sample_rate, float quality_level );          // Call Init() explicitely, quality 1==max, default is 0.75 if you need a different samplerate - 44100 is standard for the TBD!
    float Process(float modulator, float carrier);

  private:
    void lpc(float *buf, float *car, uint32_t n, uint32_t o);
    void lpc_durbin(float *r, int p, float *k, float *g);
    float fast_sqrt(float n);

    float sr_;                  // Samplerate, 44100 is default!
    float quality;
    float d0, d1, d2, d3, d4;
    float u0, u1, u2, u3, u4;
    float FX;
    float emphasis;
    float* car0p;
    float* car1p;
    float* windowp;
    float* buf0p;
    float* buf1p;
    uint32_t K, N, O, pos;
};
