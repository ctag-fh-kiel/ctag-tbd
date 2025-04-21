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

#include <tbd/sound_utils/fx/ctagMDAtalkbox.hpp>
#include <tbd/heaps.hpp>


namespace heaps = tbd::heaps;

using namespace CTAG::SP::HELPERS;

#ifndef TWO_PI
#define TWO_PI ((float)6.28318530717958647692528676655901)
#endif

#define ORD_MAX 50

// --- For more convienence with the talkbox-buffers in the code we use the space allocated at the pointer notated as an array of fixed lenght ---
typedef float TalkboxArray[SP_TALKBOX_BUFMAX];
#define car0    (*((TalkboxArray*)(car0p)))
#define car1    (*((TalkboxArray*)(car1p)))
#define window  (*((TalkboxArray*)(windowp)))
#define buf0    (*((TalkboxArray*)(buf0p)))
#define buf1    (*((TalkboxArray*)(buf1p)))

ctagMDAtalkbox::ctagMDAtalkbox()
{
  car0p = (float*)heaps::malloc(sizeof(TalkboxArray), TBD_HEAPS_INTERNAL | TBD_HEAPS_8BIT);
  car1p = (float*)heaps::malloc(sizeof(TalkboxArray), TBD_HEAPS_INTERNAL | TBD_HEAPS_8BIT);
  windowp = (float*)heaps::malloc(sizeof(TalkboxArray), TBD_HEAPS_INTERNAL | TBD_HEAPS_8BIT);
  buf0p = (float*)heaps::malloc(sizeof(TalkboxArray), TBD_HEAPS_INTERNAL | TBD_HEAPS_8BIT);
  buf1p = (float*)heaps::malloc(sizeof(TalkboxArray), TBD_HEAPS_INTERNAL | TBD_HEAPS_8BIT);
  // --- Vocoder quality HiFi is 1.f, LoFi is 0.4 or worse ---
  Init(44100.f, 1.f );    // Internal call of Init - call Init again if you need a different samplerate or LoFi Vocoder Quality
}

ctagMDAtalkbox::~ctagMDAtalkbox()
{
  heaps::free(car0p);
  heaps::free(car1p);
  heaps::free(windowp);
  heaps::free(buf0p);
  heaps::free(buf1p);
}

float ctagMDAtalkbox::fast_sqrt(float n)      // As seen here: https://www.gamedev.net/forums/topic/704525-3-quick-ways-to-calculate-the-square-root-in-c/
{
  int ni;
  memcpy(&ni, &n, sizeof(n));
  int ui = 0x2035AD0C + (ni >> 1);
  float u;
  memcpy(&u, &ui, sizeof(u));
  return n / u + u * 0.25f;
}

void ctagMDAtalkbox::lpc_durbin(float *r, int p, float *k, float *g)
{
int i, j;
float a[ORD_MAX], at[ORD_MAX], e = r[0];

  for (i = 0; i <= p; i++)
    a[i] = at[i] = 0.0f;

  for (i = 1; i <= p; i++)
  {
    k[i] = -r[i];

    for (j = 1; j < i; j++)
    {
      at[j] = a[j];
      k[i] -= a[j] * r[i - j];
    }
#ifdef DENORMALISATION_REQUIRED
    /* anti-denormal */
    if(fabs(e) < 1.0e-20f)
    {
      e = 0.0f;
      break;
    }
#endif
    k[i] /= e;
    a[i] = k[i];
    for (j = 1; j < i; j++)
      a[j] = at[j] + k[i] * at[i - j];

    e *= 1.0f - k[i] * k[i];
  }
#ifdef DENORMALISATION_REQUIRED
  /* anti-denormal */
  if(e < 1.0e-20f)
    e = 0.0f;
#endif

  *g = fast_sqrt(e); // ### powf_fast(e, 0.5f);  // *g = (float)sqrt(e)
  //*g = fastsqrt(e); // from ctagFastMath

}

void ctagMDAtalkbox::lpc(float *buf, float *car, uint32_t n, uint32_t o)
{
float z[ORD_MAX], r[ORD_MAX], k[ORD_MAX], G, x;
uint32_t i, j, nn=n;
float min;

  /* buf[] is already emphasized and windowed */
  for(j=0; j<=o; j++, nn--)
  {
    z[j] = r[j] = 0.0f;
    for(i=0; i<nn; i++)
      r[j] += buf[i] * buf[i+j]; /* autocorrelation */
  }
  r[0] *= 1.001f;  /* stability fix */

  min = 0.00001f;
  if(r[0] < min)
  {
    for(i=0; i<n; i++)
      buf[i] = 0.0f;
    return;
  }
  lpc_durbin(r, o, k, &G);  /* calc reflection coeffs */

  for(i=1; i<=o; i++)
  {
    if(k[i] > 0.995f)
      k[i] = 0.995f;
    else if(k[i] < -0.995f)
      k[i] = -.995f;
  }

  for(i=0; i<n; i++)
  {
    x = G * car[i];
    /* lattice filter */
    for(j=o; j>0; j--)
    {
      x -= k[j] * z[j-1];
      z[j] = z[j-1] + k[j] * x;
    }
    buf[i] = z[0] = x;  /* output buf[] will be windowed elsewhere */
  }
}

void ctagMDAtalkbox::Init(float samplerate, float quality_level)
{
  uint32_t n;
  quality = quality_level;
  N = 1;
  K = 0;

  sr_ = samplerate;
  n = (uint32_t)(0.01633f * sr_);
  if(n > SP_TALKBOX_BUFMAX)
    n = SP_TALKBOX_BUFMAX;

  /* calculate hanning window */
  if(n != N)
  {
    float dp;
    float pos;
    N = n;
    dp = TWO_PI / (float)N;
    pos = 0.0f;
    for(n=0; n < N; n++)
    {
      //window[n] = 0.5f - 0.5f * (float)fastcos(pos);
      window[n] = 0.5f - 0.5f * cosf(pos); // can be slow is only init
      pos += dp;
    }
  }
  /* zero out variables and buffers */
  pos = K = 0;
  emphasis = 0.0f;
  FX = 0;

  u0 = u1 = u2 = u3 = u4 = 0.0f;
  d0 = d1 = d2 = d3 = d4 = 0.0f;

  memset(buf0p, 0, sizeof(TalkboxArray));
  memset(buf1p, 0, sizeof(TalkboxArray));
  memset(car0p, 0, sizeof(TalkboxArray));
  memset(car1p, 0, sizeof(TalkboxArray));
}

float ctagMDAtalkbox::Process(float src, float exc)
{
    float out = 0.f;
    int32_t p0=pos, p1 = (pos + N/2) % N;
    float e=emphasis, w, o, x, fx=FX;
    float p, q, h0=0.3f, h1=0.77f;

    O = (uint32_t)((0.0001f + 0.0004f * quality) * sr_);

    o = src;
    x = exc;

    p = d0 + h0 * x;
    d0 = d1;
    d1 = x - h0 * p;

    q = d2 + h1 * d4;
    d2 = d3;
    d3 = d4 - h1 * q;

    d4 = x;

    x = p + q;

    if(K++)
    {
      K = 0;

      /* carrier input */
      car0[p0] = car1[p1] = x;
      /* 6dB/oct pre-emphasis */
      x = o - e;  e = o;
      /* 50% overlapping hanning windows */
      w = window[p0]; fx = buf0[p0] * w;  buf0[p0] = x * w;
      if(++p0 >= N)
      {
        lpc(buf0, car0, N, O);
        p0 = 0;
      }
      w = 1.0f - w;
      fx += buf1[p1] * w;
      buf1[p1] = x * w;
      if(++p1 >= N)
      {
        lpc(buf1, car1, N, O);
        p1 = 0;
      }
    }
    p = u0 + h0 * fx;
    u0 = u1;
    u1 = fx - h0 * p;

    q = u2 + h1 * u4;
    u2 = u3;
    u3 = u4 - h1 * q;

    u4 = fx;
    x = p + q;

    o = x * 0.5;
    out = o;
    emphasis = e;
    pos = p0;
    FX = fx;

#ifdef DENORMALISATION_REQUIRED
    /* anti-denormal */
    float den = 1.0e-10f;
    if(fabs(d0) < den) d0 = 0.0f;
    if(fabs(d1) < den) d1 = 0.0f;
    if(fabs(d2) < den) d2 = 0.0f;
    if(fabs(d3) < den) d3 = 0.0f;
    if(fabs(u0) < den) u0 = 0.0f;
    if(fabs(u1) < den) u1 = 0.0f;
    if(fabs(u2) < den) u2 = 0.0f;
    if(fabs(u3) < den) u3 = 0.0f;
#endif
    return( out );
}
