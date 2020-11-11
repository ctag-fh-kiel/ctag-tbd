/**
 *  BiQuad filter
 *
 *  Copyright (C) 2006-2018 Teru Kamogashira
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "biquad.hpp"
#include "fv3_type_float.h"
#include "fv3_ns_start.h"

FV3_(biquad)::FV3_(biquad)() {
    a1 = a2 = b0 = b1 = b2 = 0;
    mute();
}

void FV3_(biquad)::printconfig() {
    std::fprintf(stderr, "<< BiQuad Filter Coefficients >>\n");
    std::fprintf(stderr, "(in)--+----*b0-->+----------+->(out) \n");
    std::fprintf(stderr, "      |          ^          |        \n");
    std::fprintf(stderr, "      v          |          v        \n");
    std::fprintf(stderr, "  [z^-1]---*b1-->+<-*(-a1)-[z^-1]    \n");
    std::fprintf(stderr, "      |          ^          |        \n");
    std::fprintf(stderr, "      v          |          v        \n");
    std::fprintf(stderr, "  [z^-1]---*b2-->+<-*(-a2)-[z^-1]    \n\n");
    std::fprintf(stderr, "b0 = %1.8f, b1 = %1.8f, b2 = %1.8f\n", (double) b0, (double) b1, (double) b2);
    std::fprintf(stderr, "a1 = %1.8f, a2 = %1.8f\n\n", (double) a1, (double) a2);
}

void FV3_(biquad)::mute() {
    i1 = i2 = o1 = o2 = t0 = t1 = t2 = 0.f;
}

void
FV3_(biquad)::setCoefficients(fv3_float_t _b0, fv3_float_t _b1, fv3_float_t _b2, fv3_float_t _a1, fv3_float_t _a2) {
    b0 = _b0;
    b1 = _b1;
    b2 = _b2;
    a1 = _a1;
    a2 = _a2;
}

/*
  Second-Order IIR Butterworth Filters
  The majority of the definitions and helper functions below have been
  derived from the source code of Steve Harris's SWH plugins.
  Biquad filter (adapted from lisp code by Eli Brandt, http://www.cs.cmu.edu/~eli/) 
  See the Cookbook formulae for audio EQ biquad filter coefficients
  by Robert Bristow-Johnson <rbj@audioimagination.com> for more details.
*/

static inline fv3_float_t BQ_LIMIT(fv3_float_t v, fv3_float_t l, fv3_float_t u) {
    return ((v) < (l) ? (l) : ((v) > (u) ? (u) : (v)));
}

fv3_float_t FV3_(biquad)::calcAlpha(fv3_float_t fc, fv3_float_t bw, fv3_float_t fs, unsigned mode) {
    fv3_float_t omega = 2.0 * M_PI * fc / fs;
    //fv3_float_t sn = std::sin(omega);
    fv3_float_t sn = CTAG::SP::HELPERS::fastsin(omega);
    switch (mode) {
        case FV3_BIQUAD_RBJ_BW:
            return sn * CTAG::SP::HELPERS::fastsinh(M_LN2 / 2.0f * bw * omega / sn);
        case FV3_BIQUAD_RBJ_Q:
            return sn * (2.f * bw);
        case FV3_BIQUAD_RBJ_S:
        default:
            break;
    }
    return 0;
}

void FV3_(biquad)::setAPF_RBJ(fv3_float_t fc, fv3_float_t bw, fv3_float_t fs, unsigned mode) {
    // fc : determines how steep the slope of the phase response is, when it passes through -180 degrees
    fv3_float_t omega = 2.0f * M_PI * fc / fs;
    fv3_float_t cs = CTAG::SP::HELPERS::fastcos(omega);
    fv3_float_t alpha = calcAlpha(fc, bw, fs, mode);
    fv3_float_t a0r = 1.0f / (1.0f + alpha);
    b0 = a0r * (1.0f - alpha);
    b1 = a0r * (-2.0f * cs);
    b2 = a0r * (1.0f + alpha);
    a1 = a0r * (-2.0f * cs);
    a2 = a0r * (1.0f - alpha);
}

void FV3_(biquad)::setLPF_RBJ(fv3_float_t fc, fv3_float_t bw, fv3_float_t fs, unsigned mode) {
    fv3_float_t omega = 2.0f * M_PI * fc / fs;
    fv3_float_t cs = CTAG::SP::HELPERS::fastcos(omega);
    fv3_float_t alpha = calcAlpha(fc, bw, fs, mode);
    fv3_float_t a0r = 1.0f / (1.0f + alpha);
    b0 = a0r * (1.0f - cs) * 0.5f;
    b1 = a0r * (1.0f - cs);
    b2 = a0r * (1.0f - cs) * 0.5f;
    a1 = a0r * (-2.0f * cs);
    a2 = a0r * (1.0f - alpha);
    /*
      d = Scalar, damping factor (default: square root of 2)
      if nargin < 3 d = CTAG::SP::HELPERS::fastsqrt(2); end
      beta = 0.5 * ( ( 1 - ( d / 2 ) * sin( 2 * pi * (fc / fs) ) ) / ( 1 + ( d / 2 ) * sin( 2 * pi * (fc / fs) ) ) );
      gamma = ( 0.5 + beta ) * cos ( 2 * pi * (fc / fs) );
      alfa = ( 0.5 + beta - gamma ) / 4;
      a(1)*y(n) = b(1)*x(n) + b(2)*x(n-1) + b(3)*x(n-2) - a(2)*y(n-1) - a(3)*y(n-2)
      b(1) = 2*alfa;
      b(2) = 4*alfa;
      b(3) = 2*alfa;
      a(1) = 1;
      a(2) = -2*gamma; % notice the sign!
      a(3) = 2*beta;   % notice the sign!
    */
}

void FV3_(biquad)::setHPF_RBJ(fv3_float_t fc, fv3_float_t bw, fv3_float_t fs, unsigned mode) {
    fv3_float_t omega = 2.0f * M_PI * fc / fs;
    fv3_float_t cs = CTAG::SP::HELPERS::fastcos(omega);
    fv3_float_t alpha = calcAlpha(fc, bw, fs, mode);
    float a0r = 1.0f / (1.0f + alpha);
    b0 = a0r * (1.0f + cs) * 0.5f;
    b1 = a0r * -(1.0f + cs);
    b2 = a0r * (1.0f + cs) * 0.5f;
    a1 = -1.0f * a0r * (2.0f * cs);
    a2 = -1.0f * a0r * (alpha - 1.0f);
    /*
      d = Scalar, damping factor (default: square root of 2)
      if nargin < 3 d = CTAG::SP::HELPERS::fastsqrt(2); end
      beta = 0.5 * ( ( 1 - ( d / 2 ) * sin( 2 * pi * (fc / fs) ) ) / ( 1 + ( d / 2 ) * sin( 2 * pi * (fc / fs) ) ) );
      gamma = ( 0.5 + beta ) * cos ( 2 * pi * (fc / fs) );
      alfa = ( 0.5 + beta + gamma ) / 4;
      a = zeros(1,3);
      b = zeros(1,3);
      a(1)*y(n) = b(1)*x(n) + b(2)*x(n-1) + b(3)*x(n-2) - a(2)*y(n-1) - a(3)*y(n-2)
      b(1) = 2*alfa;
      b(2) = -4*alfa;
      b(3) = 2*alfa;
      a(1) = 1;
      a(2) = -2*gamma; % notice the sign!
      a(3) = 2*beta;   % notice the sign!
    */
}

void FV3_(biquad)::setBPF_RBJ(fv3_float_t fc, fv3_float_t bw, fv3_float_t fs, unsigned mode) {
    // constant 0 dB peak gain
    fv3_float_t omega = 2.0f * M_PI * fc / fs;
    fv3_float_t cs = CTAG::SP::HELPERS::fastcos(omega);
    fv3_float_t alpha = calcAlpha(fc, bw, fs, mode);
    fv3_float_t a0r = 1.0f / (1.0f + alpha);
    b0 = a0r * alpha;
    b1 = 0.f;
    b2 = a0r * (-1.0f * alpha);
    a1 = a0r * (-2.0f * cs);
    a2 = a0r * (1.0f - alpha);
    /*
      Q = Scalar, quality factor (default: 1)
      if nargin < 3 Q = 1; end
      beta = 0.5 * ( ( 1 - tan((2 * pi * (fc / fs)) / (2 * Q)) ) / ( 1 + tan((2 * pi * (fc / fs)) / (2 * Q)) ) );
      gamma = ( 0.5 + beta ) * cos ( 2 * pi * (fc / fs) );
      alfa = ( 0.5 - beta ) / 2;
      a = zeros(1,3);
      b = zeros(1,3);
      a(1)*y(n) = b(1)*x(n) + b(2)*x(n-1) + b(3)*x(n-2) - a(2)*y(n-1) - a(3)*y(n-2)
      b(1) = 2*alfa;
      b(2) = 0;
      b(3) = -2*alfa;
      a(1) = 1;
      a(2) = -2*gamma; % notice the sign!
      a(3) = 2*beta;   % notice the sign!
     */
}

void FV3_(biquad)::setBPFP_RBJ(fv3_float_t fc, fv3_float_t bw, fv3_float_t fs, unsigned mode) {
    // constant skirt gain, peak gain = Q
    fv3_float_t omega = 2.0f * M_PI * fc / fs;
    fv3_float_t sn = CTAG::SP::HELPERS::fastsin(omega);
    fv3_float_t cs = CTAG::SP::HELPERS::fastcos(omega);
    fv3_float_t alpha = calcAlpha(fc, bw, fs, mode);
    fv3_float_t a0r = 1.0f / (1.0f + alpha);
    b0 = a0r * (0.5f * sn);
    b1 = 0;
    b2 = a0r * (-0.5f * sn);
    a1 = a0r * (-2.0f * cs);
    a2 = a0r * (1.0f - alpha);
    /*
      Second-Order IIR Butterworth Peaking Filter
      Q = Scalar, quality factor (default: 1)
      g = Scalar, boost/cut gain (in dB)
      mu = Scalar, bandpass output scaling factor
      if nargin < 4 Q = 1; end
      mu = 10^(g/20);
      beta = 0.5 * ( ( 1 - (4 / (1 + mu)) * tan((2 * pi * (fc / fs)) / (2 * Q)) ) / ( 1 + (4 / (1 + mu)) * tan((2 * pi * (fc / fs)) / (2 * Q)) ) );
      gamma = ( 0.5 + beta ) * cos ( 2 * pi * (fc / fs) );
      alfa = ( 0.5 - beta ) / 2;
      a(1)*y(n) = b(1)*x(n) + b(2)*x(n-1) + b(3)*x(n-2) - a(2)*y(n-1) - a(3)*y(n-2)
      b(1) = 2*alfa;
      b(2) = 0;
      b(3) = -2*alfa;
      a(1) = 1;
      a(2) = 2*gamma;
      a(3) = -2*beta;
     */
}

void FV3_(biquad)::setBSF_RBJ(fv3_float_t fc, fv3_float_t bw, fv3_float_t fs, unsigned mode) {
    fv3_float_t omega = 2.0f * M_PI * fc / fs;
    //fv3_float_t sn = std::sin(omega);
    fv3_float_t cs = CTAG::SP::HELPERS::fastcos(omega);
    fv3_float_t alpha = calcAlpha(fc, bw, fs, mode);
    fv3_float_t a0r = 1.0f / (1.0f + alpha);
    b0 = a0r;
    b1 = a0r * (-2.0f * cs);
    b2 = a0r;
    a1 = a0r * (-2.0f * cs);
    a2 = a0r * (1.0f - alpha);
    /*
      Second-Order IIR Butterworth Band-Stop Filter
      Q = Scalar, quality factor (default: 1)
      if nargin < 3 Q = 1; end
      beta = 0.5 * ( ( 1 - tan((2 * pi * (fc / fs)) / (2 * Q)) ) / ( 1 + tan((2 * pi * (fc / fs)) / (2 * Q)) ) );
      gamma = ( 0.5 + beta ) * cos ( 2 * pi * (fc / fs) );
      alfa = ( 0.5 + beta ) / 2;
      a(1)*y(n) = b(1)*x(n) + b(2)*x(n-1) + b(3)*x(n-2) - a(2)*y(n-1) - a(3)*y(n-2)
      b(1) = 2*alfa;
      b(2) = -2*gamma;
      b(3) = 2*alfa;
      a(1) = 1;
      a(2) = -2*gamma; % notice the sign!
      a(3) = 2*beta;   % notice the sign!
    */
}

void FV3_(biquad)::setPeakEQ_RBJ(fv3_float_t fc, fv3_float_t gain, fv3_float_t bw, fv3_float_t fs) {
    fv3_float_t w = 2.0f * M_PI * BQ_LIMIT(fc, 1.0, fs / 2.0) / fs;
    fv3_float_t cw = CTAG::SP::HELPERS::fastcos(w);
    fv3_float_t sw = CTAG::SP::HELPERS::fastsin(w);
    fv3_float_t J = powf(10.0f, gain * 0.025f);
    fv3_float_t g = sw * CTAG::SP::HELPERS::fastsinh(LN_2_2 * BQ_LIMIT(bw, 0.0001f, 4.0f) * w / sw);
    fv3_float_t a0r = 1.0f / (1.0f + (g / J));
    b0 = (1.0f + (g * J)) * a0r;
    b1 = (-2.0f * cw) * a0r;
    b2 = (1.0f - (g * J)) * a0r;
    a1 = b1;
    a2 = -1.0f * ((g / J) - 1.0f) * a0r;
}

void FV3_(biquad)::setLSF_RBJ(fv3_float_t fc, fv3_float_t gain, fv3_float_t slope, fv3_float_t fs) {
    fv3_float_t w = 2.0f * M_PI * BQ_LIMIT(fc, 1.0f, fs / 2.0f) / fs;
    fv3_float_t cw = CTAG::SP::HELPERS::fastcos(w);
    fv3_float_t sw = CTAG::SP::HELPERS::fastsin(w);
    fv3_float_t A = powf((fv3_float_t) 10.0f, gain * (fv3_float_t) 0.025f);
    fv3_float_t b = CTAG::SP::HELPERS::fastsqrt(((1.0f + A * A) / BQ_LIMIT(slope, 0.0001f, 1.0f)) - ((A - 1.0f) * (A - 1.0f)));
    fv3_float_t apc = cw * (A + 1.0f);
    fv3_float_t amc = cw * (A - 1.0f);
    fv3_float_t bs = b * sw;
    fv3_float_t a0r = 1.0f / (A + 1.0f + amc + bs);
    b0 = a0r * A * (A + 1.0f - amc + bs);
    b1 = a0r * 2.0f * A * (A - 1.0f - apc);
    b2 = a0r * A * (A + 1.0f - amc - bs);
    a1 = -1.0f * a0r * 2.0f * (A - 1.0f + apc);
    a2 = -1.0f * a0r * (-A - 1.0f - amc + bs);
}

void FV3_(biquad)::setHSF_RBJ(fv3_float_t fc, fv3_float_t gain, fv3_float_t slope, fv3_float_t fs) {
    fv3_float_t w = 2.0f * M_PI * BQ_LIMIT(fc, 1.0f, fs / 2.0f) / fs;
    fv3_float_t cw = CTAG::SP::HELPERS::fastcos(w);
    fv3_float_t sw = CTAG::SP::HELPERS::fastsin(w);
    fv3_float_t A = powf((fv3_float_t) 10.0f, gain * (fv3_float_t) 0.025f);
    fv3_float_t b = CTAG::SP::HELPERS::fastsqrt(((1.0f + A * A) / BQ_LIMIT(slope, 0.0001f, 1.0f)) - ((A - 1.0f) * (A - 1.0f)));
    fv3_float_t apc = cw * (A + 1.0f);
    fv3_float_t amc = cw * (A - 1.0f);
    fv3_float_t bs = b * sw;
    fv3_float_t a0r = 1.0f / (A + 1.0f - amc + bs);
    b0 = a0r * A * (A + 1.0f + amc + bs);
    b1 = a0r * -2.0f * A * (A - 1.0f + apc);
    b2 = a0r * A * (A + 1.0f + amc - bs);
    a1 = -1.0f * a0r * -2.0f * (A - 1.0f - apc);
    a2 = -1.0f * a0r * (-A - 1.0f + amc + bs);
}

#include "fv3_ns_end.h"
