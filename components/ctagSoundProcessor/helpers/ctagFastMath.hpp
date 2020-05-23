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

    float fast_log10(float val);
    float fastatan(float x);

    float fast_dBV(float val);
    float fast_VdB(float val);

    void dsps_biquad_gen_bpf0db_f32(float *coeffs, float f, float qFactor);
    void dsps_biquad_gen_hpf_f32(float *coeffs, float f, float qFactor);
    void dsps_biquad_gen_lpf_f32(float *coeffs, float f, float qFactor);
}

