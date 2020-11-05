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

    float fasttanh(float x);

    float fasterpow2(float p);

    float fasterpow10(float p);

    float fasterexp(float p);

    float fastfabs(float p);

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

    float powf_fast(float a, float b);

    float powf_fast_lb(float a, float b);

    float powf_fast_ub(float a, float b);

    float fastatan(float x);

    float fast_dBV(float val);

    float fast_VdB(float val);

    void dsps_biquad_gen_bpf0db_f32(float *coeffs, float f, float qFactor);

    void dsps_biquad_gen_hpf_f32(float *coeffs, float f, float qFactor);

    void dsps_biquad_gen_lpf_f32(float *coeffs, float f, float qFactor);
}

