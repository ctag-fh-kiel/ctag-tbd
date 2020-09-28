// Copyright 2012 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
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
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Oscillator - digital style waveforms.
// modified version of the supersaw digital oscillator

#pragma once

#include <cstdint>
#include "stmlib/utils/dsp.h"
#include "stmlib/utils/random.h"
#include "braids/parameter_interpolation.h"
#include "braids/resources.h"

using namespace std;
using namespace braids;
using namespace stmlib;

namespace CTAG{
    namespace SP{
        class MiSuperSawOsc {
        public:
            void Render(int16_t *buffer, size_t size);
            void SetDetune(const int16_t &detune);
            void SetPitch(const int16_t &pitch);
            void SetDamp(const uint16_t &damp);
            void Init();
        private:
            uint32_t ComputePhaseIncrement(int16_t midi_pitch);
            uint32_t ComputeDelay(int16_t midi_pitch);
            uint32_t phase[6] = {0};
            uint32_t phase_ = 0;
            uint32_t phase_increment_ = 0;
            uint32_t damp_ = 3;

            int16_t detune_ = 0;
            int16_t pitch_ = 0;

            bool strike_ = false;

            static const uint16_t kHighestNote = 140 * 128;
            static const uint16_t kPitchTableStart = 128 * 128;
            static const uint16_t kOctave = 12 * 128;
        };
    }
}


