#pragma once
#include <cstdint>
#include "helpers/ctagSampleRom.hpp"
#include "helpers/ctagADSREnv.hpp"
#include "helpers/ctagSineSource.hpp"
#include "stmlib/dsp/filter.h"

using namespace CTAG::SP::HELPERS;

namespace CTAG::SYNTHESIS{
    class RomplerVoice {
    public:
        enum class FilterType : uint32_t {NONE = 0x00, LP, BP, HP};
        struct Params{
            uint32_t slice;
            float playbackSpeed, pitch, tune;
            float startOffsetRelative, lengthRelative; // relative to entire sliceLength
            float gain;
            float a, d, s, r;
            float cutoff, resonance;
            FilterType filterType;
            bool loop, loopPiPo;
            float loopMarker; // relative to length of subsection, not sliceLength
            float lfoSpeed;
            float lfoAM, lfoFM, lfoFMFilter;
            float egAM, egFM, egFMFilter;
            bool egSync;
            bool gate;
        };

        void Init(const float samplingRate, const uint32_t bufferSize);
        void Process(float* out, uint32_t size);
        void Reset();

        RomplerVoice();
        ~RomplerVoice();

        Params params;

    private:
        // mode params
        bool preGate = false;
        // internal modulation
        ctagADSREnv adsr;
        ctagSineSource lfo;
        stmlib::Svf svf;
        // process methods for modes
        void processBlock(float *out, const uint32_t size);
        // sample data
        ctagSampleRom sampleRom;
        // anti aliasing filter data
        float coeffs_lpf[5]{0.f};
        float w_lpf1[5]{0.f};
        float w_lpf2[5]{0.f};
        float w_lpf3[5]{0.f};
        float w_lpf4[5]{0.f};
        // params
        float fs;
        // modulation
        float adsrLastVal = 0.f, lfoLastVal = 0.f; // last because filter mod and pitch mod use last calculated value -> fs/buffersize, AM uses fs
        // buffer params
        enum class BufferStatus {STOPPED, READFIRST, READLAST, RUNNING};
        enum class PlayBackDirection {FWD, BWD, LOOPFWD, LOOPBWD, LOOPFWDPIPO, LOOPBWDPIPO};
        PlayBackDirection playBackDir;
        bool pipoFlip = false;
        BufferStatus bufferStatus;
        const uint32_t readBufferMaxSize = 2048; // 2k are ok given default params, there should be a bounds check with downsampling, not to exceed the buffer size
        float *readBufferFloat;
        int16_t *readBufferInt16;
        int32_t readBufferLength;
        float readBufferPhase = 0.f;
        float phaseIncrement = 0.f; // depending on pitch
        int32_t readPos = 0;
    };
}

