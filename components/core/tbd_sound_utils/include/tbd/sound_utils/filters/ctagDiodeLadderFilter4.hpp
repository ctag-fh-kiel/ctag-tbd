// This code is released under the MIT license (see below).
//
// The MIT License
//
// Copyright (c) 2012 Dominique Wurtz (www.blaukraut.info)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include "ctagFilterBase.hpp"
#include <cmath>

namespace tbd::sound_utils {
    class ctagDiodeLadderFilter4 : public ctagFilterBase {
    public:
        virtual void SetCutoff(float cutoff) override;

        virtual void SetResonance(float resonance) override;

        virtual void SetSampleRate(float fs) override;

        virtual float Process(float in) override;

        virtual void Init() override;

    private:
        static float clip(const float x) {
            return x / (1 + abs(x));
        }

        float k, A;
        float z[5] = {0.f};
        float a, ainv, a2, b, b2, c, g;
    };
}

