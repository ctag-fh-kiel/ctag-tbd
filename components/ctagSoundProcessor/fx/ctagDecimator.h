/*
 * MIT License

Copyright (c) 2020 Electrosmith, Corp.

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

// Code integrated from DaisySP: https://electro-smith.github.io/DaisySP/classdaisysp_1_1_decimator.html

#pragma once

#include <stdint.h>

namespace CTAG::SP{
/** Performs downsampling and bitcrush effects
*/
        class ctagDecimator
        {
            public:
            ctagDecimator()
            {}
            ~ctagDecimator()
            {}
            /** Initializes downsample module
            */
            void Init();

            /** Applies downsample and bitcrush effects to input signal.
                \return one sample. This should be called once per sample period.
            */
            float Process(float input);


            /** Sets amount of downsample
                Input range:
            */
            inline void SetDownsampleFactor(float downsample_factor) {
                downsample_factor_ = downsample_factor;
            }

            /** Sets amount of bitcrushing
                Input range:
            */
            inline void SetBitcrushFactor(float bitcrush_factor) {
                //            bitcrush_factor_ = bitcrush_factor;
                bits_to_crush_ = (uint32_t) (bitcrush_factor * kMaxBitsToCrush);
            }

            /** Sets the exact number of bits to crush
                0-16 bits
            */
            inline void SetBitsToCrush(const uint8_t
            &bits)
            {
                bits_to_crush_ = bits <= kMaxBitsToCrush ? bits : kMaxBitsToCrush;
            }


            /** Returns current setting of downsample
            */
            inline float GetDownsampleFactor() { return downsample_factor_; }
            /** Returns current setting of bitcrush
            */
            inline float GetBitcrushFactor() { return bitcrush_factor_; }

            private:
            const uint8_t kMaxBitsToCrush = 16;
            float downsample_factor_, bitcrush_factor_;
            uint32_t bits_to_crush_;
            float downsampled_, bitcrushed_;
            uint32_t inc_, threshold_;
        };
}
