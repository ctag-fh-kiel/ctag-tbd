/*
MIT License

Copyright (c) 2018 Andrew Minnich https://github.com/tesselode

Permission is hereby granted, free of charge, to any person obtaining a copy
        of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
        to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
        copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
        copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

 */

#pragma once

#include <cmath>
#include <cstdint>
#include <tbd/sound_utils/ctagFastMath.hpp>

using namespace tbd::sound_utils;

namespace tesselode {
    const float pi = M_PI;

// https://stackoverflow.com/a/707426
    inline int wrap(int kX, int const kLowerBound, int const kUpperBound) {
        int range_size = kUpperBound - kLowerBound + 1;

        if (kX < kLowerBound)
            kX += range_size * ((kLowerBound - kX) / range_size + 1);

        return kLowerBound + (kX - kLowerBound) % range_size;
    }

// http://musicdsp.org/archive.php?classid=5#93
    inline float interpolate(float x, float y0, float y1, float y2, float y3) {
        // 4-point, 3rd-order Hermite (x-form)
        float c0 = y1;
        float c1 = 0.5f * (y2 - y0);
        float c2 = y0 - 2.5f * y1 + 2.f * y2 - 0.5f * y3;
        float c3 = 1.5f * (y1 - y2) + 0.5f * (y3 - y0);
        return ((c3 * x + c2) * x + c1) * x + c0;
    }

    inline void adjustPanning(float inL, float inR, float angle, float &outL, float &outR) {
        auto c = fastcos(angle);
        auto s = fastsin(angle);
        outL = inL * c - inR * s;
        outR = inL * s + inR * c;
    }

// random numbers

// https://stackoverflow.com/questions/1640258/need-a-fast-random-generator-for-c
    static uint32_t x = 123456789, y = 362436069, z = 521288629;

    inline uint32_t xorshift(void) {
        unsigned long t;
        x ^= x << 16;
        x ^= x >> 5;
        x ^= x << 1;
        t = x;
        x = y;
        y = z;
        z = t ^ x ^ y;
        return z;
    }

    const float xorshiftMultiplier = 2.0f / UINT32_MAX;

    inline float random() {
        return -1.0f + xorshift() * xorshiftMultiplier;
    }
}

