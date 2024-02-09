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
#include <cstdint>
#include "helpers/ctagSampleRom.hpp"
#include "helpers/ctagADEnv.hpp"
#include "stmlib/dsp/filter.h"

using namespace CTAG::SP::HELPERS;

namespace CTAG::SYNTHESIS{
    class RomplerVoiceMinimal {
    public:
        enum class FilterType : uint32_t {NONE = 0x00, LP, BP, HP};
        struct Params{
            uint32_t slice;
            float playbackSpeed, pitch;
            float startOffsetRelative, lengthRelative; // relative to entire sliceLength
            float a, d;
            float cutoff, resonance;
            FilterType filterType;
            bool loop, loopPiPo;
            float loopMarker; // relative to length of subsection, not sliceLength
            float egFM;
            uint32_t bitReduction;
            // struct for filter type
            bool gate;
        };

        void Init(const float samplingRate);
        void Process(float* out, uint32_t size);
        void Reset();

        RomplerVoiceMinimal();
        ~RomplerVoiceMinimal();

        Params params;

    private:
        // mode params
        bool preGate = false;
        // internal modulation
        ctagADEnv ad;
        // multimode filter
        stmlib::Svf svf;

        // process methods for modes
        void processBlock(float *out, const uint32_t size);
        // sample data
        ctagSampleRom sampleRom;
        uint32_t slice = 0;
        // anti aliasing filter data
        float coeffs_lpf[5]{0.f};
        float w_lpf1[5]{0.f};
        float w_lpf2[5]{0.f};
        // params
        float fs = 44100.f;
        float sliceLockedStartOffset = 0.f;
        // modulation
        float fmDecay = 0.f;
        // buffer params
        enum class BufferStatus {STOPPED, READFIRST, READLAST, RUNNING};
        enum class PlayBackDirection {FWD, BWD, LOOPFWD, LOOPBWD, LOOPFWDPIPO, LOOPBWDPIPO};
        PlayBackDirection playBackDir;
        bool pipoFlip = false;
        BufferStatus bufferStatus;
        const uint32_t readBufferMaxSize = 2048; // 2k are ok given default params, there should be a bounds check with downsampling, not to exceed the buffer size
        float readBufferFloat[2048];
        int16_t readBufferInt16[2048];
        int32_t readBufferLength;
        float readBufferPhase = 0.f;
        float phaseIncrement = 0.f; // depending on pitch
        int32_t readPos = 0;
        // bit reduction masks
        const uint16_t bit_reduction_masks[15] = {
                0xc000,
                0xe000,
                0xf000,
                0xf800,
                0xfc00,
                0xfe00,
                0xff00,
                0xff80,
                0xffc0,
                0xffe0,
                0xfff0,
                0xfff8,
                0xfffc,
                0xfffe,
                0xffff};
    };
}

