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

#include <cmath>
#include "ctagWPkorg35.hpp"
#include <helpers/ctagFastMath.hpp>

using namespace CTAG::SP::HELPERS;

void ctagWPkorg35::Init()
{
  fs_ = 44100.f;
  alpha = 0.f;
  pcutoff = cutoff_ = 1000.f;
  pres = resonance_ = 1.f;

  /* reset memory for filters */
  lpf1_z = 0.f;
  lpf2_z = 0.f;
  hpf_z = 0.f;

  /* initialize LPF1 */
  lpf1_a = 1.f;
  lpf1_z = 0.f;
  
  /* initialize LPF2 */
  lpf2_a = 1.f;
  lpf2_b = 1.f;
  lpf2_z = 0.f;

  saturation_ = 0.f;
  
  /* update filters, prewarp for BZT */
  wd = 2*M_PI*cutoff_;
  T  = 1.f/fs_;
  wa = (2.f/T)*fasttanh(wd*T/2.f);
  g  = wa*T/2.f;
  /* the feedforward coeff in the VA One Pole */
  G = g/(1.f + g);

  /* set alphas */
  lpf1_a = G;
  lpf2_a = G;
  hpf_a = G;

  /* set betas */
  lpf2_b = (resonance_ - resonance_*G)/(1.f + g);
  hpf_b = -1.f/(1.f + g);

  alpha = 1.f/(1.f - resonance_*G + resonance_*G*G);
}

void ctagWPkorg35::SetSaturation(float saturation)
{
  saturation_ = saturation;
}

void ctagWPkorg35::SetCutoff(float cutoff)
{
  cutoff_ = cutoff;
}

void ctagWPkorg35::SetResonance(float resonance)
{
  if( resonance <= 0 )
    resonance_ = 0.0002f;   // Low value for a 10bit range from 0 to 1
  else
    resonance_ = resonance;
}

void ctagWPkorg35::SetSampleRate(float sr)
{
  fs_ = sr;
}

float ctagWPkorg35::Process(float in)
{
  /* initialize variables */
  y1 = 0.f;
  S35 = 0.f;
  u = 0.f;
  y = 0.f;
  vn = 0.f;

  if(pcutoff != cutoff_ || pres != resonance_ || psaturation != saturation_ )     // Reinit filter if external values have changed!
  {
    /* prewarp for BZT */
    wd = 2*M_PI*cutoff_;
    T  = 1.f/fs_;
    wa = (2.f/T)*fasttanh(wd*T/2.f);
    g  = wa*T/2.f;
    /* the feedforward coeff in the VA One Pole */
    G = g/(1.f + g);

    /* set alphas */
    lpf1_a = G;
    lpf2_a = G;
    hpf_a = G;

    /* set betas */
    lpf2_b = (resonance_ - resonance_*G)/(1.f + g);
    hpf_b = -1.f/(1.f + g);

    alpha = 1.f/(1.f - resonance_*G + resonance_*G*G);
  }

  /* process input through LPF1 */
  vn = (in - lpf1_z) * lpf1_a;
  y1 = vn + lpf1_z;
  lpf1_z = y1 + vn;

  /* form feedback value */
  S35 = (hpf_z * hpf_b) + (lpf2_z * lpf2_b);

  /* Calculate u */
  u = alpha * (y1 + S35);

  /* Naive NLP */
  if(saturation_ > 0.f)
    u = fasttanh(saturation_ * u);

  /* Feed it to LPF2 */
  vn = (u - lpf2_z) * lpf2_a;
  y = (vn + lpf2_z);
  lpf2_z = y + vn;
  y *= resonance_;

  /* Feed y to HPF2 */
  vn = (y - hpf_z) * hpf_a;
  hpf_z = vn + (vn + hpf_z);

  /* Auto-normalize */
  if(resonance_ > 0.f)
    y *= 1.0 / resonance_;

  pcutoff = cutoff_;
  pres = resonance_;
  psaturation = saturation_;

  return(y);
}

