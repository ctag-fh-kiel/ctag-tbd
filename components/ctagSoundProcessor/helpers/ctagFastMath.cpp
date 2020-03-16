#include "ctagFastMath.hpp"

namespace CTAG::SP::HELPERS {

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

    void dsps_biquad_gen_lpf_f32(float *coeffs, float f, float qFactor)
    {
        if (qFactor <= 0.0001) {
            qFactor = 0.0001;
        }
        float Fs = 1;

        float w0 = 2 * M_PI * f / Fs;
        float c = cosf(w0);
        float s = sinf(w0);
        float alpha = s / (2 * qFactor);

        float b0 = (1 - c) / 2;
        float b1 = 1 - c;
        float b2 = b0;
        float a0 = 1 + alpha;
        float a1 = -2 * c;
        float a2 = 1 - alpha;

        coeffs[0] = b0 / a0;
        coeffs[1] = b1 / a0;
        coeffs[2] = b2 / a0;
        coeffs[3] = a1 / a0;
        coeffs[4] = a2 / a0;
    }


    void dsps_biquad_gen_hpf_f32(float *coeffs, float f, float qFactor) {
        if (qFactor <= 0.0001) {
            qFactor = 0.0001;
        }
        float Fs = 1;

        float w0 = 2 * M_PI * f / Fs;
        float c = cosf(w0);
        float s = sinf(w0);
        float alpha = s / (2 * qFactor);

        float b0 = (1 + c) / 2;
        float b1 = -(1 + c);
        float b2 = b0;
        float a0 = 1 + alpha;
        float a1 = -2 * c;
        float a2 = 1 - alpha;

        coeffs[0] = b0 / a0;
        coeffs[1] = b1 / a0;
        coeffs[2] = b2 / a0;
        coeffs[3] = a1 / a0;
        coeffs[4] = a2 / a0;
    }


    void dsps_biquad_gen_bpf0db_f32(float *coeffs, float f, float qFactor)
    {
        if (qFactor <= 0.0001) {
            qFactor = 0.0001;
        }
        float Fs = 1;

        float w0 = 2 * M_PI * f / Fs;
        float c = fastcos(w0);
        float s = fastsin(w0);
        float alpha = s / (2 * qFactor);

        float b0 = alpha;
        float b1 = 0;
        float b2 = -alpha;
        float a0 = 1 + alpha;
        float a1 = -2 * c;
        float a2 = 1 - alpha;

        coeffs[0] = b0 / a0;
        coeffs[1] = b1 / a0;
        coeffs[2] = b2 / a0;
        coeffs[3] = a1 / a0;
        coeffs[4] = a2 / a0;
    }

    float fastfabs(float p) {
        return p < 0.f ? -p : p;
    }

    float fastexp(const float p) {
        const float LOG2_E = 1.4426950408889634f; // log e base 2
        return fastpow2(p * LOG2_E);
    }
    // from https://github.com/ekmett/approximate/blob/master/cbits/fast.c
    float expf_fast(float a) {
        union { float f; int x; } u;
        u.x = (int) (12102203 * a + 1064866805);
        return u.f;
    }
    // http://nghiaho.com/?p=997
    float fastatan(float x)
    {
        return M_PI_4*x - x*(fabs(x) - 1)*(0.2447f + 0.0663f*fabs(x));
    }

}