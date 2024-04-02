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

#include "RomplerVoice.hpp"
#include <cmath>
#include <cstring>
#include "stmlib/dsp/dsp.h"
#include "helpers/ctagFastMath.hpp"
#include "dsps_biquad.h"
#include "stmlib/dsp/units.h"
#include "clouds/resources.h" // use fade lut

namespace CTAG::SYNTHESIS {
    inline float InterpolateWave( // use this to save more cpu, however correct zdelays to 2 instead of 4
            const float *table,
            int32_t index_integral,
            float index_fractional) {
        float a = static_cast<float>(table[index_integral]);
        float b = static_cast<float>(table[index_integral + 1]);
        float t = index_fractional;
        return a + (b - a) * t;
    }

    inline float InterpolateWaveHermite( // better interpolation sounds better
            const float *table,
            int32_t index_integral,
            float index_fractional) {
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


    void RomplerVoice::Process(float *out, const uint32_t size) {
        // check for trigger signal
        if (params.gate == true && params.gate != preGate) { // trigger happened reset to beginning of sample
            adsr.Reset();
            svf.Reset();
            bufferStatus = BufferStatus::READFIRST;
            readBufferPhase = 0.f;
            pipoFlip = false;
        }
        preGate = params.gate;

        // compute slice parameters and check if slice data is available
        if(params.sliceLock){
            if(bufferStatus == BufferStatus::READFIRST){
                slice = params.slice;
                sliceLockedPitch = params.pitch;
                sliceLockedStartOffset = params.startOffsetRelative;
            }
        }else{
            slice = params.slice;
            sliceLockedPitch = params.pitch;
            sliceLockedStartOffset = params.startOffsetRelative;
        }

        if (!sampleRom.HasSlice(slice)) {
            memset(out, 0, size * sizeof(float));
            return;
        }
        uint32_t sliceLength = sampleRom.GetSliceSize(slice);

        // bit reduction
        int16_t brr_mask = bit_reduction_masks[14 - params.bitReduction];

        //  set eg and lfo parameters
        adsr.SetAttack(params.a);
        adsr.SetDecay(params.d);
        adsr.SetSustain(params.s);
        adsr.SetRelease(params.r);
        adsr.Gate(params.gate);
        lfo.SetFrequency(params.lfoSpeed);

        // calculate playback speed = dt = phase increment
        phaseIncrement = params.playbackSpeed; // speed
        phaseIncrement *= stmlib::SemitonesToRatio( // modulation
                sliceLockedPitch + params.tune + adsrLastVal * params.egFM +
                lfoLastVal * params.lfoFM); // includes pitch FM

        // evaluate loop settings
        if (params.loop) {
            if (params.loopPiPo) {
                playBackDir = phaseIncrement >= 0.f ? PlayBackDirection::LOOPFWDPIPO
                                                    : PlayBackDirection::LOOPBWDPIPO;
                if (pipoFlip) { // flip direction if currently already in pipo
                    if (playBackDir == PlayBackDirection::LOOPFWDPIPO)
                        playBackDir = PlayBackDirection::LOOPBWDPIPO;
                    else
                        playBackDir = PlayBackDirection::LOOPFWDPIPO;
                }
            } else {
                playBackDir = phaseIncrement >= 0.f ? PlayBackDirection::LOOPFWD
                                                    : PlayBackDirection::LOOPBWD;
            }
        } else {
            playBackDir = phaseIncrement >= 0.f ? PlayBackDirection::FWD : PlayBackDirection::BWD; // playing fwd or bwd
        }
        // now take abs
        phaseIncrement = fabsf(phaseIncrement);
        CONSTRAIN(phaseIncrement, -15.f, 15.f) // limit to stay in memory and CPU capability bounds

        // calc relative positions and bounds check
        int32_t startPos = static_cast<int32_t>(sliceLockedStartOffset * sliceLength);
        if (startPos < 0) startPos = 0;
        int32_t playLength = static_cast<int32_t>(params.lengthRelative * sliceLength);
        if (playLength >= sliceLength) playLength = sliceLength;
        int32_t endPos = startPos + playLength;
        if (endPos >= sliceLength) endPos = sliceLength; // end beyond slice length?
        // loop calculations
        int32_t loopPos = startPos + static_cast<int32_t >(params.loopMarker *
                                                           playLength); // relative to play length, could be also relative to sliceLength
        if (loopPos > endPos) loopPos = endPos;

        // in this cases return silence
        if (bufferStatus == BufferStatus::STOPPED) {
            memset(out, 0, size * sizeof(float));
            return;
        }
        if (startPos >= endPos - 1) {
            memset(out, 0, size * sizeof(float));
            return;
        }
        if (playLength == 0) {
            memset(out, 0, size * sizeof(float));
            return;
        }
        if (loopPos >= endPos - 1) {
            memset(out, 0, size * sizeof(float));
            return;
        }

        // calculate required buffer length
        // TODO: check if phase increment is within bounds for buffer max size --> partially done with asserts
        //  phaseIncrementMax*size*sizeof(datatype)+4), 32(5octaves up)*32(standard buffer size) --> > 1k words, we use 2k words
        readBufferLength = static_cast<uint32_t>(phaseIncrement * float(size) + readBufferPhase);


        // calc marks and read assemble buffers depending on playback mode
        // readPos semantic is: start read position from linear rom buffer, updated after every read cycle
        switch (playBackDir) {
            case PlayBackDirection::FWD:
                // first buffer ?
                if (bufferStatus == BufferStatus::READFIRST) {
                    readPos = startPos; // adjust playhead
                }
                // check bounds
                if (readPos + readBufferLength >= endPos) { // check if at end of sample, then read only last couple of samples
                    int32_t remainBuffer = endPos - readPos;
                    memset(readBufferInt16, 0, readBufferLength * sizeof(int16_t));
                    if(remainBuffer > 0){ // play last couple of samples
                        sampleRom.ReadSlice(readBufferInt16, slice, readPos, remainBuffer);
                        bufferStatus = BufferStatus::READLAST;
                    }else{
                        memset(out, 0, size * sizeof(float)); // silence output
                        bufferStatus = BufferStatus::STOPPED;
                        return;
                    }
                }else{
                    // obtain sample rom data
                    assert(readBufferLength <= (readBufferMaxSize - 4)); // beyond buffer size?
                    sampleRom.ReadSlice(readBufferInt16, slice, readPos, readBufferLength);
                    // and write convert to float buffer
                    for (int i = 0; i < readBufferLength; i++) {
                        readBufferFloat[i + 4] =
                                static_cast<float>(readBufferInt16[i]&brr_mask) * 0.000030518509476f; // only 2 for linear interp
                    }
                }

                // interpolate process buffer
                processBlock(out, size);

                // update read position
                readPos += readBufferLength;
                bufferStatus = BufferStatus::RUNNING;

                break;
            case PlayBackDirection::BWD:
                // first buffer?
                if (bufferStatus == BufferStatus::READFIRST) { // trigger happened reset to beginning of sample
                    readPos = endPos - readBufferLength; // adjust playhead
                }

                // check bounds
                if (readPos <= startPos) { // check if at start of sample, then read only remaining samples
                    memset(readBufferInt16, 0, readBufferLength * sizeof(int16_t));
                    int32_t remainBuffer = readPos - loopPos + readBufferLength;
                    if(remainBuffer > 0){ // read last couple of samples
                        readPos = startPos;
                        sampleRom.ReadSlice(readBufferInt16, slice, readPos, remainBuffer);
                        for (int i = 0; i < readBufferLength; i++) { // read convert reverse
                            readBufferFloat[i + 4] = static_cast<float>(readBufferInt16[remainBuffer - i - 1]&brr_mask) *
                                                     0.000030518509476f; // only 2 for linear interp
                        }
                        bufferStatus = BufferStatus::READLAST;
                    }else{
                        memset(out, 0, size * sizeof(float)); // silence output
                        bufferStatus = BufferStatus::STOPPED;
                        return;
                    }
                }else{
                    assert(readBufferLength <= (readBufferMaxSize - 4)); // beyond buffer size?
                    sampleRom.ReadSlice(readBufferInt16, slice, readPos, readBufferLength);
                    // and write convert reversed to float buffer
                    for (int i = 0; i < readBufferLength; i++) {
                        readBufferFloat[i + 4] = static_cast<float>(readBufferInt16[readBufferLength - i - 1]&brr_mask) *
                                                 0.000030518509476f; // only 2 for linear interp
                    }
                }

                // interpolate process buffer
                processBlock(out, size);

                // update read position
                readPos -= static_cast<uint32_t>(phaseIncrement * float(size) + readBufferPhase);
                bufferStatus = BufferStatus::RUNNING;

                break;
            case PlayBackDirection::LOOPFWD:
                // first buffer ?
                if (bufferStatus == BufferStatus::READFIRST) {
                    readPos = startPos; // adjust playhead
                }
                // check bounds
                if (readPos + readBufferLength >=
                    endPos) { // check if at end of sample, then read only last couple of samples
                    int32_t remainBuffer = endPos - readPos;
                    if (remainBuffer <= 0) {
                        readPos = loopPos;
                        sampleRom.ReadSlice(readBufferInt16, slice, readPos, readBufferLength);
                        readPos += readBufferLength;
                    } else {
                        int16_t *bufPos = readBufferInt16;
                        sampleRom.ReadSlice(bufPos, slice, readPos, remainBuffer);
                        readPos = loopPos;
                        // read wrap from loop pos the rest of the buffer
                        if (readBufferLength > remainBuffer) {
                            bufPos = &readBufferInt16[remainBuffer];
                            remainBuffer = readBufferLength - remainBuffer;
                            sampleRom.ReadSlice(bufPos, slice, readPos, remainBuffer);
                            readPos += remainBuffer;
                        }
                    }
                } else {
                    // obtain sample rom data
                    assert(readBufferLength <= (readBufferMaxSize - 4)); // beyond buffer size?
                    sampleRom.ReadSlice(readBufferInt16, slice, readPos, readBufferLength);
                    // update read position
                    readPos += readBufferLength;
                }

                // and write convert to float buffer
                for (int i = 0; i < readBufferLength; i++) {
                    readBufferFloat[i + 4] =
                            static_cast<float>(readBufferInt16[i]&brr_mask) * 0.000030518509476f; // only 2 for linear interp
                }

                // interpolate process buffer
                processBlock(out, size);

                bufferStatus = BufferStatus::RUNNING;
                break;

            case PlayBackDirection::LOOPBWD:
                // first buffer?
                if (bufferStatus == BufferStatus::READFIRST) { // trigger happened reset to beginning of sample
                    readPos = endPos - readBufferLength;
                }
                // check bounds
                if (readPos <= loopPos) { // in reverse mode go only until loop mark
                    int32_t remainBuffer = readPos - loopPos + readBufferLength;
                    if (remainBuffer <= 0) {
                        readPos = endPos - readBufferLength;
                        sampleRom.ReadSlice(readBufferInt16, slice, readPos, readBufferLength);
                        // and write convert reversed to float buffer
                        for (int i = 0; i < readBufferLength; i++) { // read convert reverse
                            readBufferFloat[i + 4] = static_cast<float>(readBufferInt16[readBufferLength - i - 1]&brr_mask) *
                                                     0.000030518509476f; // only 2 for linear interp
                        }
                        // interpolate process buffer
                        processBlock(out, size);
                        readPos -= static_cast<uint32_t>(phaseIncrement * float(size) + readBufferPhase);
                    } else {
                        readPos = loopPos;
                        int16_t *bufPos = &readBufferInt16[readBufferLength - remainBuffer];;
                        sampleRom.ReadSlice(bufPos, slice, readPos, remainBuffer);
                        if (readBufferLength > remainBuffer) {
                            // read remaining elements
                            bufPos = readBufferInt16;
                            remainBuffer = readBufferLength - remainBuffer;
                            readPos = endPos - remainBuffer;
                            sampleRom.ReadSlice(bufPos, slice, readPos, remainBuffer);
                        }
                        // and write convert reversed to float buffer
                        for (int i = 0; i < readBufferLength; i++) { // read convert reverse
                            readBufferFloat[i + 4] = static_cast<float>(readBufferInt16[readBufferLength - i - 1]&brr_mask) *
                                                     0.000030518509476f; // only 2 for linear interp
                        }
                        // interpolate process buffer
                        processBlock(out, size);
                        readPos -= static_cast<uint32_t>(phaseIncrement * float(size) + readBufferPhase);
                    }
                } else { // normal reverse read
                    // obtain sample rom forward
                    assert(readBufferLength <= (readBufferMaxSize - 4)); // beyond buffer size?
                    sampleRom.ReadSlice(readBufferInt16, slice, readPos, readBufferLength);
                    // and write convert reversed to float buffer
                    for (int i = 0; i < readBufferLength; i++) { // read convert reverse
                        readBufferFloat[i + 4] = static_cast<float>(readBufferInt16[readBufferLength - i - 1]&brr_mask) *
                                                 0.000030518509476f; // only 2 for linear interp
                    }
                    // interpolate process buffer
                    processBlock(out, size);
                    // update read position
                    readPos -= static_cast<uint32_t>(phaseIncrement * float(size) + readBufferPhase);
                }

                bufferStatus = BufferStatus::RUNNING;
                break;

            case PlayBackDirection::LOOPFWDPIPO:
                // first buffer ?
                if (bufferStatus == BufferStatus::READFIRST) {
                    readPos = startPos; // adjust playhead
                }
                // check bounds
                if (readPos + readBufferLength >=
                    endPos) { // check if at end of sample, then read only last couple of samples
                    int32_t remainBuffer = endPos - readPos;
                    if (remainBuffer <= 0) { // pipo event
                        readPos = endPos - readBufferLength;
                        sampleRom.ReadSlice(readBufferInt16, slice, readPos, readBufferLength);
                        // and write convert reversed to float buffer
                        for (int i = 0; i < readBufferLength; i++) { // read convert reverse
                            readBufferFloat[i + 4] = static_cast<float>(readBufferInt16[readBufferLength - i - 1]&brr_mask) *
                                                     0.000030518509476f; // only 2 for linear interp
                        }
                        // interpolate process buffer
                        processBlock(out, size);
                        pipoFlip ^= true; // toggle flip
                        readPos -= static_cast<uint32_t>(phaseIncrement * float(size) + readBufferPhase);
                    } else {
                        sampleRom.ReadSlice(readBufferInt16, slice, readPos, remainBuffer);
                        int i;
                        for (i = 0; i < remainBuffer; i++) { // still fwd
                            readBufferFloat[i + 4] =
                                    static_cast<float>(readBufferInt16[i]&brr_mask) *
                                    0.000030518509476f; // only 2 for linear interp
                        }
                        readPos += remainBuffer;
                        if (readBufferLength > remainBuffer) {
                            // read remaining elements
                            remainBuffer = readBufferLength - remainBuffer;
                            readPos = readPos - remainBuffer;
                            sampleRom.ReadSlice(readBufferInt16, slice, readPos, remainBuffer);
                            for (; i < readBufferLength; i++) { // read convert reverse
                                readBufferFloat[i + 4] = static_cast<float>(readBufferInt16[readBufferLength - i - 1]&brr_mask) *
                                                         0.000030518509476f; // only 2 for linear interp
                            }
                        }
                        // interpolate process buffer
                        processBlock(out, size);
                        readPos -= static_cast<uint32_t>(phaseIncrement * float(size) + readBufferPhase);
                        pipoFlip ^= true;
                    }
                } else {
                    // obtain sample rom data
                    assert(readBufferLength <= (readBufferMaxSize - 4)); // beyond buffer size?
                    sampleRom.ReadSlice(readBufferInt16, slice, readPos, readBufferLength);
                    // update read position
                    readPos += readBufferLength;
                    // and write convert to float buffer
                    for (int i = 0; i < readBufferLength; i++) {
                        readBufferFloat[i + 4] =
                                static_cast<float>(readBufferInt16[i]&brr_mask) * 0.000030518509476f; // only 2 for linear interp
                    }
                    // interpolate process buffer
                    processBlock(out, size);
                }

                bufferStatus = BufferStatus::RUNNING;
                break;

            case PlayBackDirection::LOOPBWDPIPO:
                // first buffer?
                if (bufferStatus == BufferStatus::READFIRST) { // trigger happened reset to beginning of sample
                    readPos = endPos - readBufferLength - 1;
                }
                // check bounds
                if (readPos <= loopPos) { // in reverse mode go only until loop mark
                    int32_t remainBuffer = readPos - loopPos + readBufferLength;
                    readPos = loopPos;
                    if (remainBuffer <= 0) { // jump to loop marker and read entire buffer from there
                        sampleRom.ReadSlice(readBufferInt16, slice, readPos, readBufferLength);
                        // and write convert to float buffer
                        for (int i = 0; i < readBufferLength; i++) { // forward play
                            readBufferFloat[i + 4] =
                                    static_cast<float>(readBufferInt16[i]&brr_mask) *
                                    0.000030518509476f; // only 2 for linear interp
                        }
                        // interpolate process buffer
                        processBlock(out, size);
                        readPos += readBufferLength;
                        pipoFlip ^= true;
                    } else {
                        int16_t *bufPos = readBufferInt16;
                        sampleRom.ReadSlice(bufPos, slice, readPos, remainBuffer);
                        // still reverse
                        int i;
                        // and write convert reversed to float buffer
                        for (i = 0; i < remainBuffer; i++) { // read convert reverse
                            readBufferFloat[i + 4] = static_cast<float>(readBufferInt16[remainBuffer - i - 1]&brr_mask) *
                                                     0.000030518509476f; // only 2 for linear interp
                        }
                        // rest forward
                        if (readBufferLength > remainBuffer) {
                            // read remaining elements
                            bufPos = &readBufferInt16[i];
                            remainBuffer = readBufferLength - remainBuffer;
                            sampleRom.ReadSlice(bufPos, slice, readPos, remainBuffer);
                            readPos += remainBuffer;
                            for (; i < readBufferLength; i++) { // rest forward
                                readBufferFloat[i + 4] =
                                        static_cast<float>(readBufferInt16[i]&brr_mask) *
                                        0.000030518509476f; // only 2 for linear interp
                            }
                        }
                        // interpolate process buffer
                        processBlock(out, size);
                        pipoFlip ^= true;
                    }
                } else { // normal reverse read
                    // obtain sample rom forward
                    assert(readBufferLength <= (readBufferMaxSize - 4)); // beyond buffer size?
                    sampleRom.ReadSlice(readBufferInt16, slice, readPos, readBufferLength);
                    // and write convert reversed to float buffer
                    for (int i = 0; i < readBufferLength; i++) { // read convert reverse
                        readBufferFloat[i + 4] = static_cast<float>(readBufferInt16[readBufferLength - i - 1]&brr_mask) *
                                                 0.000030518509476f; // only 2 for linear interp
                    }
                    // interpolate process buffer
                    processBlock(out, size);
                    // update read position
                    readPos -= static_cast<uint32_t>(phaseIncrement * float(size) + readBufferPhase);
                }

                bufferStatus = BufferStatus::RUNNING;
                break;
        }

        // check if buffer should be stopped
        if (bufferStatus == BufferStatus::READLAST) {
            bufferStatus = BufferStatus::STOPPED;
        }
        if (params.egSync) {
            if (adsr.IsIdle()) bufferStatus = BufferStatus::STOPPED;
        }

// apply filter, TODO possibly move to interpolation loop for sample accurate modulation
        float fCut = params.cutoff + params.egFMFilter * adsrLastVal +
                     params.lfoFMFilter * lfoLastVal; // TODO: Pitch tracking
        CONSTRAIN(fCut, 0.f, 1.f)
        fCut = 20.f * stmlib::SemitonesToRatio(fCut * 120.f);
        float fReso = params.resonance;
        CONSTRAIN(fReso, 1.f, 20.f)
        svf.
                set_f_q<stmlib::FREQUENCY_FAST>(fCut
                                                / 44100.f, fReso);
        switch (params.filterType) {
            case FilterType::LP:
                svf.
                        Process<stmlib::FILTER_MODE_LOW_PASS>(out, out, size
                );
                break;
            case FilterType::BP:
                svf.
                        Process<stmlib::FILTER_MODE_BAND_PASS>(out, out, size
                );
                break;
            case FilterType::HP:
                svf.
                        Process<stmlib::FILTER_MODE_HIGH_PASS>(out, out, size
                );
                break;
            default:
                break;
        }

    }

    void RomplerVoice::Init(const float samplingRate) {
        fs = samplingRate;
        adsr.SetSampleRate(fs);
        adsr.SetModeExp();
        lfo.SetSampleRate(fs);
        svf.Init();
        bufferStatus = BufferStatus::STOPPED;
        pipoFlip = false;
    }

    RomplerVoice::RomplerVoice() {
        memset(&params, 0, sizeof(Params));
        Reset();
    }

    RomplerVoice::~RomplerVoice() {
    }

    void RomplerVoice::processBlock(float *out, const uint32_t size) {
        // fade first incoming buffer
        if (bufferStatus == BufferStatus::READFIRST) {
            readBufferFloat[0] = 0.001f * readBufferFloat[4];
            readBufferFloat[1] = 0.01f * readBufferFloat[4];
            readBufferFloat[2] = 0.1f * readBufferFloat[4];
            readBufferFloat[3] = 0.5f * readBufferFloat[4];
        }
        // TODO fade last buffer
        // apply anti-aliasing low-pass when downsampling, i.e. pitch up, not required if pitch down (upsampling)
        float fAntiAlias = 0.5f / phaseIncrement;
        if (fAntiAlias < 0.5f) {
            dsps_biquad_gen_lpf_f32(coeffs_lpf, fAntiAlias, .5f);
            // the more cascades the better, but beware of cost
            dsps_biquad_f32(&readBufferFloat[4], &readBufferFloat[4], readBufferLength, coeffs_lpf, w_lpf1);
            dsps_biquad_f32(&readBufferFloat[4], &readBufferFloat[4], readBufferLength, coeffs_lpf, w_lpf2);
            dsps_biquad_f32(&readBufferFloat[4], &readBufferFloat[4], readBufferLength, coeffs_lpf, w_lpf3);
            //dsps_biquad_f32(&readBufferFloat[4], &readBufferFloat[4], readBufferLength, coeffs_lpf, w_lpf4);
        }
        // interpolate sample buffer from data
        // and apply AM
        for (int i = 0; i < size; i++) {
            // AM precalculations
            adsrLastVal = adsr.Process();
            lfoLastVal = lfo.Process();
            float amFactor = adsrLastVal * params.egAM; // adsr
            if (params.egAM < 0.f) amFactor -= params.egAM; // adsr
            amFactor = ((1.f - fabsf(params.egAM)) + amFactor); // adsr
            amFactor *= (1.f - (lfoLastVal + 1.f) * 0.5f * params.lfoAM); // lfo
            amFactor *= params.gain * params.gain; // gain (quadratic)
            // interpolate wave
            const float p = readBufferPhase;
            MAKE_INTEGRAL_FRACTIONAL(p);
            float x = InterpolateWaveHermite(readBufferFloat, p_integral, p_fractional);
            //float x = InterpolateWave(readBuffer, p_integral, p_fractional);
            // apply AM
            out[i] = x * amFactor;
            readBufferPhase += phaseIncrement;
        }
        // first buffer, fade in, TODO check for buffer sizes (LUT is 17 default, size is 32 default)
        if (bufferStatus == BufferStatus::READFIRST) {
            for (int i = 0; i < LUT_XFADE_IN_SIZE; i++) {
                out[i] *= clouds::lut_xfade_in[i];
            }
        }
        // last buffer, fade out, TODO, see above
        if (bufferStatus == BufferStatus::READLAST) {
            for (int i = 0, j = size - LUT_XFADE_OUT_SIZE; i < LUT_XFADE_OUT_SIZE; i++, j++) {
                out[j] *= clouds::lut_xfade_out[i];
            }
        }
        // TODO: interpolation artefact reduction filter
        // update phase
        readBufferPhase = readBufferPhase - static_cast<int32_t >(readBufferPhase); // phase remainder for next cycle
        // update Zs
        readBufferFloat[0] = readBufferFloat[readBufferLength];
        readBufferFloat[1] = readBufferFloat[readBufferLength + 1];
        readBufferFloat[2] = readBufferFloat[readBufferLength + 2]; // only two if linear interp
        readBufferFloat[3] = readBufferFloat[readBufferLength + 3]; // only two if linear interp
    }

    void RomplerVoice::Reset() {
        adsr.Reset();
        svf.Reset();
        readBufferFloat[0] = readBufferFloat[1] = readBufferFloat[2] = readBufferFloat[3] = 0.f;
        preGate = false;
        bufferStatus = BufferStatus::STOPPED;
        readBufferPhase = 0.f;
        pipoFlip = false;
    }

}