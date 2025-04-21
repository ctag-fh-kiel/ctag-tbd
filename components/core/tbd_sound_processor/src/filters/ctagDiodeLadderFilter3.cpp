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

#include <tbd/sound_utils/filters/ctagDiodeLadderFilter3.hpp>
#include <cstdio>
#include <tbd/logging.hpp>
#include <cmath>
#include <limits>

void CTAG::SP::HELPERS::ctagDiodeLadderFilter3::SetCutoff(float cutoff) {
    b_fenv = cutoff / fs_;
}

void CTAG::SP::HELPERS::ctagDiodeLadderFilter3::SetResonance(float resonance) {
    b_fres = resonance * 10.f;
}

void CTAG::SP::HELPERS::ctagDiodeLadderFilter3::SetSampleRate(float fs) {
    fs_ = fs;
}

/*
 * Karlsen Fast Ladder III - inspired by "transistors set to work as diode" type Roland filters. The best fast and non-nonsensical approximation of popular analog filter sound, as in for instance Roland SH-5, and the smaller TB-303.

//Coupled with oversampling and simple oscs you will probably get the best analog approximation.

//          // for nice low sat, or sharper type low deemphasis saturation, one can use a onepole shelf before the filter.
//          b_lf = b_lf + ((-b_lf + b_v) * b_lfcut); // b_lfcut 0..1
//          double b_lfhp = b_v - b_lf;
//          b_v = b_lf + (b_lf1hp * ((b_lfgain*0.5)+1));

            double b_rez = b_aflt4 - b_v; // no attenuation with rez, makes a stabler filter.
            b_v = b_v - (b_rez*b_fres); // b_fres = resonance amount. 0..4 typical "to selfoscillation", 0.6 covers a more saturated range.

            double b_vnc = b_v; // clip, and adding back some nonclipped, to get a dynamic like analog.
            if (b_v > 1) {b_v = 1;} else if (b_v < -1) {b_v = -1;}
            b_v = b_vnc + ((-b_vnc + b_v) * 0.9840);

            b_aflt1 = b_aflt1 + ((-b_aflt1 + b_v) * b_fenv); // straightforward 4 pole filter, (4 normalized feedback paths in series)
            b_aflt2 = b_aflt2 + ((-b_aflt2 + b_aflt1) * b_fenv);
            b_aflt3 = b_aflt3 + ((-b_aflt3 + b_aflt2) * b_fenv);
            b_aflt4 = b_aflt4 + ((-b_aflt4 + b_aflt3) * b_fenv);
            b_v = b_aflt4;

// Behave.
// Ove Hy Karlsen.
*/
float CTAG::SP::HELPERS::ctagDiodeLadderFilter3::Process(float in) {
    float b_v = in;

    b_lf = b_lf + ((-b_lf + b_v) * b_lfcut); // b_lfcut 0..1
    float b_lfhp = b_v - b_lf;
    b_v = b_lf + (b_lfhp * ((b_lfgain * 0.5f) + 1.f));

    float b_rez = b_aflt4 - b_v; // no attenuation with rez, makes a stabler filter.
    if(b_fres > 10.f){
        TBD_LOGE("FILT", "Error reso %f", b_fres);
        b_fres = 0.f;
    }
    b_v = b_v - (b_rez *
                 b_fres); // b_fres = resonance amount. 0..4 typical "to selfoscillation", 0.6 covers a more saturated range.

    float b_vnc = b_v; // clip, and adding back some nonclipped, to get a dynamic like analog.
    if (b_v > 1) { b_v = 1; } else if (b_v < -1) { b_v = -1; }
    // lower number for more grit at high resonance
    b_v = b_vnc + ((-b_vnc + b_v) * 0.6123f); // original 0.9740 seems to affect barking at high resonance levels



    b_aflt1 = b_aflt1 +
              ((-b_aflt1 + b_v) * b_fenv); // straightforward 4 pole filter, (4 normalized feedback paths in series)

    b_aflt2 = b_aflt2 + ((-b_aflt2 + b_aflt1) * b_fenv);
    b_aflt3 = b_aflt3 + ((-b_aflt3 + b_aflt2) * b_fenv);
    b_aflt4 = b_aflt4 + ((-b_aflt4 + b_aflt3) * b_fenv);
    b_v = b_aflt4;

    return b_v;
}

void CTAG::SP::HELPERS::ctagDiodeLadderFilter3::Init() {
    b_fres = 0.f;
    b_fenv = 0.5f;
    b_aflt1 = b_aflt2 = b_aflt3 = b_aflt4 = 0.f;
    b_lf = 0.f;
    b_lfcut = 0.2f;
    b_lfgain = 10.f;
}

void CTAG::SP::HELPERS::ctagDiodeLadderFilter3::Debug() {
    ctagFilterBase::Debug();
    printf("afilt 1-4 %f %f %f %f\n", b_aflt1, b_aflt2, b_aflt3, b_aflt4);
    printf("fres, fenv %f, %f\n", b_fres, b_fenv);
    printf("lf, lfcut, lfgain %f, %f, %f\n", b_lf, b_lfcut, b_lfgain);
}
