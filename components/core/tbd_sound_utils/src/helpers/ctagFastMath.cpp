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
#include <tbd/sound_utils/ctagFastMath.hpp>

#include <cstdio>
#include <stmlib/dsp/dsp.h>

namespace tbd::sound_utils {

    //static float cos_73(float x);

    float fastpow2(const float p) {
        float offset = (p < 0) ? 1.0f : 0.0f;
        float clipp = (p < -126) ? -126.0f : p;
        int w = clipp;
        float z = clipp - w + offset;
        union {
            uint32_t i;
            float f;
        } v = {(uint32_t) ((1 << 23) * (clipp + 121.2740575f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z))};
        return v.f;
    }

#define PI_2    1.5707963267948966192313216916398f
#define PI      3.1415926535897932384626433832795f
#define TWO_PI  6.283185307179586476925286766559f
#define PI_4    0.78539816339744830961566084581988f
#define FOUR_OVER_PI    1.2732395447351626861510701069801f

// from http://www.ganssle.com/approx.htm
// accepts values 0 < x < pi/2
    float fasttan(float x) {
#define TAN_C1 211.849369664121f
#define TAN_C2 -12.5288887278448f
#define TAN_C3 269.7350131214121f
#define TAN_C4 -71.4145309347748f
        int octant = (int) x / PI_4;
        float v = octant == 0 ? (x * FOUR_OVER_PI) : ((PI_2 - x) * FOUR_OVER_PI);
        float v2 = v * v;
        v = v * (TAN_C1 + TAN_C2 * v2) / (TAN_C3 + v2 * (TAN_C4 + v2));
        if (octant > 0)
            return 1.0f / v;
        return v;
    }

    float cos_73(float x) {
#define COS_C1 0.999999953464f
#define COS_C2 -0.4999999053455f
#define COS_C3 0.0416635846769f
#define COS_C4 -0.0013853704264f
#define COS_C5 0.00002315393167f
        float x2 = x * x;
        return (COS_C1 + x2 * (COS_C2 + x2 * (COS_C3 + x2 * (COS_C4 + COS_C5 * x2))));
    }

// accepts values 0 < x < 2*pi
    float fastcos(float x) {
        if (x < 0)x = -x;
        int quadrant = (int) (x / PI_2);
        switch (quadrant) {
            case 0:
                return cos_73(x);
            case 1:
                return -cos_73(PI - x);
            case 2:
                return -cos_73(x - PI);
            case 3:
                return cos_73(TWO_PI - x);
        }
        return 0.f;
    }

    float fastsin(float x) {
        return fastcos(PI_2 - x);
    }

    float fasttanh(float x) {
        float x2 = x * x;
        return x * (27.f + x2) / (27.f + 9.f * x2);
    }

    float fasterpow2(float p) {
        float clipp = (p < -126) ? -126.0f : p;
        union {
            uint32_t i;
            float f;
        } v = {(uint32_t) ((1 << 23) * (clipp + 126.94269504f))};
        return v.f;
    }

    float fasterpow10(float p) {
        const float LOG2_10 = 3.3219281f; // log 10 base 2
        return fasterpow2(p * LOG2_10);
    }

    float fasterexp(float p) {
        const float LOG2_E = 1.4426950408889634f; // log e base 2
        return fasterpow2(p * LOG2_E);
    }


    //https://www.musicdsp.org/en/latest/Other/63-fast-log2.html
    float fast_log2(float val) {
        if (val < 0.f) return NAN;

        int *const exp_ptr = (int *) (&val);
        int x = *exp_ptr;
        const int log_2 = ((x >> 23) & 255) - 128;
        x &= ~(255 << 23);
        x += 127 << 23;
        *exp_ptr = x;

        return (val + log_2);
    }

    float fast_logN(float val) {
        const float LN_2 = 0.693147180559945f; // log 2 base e
        return LN_2 * fast_log2(val);
    }

    float fast_log10(float val) {
        const float LOG10_2 = 0.301029995663981f; // log 2 base 10
        return LOG10_2 * fast_log2(val);
    }

    float fast_dBV(float val) {
        return 20.f * fast_log10(val);
    }

    float fast_VdB(float val) {
        return fasterpow10(val / 20.f);
    }

    void dsps_biquad_gen_lpf_f32(float *coeffs, float f, float qFactor) {
        if (qFactor <= 0.0001f) {
            qFactor = 0.0001f;
        }
        float Fs = 1.f;

        float w0 = 2.f * PI * f / Fs;
        float c = fastcos(w0);
        float s = fastsin(w0);
        float alpha = s / (2.f * qFactor);

        float b0 = (1.f - c) / 2.f;
        float b1 = 1.f - c;
        float b2 = b0;
        float a0 = 1.f + alpha;
        float a1 = -2.f * c;
        float a2 = 1.f - alpha;

        coeffs[0] = b0 / a0;
        coeffs[1] = b1 / a0;
        coeffs[2] = b2 / a0;
        coeffs[3] = a1 / a0;
        coeffs[4] = a2 / a0;
    }


    void dsps_biquad_gen_hpf_f32(float *coeffs, float f, float qFactor) {
        if (qFactor <= 0.0001f) {
            qFactor = 0.0001f;
        }
        float Fs = 1.f;

        float w0 = 2.f * PI * f / Fs;
        float c = fastcos(w0);
        float s = fastsin(w0);
        float alpha = s / (2.f * qFactor);

        float b0 = (1.f + c) / 2.f;
        float b1 = -(1.f + c);
        float b2 = b0;
        float a0 = 1.f + alpha;
        float a1 = -2.f * c;
        float a2 = 1.f - alpha;

        coeffs[0] = b0 / a0;
        coeffs[1] = b1 / a0;
        coeffs[2] = b2 / a0;
        coeffs[3] = a1 / a0;
        coeffs[4] = a2 / a0;
    }


    void dsps_biquad_gen_bpf0db_f32(float *coeffs, float f, float qFactor) {
        if (qFactor <= 0.0001f) {
            qFactor = 0.0001f;
        }
        float Fs = 1.f;

        float w0 = 2.f * PI * f / Fs;
        float c = fastcos(w0);
        float s = fastsin(w0);
        float alpha = s / (2.f * qFactor);

        float b0 = alpha;
        float b1 = 0.f;
        float b2 = -alpha;
        float a0 = 1.f + alpha;
        float a1 = -2.f * c;
        float a2 = 1.f - alpha;

        coeffs[0] = b0 / a0;
        coeffs[1] = b1 / a0;
        coeffs[2] = b2 / a0;
        coeffs[3] = a1 / a0;
        coeffs[4] = a2 / a0;
    }

    float fastfabs(float p) {
        // slower than fabs!
        //return p < 0.f ? -p : p;
        return fabsf(p);
    }

    float fastexp(const float p) {
        const float LOG2_E = 1.4426950408889634f; // log e base 2
        return fastpow2(p * LOG2_E);
    }

    // from https://github.com/ekmett/approximate/blob/master/cbits/fast.c
    /* 1065353216 - 722019 */
    float logf_fast_ub(float a) {
        union {
            float f;
            int x;
        } u = {a};
        return (u.x - 1064631197) * 8.262958405176314e-8f; /* 1 / 12102203.0; */
    }

    // https://stackoverflow.com/questions/10552280/fast-exp-calculation-possible-to-improve-accuracy-without-losing-too-much-perfo
    float another_fast_exp(float x) {
        volatile union {
            float f;
            unsigned int i;
        } cvt;

        /* exp(x) = 2^i * 2^f; i = floor (log2(e) * x), 0 <= f <= 1 */
        float t = x * 1.442695041f;
        float fi = floorf(t);
        float f = t - fi;
        int i = (int) fi;
        cvt.f = (0.3371894346f * f + 0.657636276f) * f + 1.00172476f; /* compute 2^f */
        cvt.i = cvt.i + (i << 23);                                          /* scale by 2^i */
        return cvt.f;
    }

    // from https://github.com/ekmett/approximate/blob/master/cbits/fast.c
    /*
 These constants are based loosely on the following comment off of Ankerl's blog:
 "I have used the same trick for float, not double, with some slight modification to the constants to suite IEEE754 float format. The first constant for float is 1<<23/log(2) and the second is 127<<23 (for double they are 1<<20/log(2) and 1023<<20)." -- John
*/

    /* should be much more precise with large b */
    float powf_fast_precise(float a, float b) {
        int flipped = 0;
        if (b < 0) {
            flipped = 1;
            b = -b;
        }

        /* calculate approximation with fraction of the exponent */
        int e = (int) b;
        union { float f; int x; } u = { a };
        u.x = (int)((b - e) * (u.x - 1065353216) + 1065353216);

        float r = 1.0f;
        while (e) {
            if (e & 1) {
                r *= a;
            }
            a *= a;
            e >>= 1;
        }

        r *= u.f;

        return flipped ? 1.0f/r : r;
    }

/* 1065353216 + 1      = 1065353217 ub */
/* 1065353216 - 486411 = 1064866805 min RMSE */
/* 1065353216 - 722019 = 1064631197 lb */
    float powf_fast(float a, float b) {
        union {
            float d;
            int x;
        } u = {a};
        u.x = (int) (b * (u.x - 1064866805) + 1064866805);
        //float f = u.d; printf("Args base %f, exp %f, True pow %f, fast pow %f, relative error %f%%\n", a, b, powf(a, b), f,  (f/ powf(a, b) - 1.f) * 100.f);
        return u.d;
    }

    float powf_fast_lb(float a, float b) {
        union {
            float d;
            int x;
        } u = {a};
        u.x = (int) (b * (u.x - 1065353217) + 1064631197);
        return u.d;
    }

    float powf_fast_ub(float a, float b) {
        union {
            float d;
            int x;
        } u = {a};
        u.x = (int) (b * (u.x - 1064631197) + 1065353217);
        return u.d;
    }


    //https://stackoverflow.com/questions/10552280/fast-exp-calculation-possible-to-improve-accuracy-without-losing-too-much-perfo
    float exp1(float x) {
        return (6 + x * (6 + x * (3 + x))) * 0.16666666f;
    }

    float exp2(float x) {
        return (24 + x * (24 + x * (12 + x * (4 + x)))) * 0.041666666f;
    }

    float exp3(float x) {
        return (120 + x * (120 + x * (60 + x * (20 + x * (5 + x))))) * 0.0083333333f;
    }

    float exp4(float x) {
        return (720 + x * (720 + x * (360 + x * (120 + x * (30 + x * (6 + x)))))) * 0.0013888888f;
    }

    float exp5(float x) {
        return (5040 + x * (5040 + x * (2520 + x * (840 + x * (210 + x * (42 + x * (7 + x))))))) * 0.00019841269f;
    }

    float exp6(float x) {
        return (40320 + x * (40320 + x * (20160 + x * (6720 + x * (1680 + x * (336 + x * (56 + x * (8 + x)))))))) *
               2.4801587301e-5f;
    }

    float exp7(float x) {
        return (362880 + x * (362880 + x * (181440 + x * (60480 + x * (15120 + x * (3024 + x * (504 + x * (72 + x * (9 +
                                                                                                                     x))))))))) *
               2.75573192e-6f;
    }

/* Ankerl's adaptation of Schraudolph's published algorithm with John's constants */
/* 1065353216 - 486411 = 1064866805 */
    float logf_fast(float a) {
        union {
            float f;
            int x;
        } u = {a};
        return (u.x - 1064866805) * 8.262958405176314e-8f; /* 1 / 12102203.0; */
    }

/* 1065353216 + 1 */
    float logf_fast_lb(float a) {
        union {
            float f;
            int x;
        } u = {a};
        return (u.x - 1065353217) * 8.262958405176314e-8f; /* 1 / 12102203.0 */
    }

    /* 1065353216 + 1 */
    float expf_fast_ub(float a) {
        union {
            float f;
            int x;
        } u;
        u.x = (int) (12102203 * a + 1065353217);
        return u.f;
    }

/* Schraudolph's published algorithm with John's constants */
/* 1065353216 - 486411 = 1064866805 */
    float expf_fast(float a) {
        union {
            float f;
            int x;
        } u;
        u.x = (int) (12102203 * a + 1064866805);
        return u.f;
    }

    /* 1065353216 - 722019 */
    float expf_fast_lb(float a) {
        union {
            float f;
            int x;
        } u;
        u.x = (int) (12102203 * a + 1064631197);
        return u.f;
    }

    // http://nghiaho.com/?p=997
    float fastatan(float x) {
        return M_PI_4 * x - x * (fabs(x) - 1) * (0.2447f + 0.0663f * fabs(x));
    }

    float fastsinh(float x) {
        return (expf_fast(x) - expf_fast(-x)) * 0.5f;
    }

    // https://www.codeproject.com/Articles/69941/Best-Square-Root-Method-Algorithm-Function-Precisi
    float fastsqrt(float x) {
        union
        {
            int i;
            float x;
        } u;
        u.x = x;
        u.i = (1<<29) + (u.i >> 1) - (1<<22);

        // Two Babylonian Steps (simplified from:)
        // u.x = 0.5f * (u.x + x/u.x);
        // u.x = 0.5f * (u.x + x/u.x);
        u.x =       u.x + x/u.x;
        u.x = 0.25f*u.x + x/u.x;

        //float f = u.x; printf("True sqrt %f, fast pow %f, relative error %f%%\n", sqrtf(x), f,  (f/ sqrtf(x) - 1.f) * 100.f);

        return u.x;
    }

    // from Mutable Instruments
    float InterpolateWaveLinear(
        const float *table,
        const int32_t index_integral,
        const float index_fractional) {
        const float a = table[index_integral];
        const float b = table[index_integral + 1];
        const float t = index_fractional;
        return a + (b - a) * t;
    }

    // from Mutable Instruments
    float InterpolateWaveHermite(
            const float *table,
            const int32_t index_integral,
            const float index_fractional) {
        const float xm1 = table[index_integral];
        const float x0 = table[index_integral + 1];
        const float x1 = table[index_integral + 2];
        const float x2 = table[index_integral + 3];
        const float c = (x1 - xm1) * 0.5f;
        const float v = x0 - x1;
        const float w = c + v;
        const float a = w + v + (x2 - x0) * 0.5f;
        const float b_neg = w + a;
        const float f = index_fractional;
        return (((a * f) - b_neg) * f + c) * f + x0;
    }

    float InterpolateWaveLinearWrap(
    const float *table,
    const float index, const uint32_t size) {
        MAKE_INTEGRAL_FRACTIONAL(index)
        const float a = table[index_integral%size];
        const float b = table[(index_integral + 1)%size];
        const float t = index_fractional;
        return a + (b - a) * t;
    }

    float InterpolateWaveHermiteWrap(
        const float *table,
        const float index,
        const uint32_t size){
        MAKE_INTEGRAL_FRACTIONAL(index)
        const float xm1 = table[index_integral%size];
        const float x0 = table[(index_integral + 1)%size];
        const float x1 = table[(index_integral + 2)%size];
        const float x2 = table[(index_integral + 3)%size];
        const float c = (x1 - xm1) * 0.5f;
        const float v = x0 - x1;
        const float w = c + v;
        const float a = w + v + (x2 - x0) * 0.5f;
        const float b_neg = w + a;
        const float f = index_fractional;
        return (((a * f) - b_neg) * f + c) * f + x0;
    }

}
