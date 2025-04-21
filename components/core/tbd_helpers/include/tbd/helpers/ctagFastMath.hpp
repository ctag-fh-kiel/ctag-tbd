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

#pragma once

#include <stdint.h>
#include <cmath>

namespace CTAG::SP::HELPERS {
    float fastpow2(const float p);

    float fastexp(const float p);

    float expf_fast(float a);

    float fasttan(float x);

    float fastsinh(float x);

    float fastcos(float x);

    float fastsin(float x);

    inline float fastasin(float a){
        // Approximate acos(a) with relative error < 5.15e-3
// This uses an idea from Robert Harley's posting in comp.arch.arithmetic on 1996/07/12
// https://groups.google.com/forum/#!original/comp.arch.arithmetic/wqCPkCCXqWs/T9qCkHtGE2YJ
            const float PI = 3.14159265f;
            const float C  = 0.10501094f;
            float r, s, t, u;
            t = (a < 0) ? (-a) : a;  // handle negative arguments
            u = 1.0f - t;
            s = sqrtf (u + u);
            r = C * u * s + s;
            if (a < 0) r = PI - r;  // handle negative arguments
            return r;
    };

    float fasttanh(float x);

    float fasterpow2(float p);

    float fasterpow10(float p);

    float fasterexp(float p);

    float fastfabs(float p);

    float fastsqrt(float x);

//https://www.musicdsp.org/en/latest/Other/63-fast-log2.html


    float fast_log2(float val);

    float fast_logN(float val);

    float fast_log10(float val);


    float another_fast_exp(float x);

    float exp1(float x);

    float exp2(float x);

    float exp3(float x);

    float exp4(float x);

    float exp5(float x);

    float exp6(float x);

    float exp7(float x);

    float logf_fast_ub(float a);

    float logf_fast(float a);

    float logf_fast_lb(float a);

    float expf_fast_ub(float a);

    float expf_fast(float a);

    float expf_fast_lb(float a);

    float powf_fast_precise(float a, float b);

    float powf_fast(float a, float b);

    float powf_fast_lb(float a, float b);

    float powf_fast_ub(float a, float b);

    float fastatan(float x);

    float fast_dBV(float val);

    float fast_VdB(float val);

    void dsps_biquad_gen_bpf0db_f32(float *coeffs, float f, float qFactor);

    void dsps_biquad_gen_hpf_f32(float *coeffs, float f, float qFactor);

    void dsps_biquad_gen_lpf_f32(float *coeffs, float f, float qFactor);

    float InterpolateWaveLinear(
        const float *table,
        int32_t index_integral,
        float index_fractional);

    float InterpolateWaveHermite(
        const float *table,
        int32_t index_integral,
        float index_fractional);

    float InterpolateWaveLinearWrap(
    const float *table,
    const float index, const uint32_t size);

    float InterpolateWaveHermiteWrap(
        const float *table,
        const float index,
        const uint32_t size);


}

