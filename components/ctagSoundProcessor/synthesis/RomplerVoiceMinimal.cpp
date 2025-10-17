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

#include "RomplerVoiceMinimal.hpp"
#include <cmath>
#include <cstring>
#include <cassert>
#include "stmlib/dsp/dsp.h"
#include "helpers/ctagFastMath.hpp"
#include "dsps_biquad.h"
#include "stmlib/dsp/units.h"
#include "clouds/resources.h" // use fade lut

namespace CTAG::SYNTHESIS {


    void RomplerVoiceMinimal::Process(float *out, const uint32_t size) {
        // Ensure internal temp buffers are large enough (adjust kTSMaxBlock if this asserts)
        assert(size <= kTSMaxBlock);

        tsCurrentStretch = params.timeStretch;

        // check for trigger signal
        if (params.gate == true && params.gate != preGate) { // trigger happened reset to beginning of sample
            bufferStatus = BufferStatus::READFIRST;
            readBufferPhase = 0.f;
            pipoFlip = false;
            fmDecay = 1.f;

            // when time stretching enabled
            if (params.timeStretchEnable) {
                // Apply preset window unless Custom
                float reqMs;
                switch (params.timeStretchQuality) {
                    case Params::TSQuality::Low:      reqMs = 6.f;  break;
                    case Params::TSQuality::Balanced: reqMs = 8.f; break;
                    case Params::TSQuality::Smooth:   reqMs = 12.f; break;
                    case Params::TSQuality::Custom:
                    default:                          reqMs = params.timeStretchWindowMs; break;
                }
                // Clamp and set window
                const float maxMs = (PitchShifterTD::MAX_WINDOW_SIZE * 1000.0f) / fs;
                const float minMs = 2.0f; // practical floor
                reqMs = std::clamp(reqMs, minMs, maxMs);
                if (fabsf(reqMs - tsWindowMsApplied) > 0.01f){
                    int w = static_cast<int>(reqMs * (fs * 0.001f) + 0.5f);
                    if (w < 8) w = 8;
                    if (w > PitchShifterTD::MAX_WINDOW_SIZE) w = PitchShifterTD::MAX_WINDOW_SIZE;
                    if (w & 1) ++w; // even length preferred
                    shifter.setWindowSize(w);
                    tsWindowMsApplied = reqMs;
                }
            }
            ad.Trigger();
        }
        preGate = params.gate;

        // compute slice parameters and check if slice data is available
        slice = params.slice;
        sliceLockedStartOffset = params.startOffsetRelative;

        if (!sampleRom.HasSlice(slice)) {
            memset(out, 0, size * sizeof(float));
            return;
        }
        uint32_t sliceLength = sampleRom.GetSliceSize(slice);

        // bit reduction
        int16_t brr_mask = bit_reduction_masks[14 - params.bitReduction];

        //  set eg parameters
        ad.SetAttack(params.a);
        ad.SetDecay(params.d);

        // calculate playback speed = dt = phase increment (transport)
        const float Suse = params.timeStretchEnable ? tsCurrentStretch : 1.0f;

        // Direction is based solely on playbackSpeed sign (timeStretch >= 0)
        const float dirVal = params.playbackSpeed;
        if (params.loop) {
            if (params.loopPiPo) {
                playBackDir = dirVal >= 0.f ? PlayBackDirection::LOOPFWDPIPO
                                            : PlayBackDirection::LOOPBWDPIPO;
                if (pipoFlip) { // flip direction if currently already in pipo
                    if (playBackDir == PlayBackDirection::LOOPFWDPIPO)
                        playBackDir = PlayBackDirection::LOOPBWDPIPO;
                    else
                        playBackDir = PlayBackDirection::LOOPFWDPIPO;
                }
            } else {
                playBackDir = dirVal >= 0.f ? PlayBackDirection::LOOPFWD
                                            : PlayBackDirection::LOOPBWD;
            }
        } else {
            playBackDir = dirVal >= 0.f ? PlayBackDirection::FWD : PlayBackDirection::BWD;
        }

        // Build original transport as in legacy path, then apply smoothed S if time-stretch is active
        float baseRate = fabsf(params.playbackSpeed) * stmlib::SemitonesToRatio(params.pitch);
        if (baseRate >= 6.f) baseRate = 6.f;
        float fmAdd = fmDecay * params.egFM * 4.f; // legacy additive behavior retained
        float transportNoStretch = baseRate + fmAdd; // legacy effective increment
        float transportRate = transportNoStretch * Suse;

        phaseIncrement = transportRate; // used by interpolation and AA

        // update FM decay
        fmDecay *= (0.9f + 0.0999999f * params.d / 50.f);

        // calc relative positions and bounds check
        int32_t startPos = static_cast<int32_t>(sliceLockedStartOffset * sliceLength);
        if (startPos < 0) startPos = 0;
        int32_t playLength = static_cast<int32_t>(params.lengthRelative * sliceLength);
        if (playLength >= sliceLength) playLength = sliceLength;
        int32_t endPos = startPos + playLength;
        if (endPos >= sliceLength) endPos = sliceLength;
        int32_t loopPos = startPos + static_cast<int32_t >(params.loopMarker * playLength);
        if (loopPos > endPos) loopPos = endPos;

        // in this cases return silence
        if (bufferStatus == BufferStatus::STOPPED) { memset(out, 0, size * sizeof(float)); return; }
        if (startPos >= endPos - 1) { memset(out, 0, size * sizeof(float)); return; }
        if (playLength == 0) { memset(out, 0, size * sizeof(float)); return; }
        if (loopPos >= endPos - 1) { memset(out, 0, size * sizeof(float)); return; }

        // calculate required buffer length
        readBufferLength = static_cast<uint32_t>(phaseIncrement * float(size) + readBufferPhase);

        // Choose output sink for the interpolation stage
        float* procOut = params.timeStretchEnable ? tsIn : out;

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
                    assert(readBufferLength <= (readBufferMaxSize - 2)); // beyond buffer size?
                    sampleRom.ReadSlice(readBufferInt16, slice, readPos, readBufferLength);
                    // and write convert to float buffer
                    for (int i = 0; i < readBufferLength; i++) {
                        readBufferFloat[i + 2] =
                                static_cast<float>(readBufferInt16[i]&brr_mask) * 0.000030518509476f; // only 2 for linear interp
                    }
                }

                // interpolate process buffer
                processBlock(procOut, size);

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
                            readBufferFloat[i + 2] = static_cast<float>(readBufferInt16[remainBuffer - i - 1]&brr_mask) *
                                                     0.000030518509476f; // only 2 for linear interp
                        }
                        bufferStatus = BufferStatus::READLAST;
                    }else{
                        memset(out, 0, size * sizeof(float)); // silence output
                        bufferStatus = BufferStatus::STOPPED;
                        return;
                    }
                }else{
                    assert(readBufferLength <= (readBufferMaxSize - 2)); // beyond buffer size?
                    sampleRom.ReadSlice(readBufferInt16, slice, readPos, readBufferLength);
                    // and write convert reversed to float buffer
                    for (int i = 0; i < readBufferLength; i++) {
                        readBufferFloat[i + 2] = static_cast<float>(readBufferInt16[readBufferLength - i - 1]&brr_mask) *
                                                 0.000030518509476f; // only 2 for linear interp
                    }
                }

                // interpolate process buffer
                processBlock(procOut, size);

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
                    assert(readBufferLength <= (readBufferMaxSize - 2)); // beyond buffer size?
                    sampleRom.ReadSlice(readBufferInt16, slice, readPos, readBufferLength);
                    // update read position
                    readPos += readBufferLength;
                }

                // and write convert to float buffer
                for (int i = 0; i < readBufferLength; i++) {
                    readBufferFloat[i + 2] =
                            static_cast<float>(readBufferInt16[i]&brr_mask) * 0.000030518509476f; // only 2 for linear interp
                }

                // interpolate process buffer
                processBlock(procOut, size);

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
                            readBufferFloat[i + 2] = static_cast<float>(readBufferInt16[readBufferLength - i - 1]&brr_mask) *
                                                     0.000030518509476f; // only 2 for linear interp
                        }
                        // interpolate process buffer
                        processBlock(procOut, size);
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
                            readBufferFloat[i + 2] = static_cast<float>(readBufferInt16[readBufferLength - i - 1]&brr_mask) *
                                                     0.000030518509476f; // only 2 for linear interp
                        }
                        // interpolate process buffer
                        processBlock(procOut, size);
                        readPos -= static_cast<uint32_t>(phaseIncrement * float(size) + readBufferPhase);
                    }
                } else { // normal reverse read
                    // obtain sample rom forward
                    assert(readBufferLength <= (readBufferMaxSize - 2)); // beyond buffer size?
                    sampleRom.ReadSlice(readBufferInt16, slice, readPos, readBufferLength);
                    // and write convert reversed to float buffer
                    for (int i = 0; i < readBufferLength; i++) { // read convert reverse
                        readBufferFloat[i + 2] = static_cast<float>(readBufferInt16[readBufferLength - i - 1]&brr_mask) *
                                                 0.000030518509476f; // only 2 for linear interp
                    }
                    // interpolate process buffer
                    processBlock(procOut, size);
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
                            readBufferFloat[i + 2] = static_cast<float>(readBufferInt16[readBufferLength - i - 1]&brr_mask) *
                                                     0.000030518509476f; // only 2 for linear interp
                        }
                        // interpolate process buffer
                        processBlock(procOut, size);
                        pipoFlip ^= true; // toggle flip
                        tsFlipFadeCount = kFlipFadeLen; // request short declick fade after flip
                        readPos -= static_cast<uint32_t>(phaseIncrement * float(size) + readBufferPhase);
                    } else {
                        sampleRom.ReadSlice(readBufferInt16, slice, readPos, remainBuffer);
                        int i;
                        for (i = 0; i < remainBuffer; i++) { // still fwd
                            readBufferFloat[i + 2] =
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
                                readBufferFloat[i + 2] = static_cast<float>(readBufferInt16[readBufferLength - i - 1]&brr_mask) *
                                                         0.000030518509476f; // only 2 for linear interp
                            }
                        }
                        // interpolate process buffer
                        processBlock(procOut, size);
                        readPos -= static_cast<uint32_t>(phaseIncrement * float(size) + readBufferPhase);
                        pipoFlip ^= true;
                        tsFlipFadeCount = kFlipFadeLen; // request short declick fade after flip
                    }
                } else {
                    // obtain sample rom data
                    assert(readBufferLength <= (readBufferMaxSize - 2)); // beyond buffer size?
                    sampleRom.ReadSlice(readBufferInt16, slice, readPos, readBufferLength);
                    // update read position
                    readPos += readBufferLength;
                    // and write convert to float buffer
                    for (int i = 0; i < readBufferLength; i++) {
                        readBufferFloat[i + 2] =
                                static_cast<float>(readBufferInt16[i]&brr_mask) * 0.000030518509476f; // only 2 for linear interp
                    }
                    // interpolate process buffer
                    processBlock(procOut, size);
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
                            readBufferFloat[i + 2] =
                                    static_cast<float>(readBufferInt16[i]&brr_mask) *
                                    0.000030518509476f; // only 2 for linear interp
                        }
                        // interpolate process buffer
                        processBlock(procOut, size);
                        readPos += readBufferLength;
                        pipoFlip ^= true;
                        tsFlipFadeCount = kFlipFadeLen; // request short declick fade after flip
                    } else {
                        int16_t *bufPos = readBufferInt16;
                        sampleRom.ReadSlice(bufPos, slice, readPos, remainBuffer);
                        // still reverse
                        int i;
                        // and write convert reversed to float buffer
                        for (i = 0; i < remainBuffer; i++) { // read convert reverse
                            readBufferFloat[i + 2] = static_cast<float>(readBufferInt16[remainBuffer - i - 1]&brr_mask) *
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
                                readBufferFloat[i + 2] =
                                        static_cast<float>(readBufferInt16[i]&brr_mask) *
                                        0.000030518509476f; // only 2 for linear interp
                            }
                        }
                        // interpolate process buffer
                        processBlock(procOut, size);
                        pipoFlip ^= true;
                        tsFlipFadeCount = kFlipFadeLen; // request short declick fade after flip
                    }
                } else { // normal reverse read
                    // obtain sample rom forward
                    assert(readBufferLength <= (readBufferMaxSize - 2)); // beyond buffer size?
                    sampleRom.ReadSlice(readBufferInt16, slice, readPos, readBufferLength);
                    // and write convert reversed to float buffer
                    for (int i = 0; i < readBufferLength; i++) { // read convert reverse
                        readBufferFloat[i + 2] = static_cast<float>(readBufferInt16[readBufferLength - i - 1]&brr_mask) *
                                                 0.000030518509476f; // only 2 for linear interp
                    }
                    // interpolate process buffer
                    processBlock(procOut, size);
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
        // Don't stop while priming time-stretch (envelope intentionally idle then)
        if (!ad.GetIsRunning()) bufferStatus = BufferStatus::STOPPED;

        if (params.timeStretchEnable) {
            // Apply pitch correction via shifter: correct only the stretch, keep legacy pitch behavior
            const float correction = 1.0f / std::clamp(Suse, 0.01f, 64.f);
            shifter.process(tsIn, out, size, correction);
        }

        // Apply short fade after ping-pong flips to reduce clicks (both TS and non-TS paths)
        if (tsFlipFadeCount > 0) {
            int n = tsFlipFadeCount < static_cast<int>(size) ? tsFlipFadeCount : static_cast<int>(size);
            // Use clouds LUT if available, else linear ramp
            for (int i = 0; i < n; ++i) {
                float w;
                if (i < LUT_XFADE_IN_SIZE) w = clouds::lut_xfade_in[i]; else w = (i + 1) / static_cast<float>(n);
                out[i] *= w;
            }
            tsFlipFadeCount -= n;
        }

        // filter
        float fCut = params.cutoff;
        CONSTRAIN(fCut, 0.f, 1.f)
        fCut = 20.f * stmlib::SemitonesToRatio(fCut * 120.f);
        float fReso = params.resonance;
        CONSTRAIN(fReso, .5f, 20.f)
        svf.set_f_q<stmlib::FREQUENCY_FAST>(fCut / 44100.f, fReso);
        switch (params.filterType) {
            case Params::FilterType::LP: svf.Process<stmlib::FILTER_MODE_LOW_PASS>(out, out, size); break;
            case Params::FilterType::BP: svf.Process<stmlib::FILTER_MODE_BAND_PASS>(out, out, size); break;
            case Params::FilterType::HP: svf.Process<stmlib::FILTER_MODE_HIGH_PASS>(out, out, size); break;
            default: break;
        }
    }

    void RomplerVoiceMinimal::processBlock(float *out, const uint32_t size) {
        // fade first incoming buffer
        if (bufferStatus == BufferStatus::READFIRST) {
            readBufferFloat[0] = 0.01f * readBufferFloat[2];
            readBufferFloat[1] = 0.33f * readBufferFloat[2];
        }
        // TODO fade last buffer
        // apply anti-aliasing low-pass when downsampling, i.e. pitch up, not required if pitch down (upsampling)
        // > 0.1f to limit processing power at high pitch, has aliasing then
        // TODO anti aliasing could possibly completely be removed
        float fAntiAlias = 0.5f / phaseIncrement;
        if (fAntiAlias < 0.5f && fAntiAlias > 0.1f && params.filterType == Params::FilterType::NONE) {
            // the more cascades the better, but beware of cost
            dsps_biquad_gen_lpf_f32(coeffs_lpf, fAntiAlias, .5f);
            dsps_biquad_f32(&readBufferFloat[2], &readBufferFloat[2], readBufferLength, coeffs_lpf, w_lpf1);
        }

        // interpolate sample buffer from data
        // and apply AM
        for (int i = 0; i < size; i++) {
            // interpolate wave
            const float p = readBufferPhase;
            MAKE_INTEGRAL_FRACTIONAL(p);
            // use this to save more cpu, however correct zdelays to 2 instead of 4
            float x = InterpolateWaveLinear(readBufferFloat, p_integral, p_fractional);
            // apply AM
            if (!params.disableADEnvelopeVolume)  x *= ad.Process();
            out[i] = x;
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
    }

    void RomplerVoiceMinimal::Init(const float samplingRate) {
        fs = samplingRate;
        ad.SetSampleRate(fs);
        ad.SetModeExp();
        ad.SetLoop(false);
        bufferStatus = BufferStatus::STOPPED;
        pipoFlip = false;
        // Configure initial window from params in ms (clamped to shifter bounds)
        const float maxMs = (PitchShifterTD::MAX_WINDOW_SIZE * 1000.0f) / fs;
        const float minMs = 2.0f;
        float reqMs = std::clamp(params.timeStretchWindowMs, minMs, maxMs);
        int w = static_cast<int>(reqMs * (fs * 0.001f) + 0.5f);
        if (w < 8) w = 8;
        if (w > PitchShifterTD::MAX_WINDOW_SIZE) w = PitchShifterTD::MAX_WINDOW_SIZE;
        if (w & 1) ++w;
        shifter.setWindowSize(w);
        tsWindowMsApplied = reqMs;
    }

    RomplerVoiceMinimal::RomplerVoiceMinimal() {
        params = Params{};
        Reset();
    }

    RomplerVoiceMinimal::~RomplerVoiceMinimal() {
    }

    void RomplerVoiceMinimal::Reset() {
        ad.Reset();
        readBufferFloat[0] = readBufferFloat[1] = 0.f;
        preGate = false;
        bufferStatus = BufferStatus::STOPPED;
        readBufferPhase = 0.f;
        pipoFlip = false;
    }

}