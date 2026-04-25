/***************
TBD-16 — Macro/Preset System & PicoSeqRack

(c) 2025-2026 Per-Olov Jernberg (possan). https://possan.codes

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

dadamachines has a commercial license to use this code in the TBD-16 product.
Other commercial use requires a separate license agreement.

Provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "ctagSoundProcessorPicoSeqRack.hpp"
#include "braids/quantizer_scales.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_cpu.h"
#include "esp_timer.h"
// #include "freertos/FreeRTOS.h"

using namespace CTAG::SP;

// File-scope peak meters for the OLED Input / Output indicators. Defined
// here, declared extern in ctagSoundProcessorPicoSeqRack.hpp so SPManager
// can read them without needing access to the rack class internals.
volatile float g_peakInputTrack = 0.f;
volatile float g_peakSynthOnly  = 0.f;

// TODOs: fx return before compressor, stereo panning with delay -> when panned right, levels are lower, metallic sound of reverb.

#define maxFXSendLevelRev 1.5f

void ctagSoundProcessorPicoSeqRack::mixRenderOutputMono(float *source, float level, float pan, float fx1, float fx2) {
    float mL = (1.0f - pan);
    float mR = (1.0f + pan);

    CONSTRAIN(mL, 0.0f, 1.f);
    CONSTRAIN(mR, 0.0f, 1.f);

    mL *= level;
    mR *= level;

    float sL1 = mL * fx1;
    float sR1 = mR * fx1;
    float sL2 = mL * fx2;
    float sR2 = mR * fx2;

    for (int i = 0; i < bufSz; i++) {
        combined_out[i*2+0] += source[i] * mL;
        combined_out[i*2+1] += source[i] * mR;
        send1_out[i*2+0] += source[i] * sL1;
        send1_out[i*2+1] += source[i] * sR1;
        send2_out[i*2+0] += source[i] * sL2;
        send2_out[i*2+1] += source[i] * sR2;
    }
}

void ctagSoundProcessorPicoSeqRack::mixRenderOutputStereo(float *source, float level, float pan, float fx1, float fx2) {
    float mL = (1.0f - pan);
    float mR = (1.0f + pan);

    CONSTRAIN(mL, 0.0f, 1.f);
    CONSTRAIN(mR, 0.0f, 1.f);

    mL *= level;
    mR *= level;

    float sL1 = mL * fx1;
    float sR1 = mR * fx1;
    float sL2 = mL * fx2;
    float sR2 = mR * fx2;

    for (int i = 0; i < bufSz; i++) {
        combined_out[i*2+0] += source[i*2+0] * mL;
        combined_out[i*2+1] += source[i*2+1] * mR;
        send1_out[i*2+0] += source[i*2+0] * sL1;
        send1_out[i*2+1] += source[i*2+1] * sR1;
        send2_out[i*2+0] += source[i*2+0] * sL2;
        send2_out[i*2+1] += source[i*2+1] * sR2;
    }
}

void ctagSoundProcessorPicoSeqRack::preprocessFX1(const ProcessData& data) {
    // int global_bpm_lo2 = global_bpm_lo / 32;
    // int global_bpm_hi2 = global_bpm_hi / 32;
    // int scaledbpm = global_bpm_lo2 + (global_bpm_hi2 << 7);
    int scaledbpm = data.sequencer_tempo;
    if (scaledbpm != last_scaledbpm) {
        last_scaledbpm = scaledbpm;
        scaledbpm = scaledbpm / 10;
        if (scaledbpm < 32) scaledbpm = 32;
        // printf("Scaled BPM (%ld) set to %1.1f\n",
        //     data.sequencer_tempo,
        //     (float)scaledbpm/10.0f);
		last_msPerBeat = 60000.0f / ((float)(scaledbpm) / 10.0f);
    }

    // Sync knob branches the Time resolution:
    //   ON  → 12 musical divisors of the live msPerBeat (tempo-tracking).
    //   OFF → free mode, wire 0..127 → 0..2000 ms linear.
    bool bSync = fx1_sync;
    int wire = fx1_time_ms / 32;                     // atomic is 0..4064, knob is 0..127
    if (wire < 0) wire = 0;
    if (wire > 127) wire = 127;

    float dt_ms;
    if (bSync) {
        int idx = (wire * 12) / 128;
        if (idx < 0) idx = 0;
        if (idx > 11) idx = 11;
        // Per-beat fractions: quarter note = msPerBeat. 1/16 = msPerBeat/4 etc.
        const float divisor_factor[12] = {
            1.f/8.f,  1.f/6.f,  1.f/4.f,  3.f/8.f,   // 1/32, 1/16T, 1/16, 1/16D
            1.f/3.f,  1.f/2.f,  3.f/4.f,  2.f/3.f,   // 1/8T,  1/8,   1/8D, 1/4T
            1.f,      3.f/2.f,  2.f,      4.f        // 1/4,   1/4D,  1/2,  1/1
        };
        dt_ms = last_msPerBeat * divisor_factor[idx];
    } else {
        // Free mode: 0..2000 ms linear.
        dt_ms = (float)wire / 127.f * 2000.f;
    }
    float dt = dt_ms * 44.1f;                         // ms → samples
    CONSTRAIN(dt, 4.0f, 88200.f);
    int idt = (int)dt;
    if (idt != delaySamples) {
        delaySamples = idt;
    }

    MK_FLT_PAR_ABS_NOCV(fBase, fx1_base, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(fWidth, fx1_width, 4095.f, 1.f)
    bool bSyncTrig {false};
    // if(trig_fx1_sync != -1) bSyncTrig = data.trig[trig_fx1_sync] == 1 ? false : true;
    // if(!bSync){
    // if(cv_fx1_time_ms != -1) fDelayTime = fabsf(data.cv[cv_fx1_time_ms]) * 2000.f;
    // }

    fBase = 20.f * stmlib::SemitonesToRatio(fBase * 120.f);
    fWidth = 20.f * stmlib::SemitonesToRatio(fWidth * 120.f);
    CONSTRAIN(fBase, 20.f, 20000.f)
    CONSTRAIN(fWidth, 50.f, 20000.f)
    float hp_cut = fBase;
    float lp_cut = fBase + fWidth;
    CONSTRAIN(lp_cut, 20.f, 20000.f)
    CONSTRAIN(hp_cut, 20.f, 20000.f)
    lp_l.set_f<stmlib::FREQUENCY_ACCURATE>(lp_cut / 44100.f);
    hp_l.set_f<stmlib::FREQUENCY_ACCURATE>(hp_cut / 44100.f);
    lp_r.copy_f(lp_l);
    hp_r.copy_f(hp_l);

    // Delay-input HP corner — independent of the feedback-path HP. Log
    // sweep 20 Hz..~2 kHz via 80 semitones (≈100×). Wire 0 → 20 Hz is
    // effectively bypassed for musical content above 30 Hz.
    MK_FLT_PAR_ABS_NOCV(fInputHpNorm, fx1_input_hp, 4095.f, 1.f)
    float dly_in_hp = 20.f * stmlib::SemitonesToRatio(fInputHpNorm * 80.f);
    CONSTRAIN(dly_in_hp, 20.f, 20000.f)
    dly_input_hp_l.set_f<stmlib::FREQUENCY_ACCURATE>(dly_in_hp / 44100.f);
    dly_input_hp_r.copy_f(dly_input_hp_l);

    // sync mechanism
    // if(bSyncTrig != pre_sync){
    //     pre_sync = bSyncTrig;
    //     if(bSyncTrig && bSync){
    //         int delta = timer - pre_timer;
    //         if(std::abs(delta) > 1){
    //             fDelayTime = static_cast<float>(timer) * 32.f / 44.1f;
    //         }
    //         pre_timer = timer;
    //         timer = 0;
    //     }
    // }
    timer++;
}

void ctagSoundProcessorPicoSeqRack::preprocessFX2(const ProcessData& data) {
    MK_FLT_PAR_ABS_NOCV(fRevTime, fx2_time, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(fReverbLPF, fx2_lp, 4095.f, 1.f)
    // fx2_diffuse → reverb.set_diffusion (was hardcoded 0.7). 0.0 ≈ slap
    // echo, ~0.9 ≈ dense diffuse tail.
    MK_FLT_PAR_ABS_NOCV(fDiffuse, fx2_diffuse, 4095.f, 0.95f)
    reverb.set_time(fRevTime);
    reverb.set_lp(fReverbLPF);
    reverb.set_diffusion(fDiffuse);
    // fx2_modulation 0..2× scaler applied to the LFO frequencies. Bases
    // 0.5/0.3 Hz match Init() (reverb.h:45-46). Wire 0 freezes both LFOs.
    MK_FLT_PAR_ABS_NOCV(fMod, fx2_modulation, 4095.f, 2.f)
    reverb.set_lfo1_freq(0.5f * fMod);
    reverb.set_lfo2_freq(0.3f * fMod);
    // fx2_input_gain / fx2_tank_level promote the previously-hardcoded
    // set_input_gain(0.5) and set_amount(1.0). Default wires (64 / 127)
    // preserve those legacy values within float rounding.
    MK_FLT_PAR_ABS_NOCV(fInGain, fx2_input_gain, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(fTankLvl, fx2_tank_level, 4095.f, 1.f)
    reverb.set_input_gain(fInGain);
    reverb.set_amount(fTankLvl);
    // Reverb-input HP shelf, applied per-sample before the tank in
    // renderMasterOutput. Same 20 Hz..~2 kHz log sweep as fx1_input_hp.
    MK_FLT_PAR_ABS_NOCV(fRevHpNorm, fx2_hp, 4095.f, 1.f)
    float rev_hp = 20.f * stmlib::SemitonesToRatio(fRevHpNorm * 80.f);
    CONSTRAIN(rev_hp, 20.f, 20000.f)
    rev_hp_l.set_f<stmlib::FREQUENCY_ACCURATE>(rev_hp / 44100.f);
    rev_hp_r.copy_f(rev_hp_l);
}

void ctagSoundProcessorPicoSeqRack::preprocessMaster(const ProcessData& data) {
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(fCompThresdB, c_thres, 4095.f, -80.f, 0.f)
    sumCompressor.setThresh(fCompThresdB);
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(fCompAtk, c_atk, 4095.f, 0.3f, 30.f)
    sumCompressor.setAttack(fCompAtk);
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(fCompRel, c_rel, 4095.f, 40.f, 2000.f)
    sumCompressor.setRelease(fCompRel);
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(fCompRatio, c_ratio, 4095.f, 0.0001f, 1.25f)
    sumCompressor.setRatio(fCompRatio);
}

void ctagSoundProcessorPicoSeqRack::renderMasterOutput(const ProcessData& data) {
    // delay
    MK_BOOL_PAR_NOCV(bFreeze, fx1_freeze)
    MK_FLT_PAR_ABS_NOCV(fDelayStereoWidth, fx1_st_width, 4095.f, 1.f)
    // Concave taper on Width: lower half of travel stays near-mono, full
    // stereo expansion is concentrated in the upper half. k=0.06 = the
    // upstream "aggressive" preset. (Commit 38f2975e.)
    fDelayStereoWidth = HELPERS::FastConcaveTransfer(fDelayStereoWidth, 0.06f);
    MK_FLT_PAR_ABS_NOCV(fDelayReverbSend, fx1_fx_send, 4095.f, maxFXSendLevelRev)
    fDelayReverbSend *= fDelayReverbSend;
    // Feedback ceiling 1.2× pairs with the FastConcaveTransfer Width curve
    // so the cross-feed dominates the upper half of knob travel before
    // feedback-only buildup can run away. (Upstream commit 38f2975e.)
    MK_FLT_PAR_ABS_NOCV(fFeedback, fx1_feedback, 4095.f, 1.2f)
    MK_FLT_PAR_ABS_NOCV(fDelayAmount, fx1_amount, 4095.f, 2.f)

    // reverb
    MK_FLT_PAR_ABS_NOCV(fRevAmount, fx2_amount, 4095.f, 2.f)

    // sum compressor
    float buf_fx1_l[BUF_SZ], buf_fx1_r[BUF_SZ], buf_fx2[BUF_SZ];
    MK_BOOL_PAR_NOCV(bTapeDigital, fx1_tape_digital)
    MK_BOOL_PAR_NOCV(bSideChainLPF, c_lpf)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(fCompMUPGain, c_gain, 4095.f, 0.f, 60.f) // in dB
    if (fCompMUPGain != fCompMUPGain_pre){
        fCompMUPGain = chunkware_simple::dB2lin(fCompMUPGain);
        fCompMUPGain_pre = fCompMUPGain;
    }
    MK_FLT_PAR_ABS_PAN_NOCV(fCompMix, c_mix, 4095.f, 1.f)
    // CCs 67/68 (c_dly_level / c_rev_level) retired — they had no DSP
    // referent. FX returns are scaled by fRevAmount / fDelayAmount at the
    // end of renderMasterOutput().

    // overall mix
    MK_FLT_PAR_ABS_NOCV(fMixLevel, sum_lev, 4095.f, 3.f)
    fMixLevel *= fMixLevel;

    // Render final buffer
    for (int i = 0; i < bufSz; i++){
        float fVal_l = combined_out[i * 2 + 0];
        float fVal_r = combined_out[i * 2 + 1];

        // FX1 models
        buf_fx1_l[i] = send1_out[i * 2 + 0];
        buf_fx1_r[i] = send1_out[i * 2 + 1];

        // FX2 models, reverb is mono in stereo out, but input buffer is stereo
        buf_fx2[i] = send2_out[i * 2 + 0];

        float dry_l = fVal_l;
        float dry_r = fVal_r;
        if (bSideChainLPF){
            ONE_POLE(side_l, fVal_l, 0.0005f);
            ONE_POLE(side_r, fVal_r, 0.0005f);
        }
        else{
            side_l = fVal_l;
            side_r = fVal_r;
        }
        side_l = fabsf(side_l);
        side_r = fabsf(side_r);
        float side = std::max(side_l, side_r);
        sumCompressor.process(fVal_l, fVal_r, side);
        fVal_l = fVal_l * fCompMUPGain * fCompMix + dry_l * (1.f - fCompMix);
        fVal_r = fVal_r * fCompMUPGain * fCompMix + dry_r * (1.f - fCompMix);
        data.buf[i * 2] = fVal_l * fMixLevel;
        data.buf[i * 2 + 1] = fVal_r * fMixLevel;
    }

    // fx buffers
    float dly_buf_l[BUF_SZ], dly_buf_r[BUF_SZ];
    float rev_buf_l[BUF_SZ], rev_buf_r[BUF_SZ];

    // delay
    // CONSTRAIN(delaySamples, 4.0f, 88200.f);
    float ofs = delaySamples;
    if(fabsf(ofs - delayOffset) < 16) ofs = delayOffset;
    // if (obs < 16) {
    //     obs = 16;
    // }
    for(int i=0; i<bufSz; i++){
        // Calculate the delay offset in samples
        if(delayOffset != ofs){
            if(bTapeDigital){
                if(ofs != delayOffset){
                    duck = 1.f;
                }
                delayOffset = ofs;
            } else {
                float temp = delayOffset;
                delayOffset = ONE_POLE(temp, ofs, 0.0001f);
            }
            readPos = static_cast<float>(writeIndex) - delayOffset;
            if(readPos < 0.f) readPos += float(delayBufferSizeMax);
            if(readPos >= float(delayBufferSizeMax)) readPos -= float(delayBufferSizeMax);
        }

        float inputSample_l = buf_fx1_l[i];
        float inputSample_r = buf_fx1_r[i];
        // HP applied to dry input before the delay loop, independent of
        // the feedback-path HP/LP below.
        inputSample_l = dly_input_hp_l.Process<stmlib::FILTER_MODE_HIGH_PASS>(inputSample_l);
        inputSample_r = dly_input_hp_r.Process<stmlib::FILTER_MODE_HIGH_PASS>(inputSample_r);
        float outputSample_l, outputSample_r;

        outputSample_l = HELPERS::InterpolateWaveLinearWrap(delayBuffer_l, readPos, delayBufferSizeMax);
        outputSample_r = HELPERS::InterpolateWaveLinearWrap(delayBuffer_r, readPos, delayBufferSizeMax);
        readPos += 1.f;
        readPos > float(delayBufferSizeMax) ? readPos -= float(delayBufferSizeMax) : readPos;

        float temp = duck;
        duck = ONE_POLE(temp, 0.f, 0.35f)
        outputSample_l = outputSample_l * (1.f - duck);
        outputSample_r = outputSample_r * (1.f - duck);
        // Write the input sample to the delay buffer
        float out_l, out_r;
        if(!bFreeze){
            out_l = inputSample_l + fFeedback * ((1.f - fDelayStereoWidth) * outputSample_l + fDelayStereoWidth * outputSample_r);
            out_l = lp_l.Process<stmlib::FILTER_MODE_LOW_PASS>(out_l);
            out_l = hp_l.Process<stmlib::FILTER_MODE_HIGH_PASS>(out_l);
            out_r = (1.f - fDelayStereoWidth) * inputSample_r + fFeedback * ((1.f - fDelayStereoWidth) * outputSample_r + fDelayStereoWidth * outputSample_l);
            out_r = lp_r.Process<stmlib::FILTER_MODE_LOW_PASS>(out_r);
            out_r = hp_r.Process<stmlib::FILTER_MODE_HIGH_PASS>(out_r);
        }
        else{
            out_l = ((1.f - fDelayStereoWidth) * outputSample_l + fDelayStereoWidth * outputSample_r);
            out_r = ((1.f - fDelayStereoWidth) * outputSample_r + fDelayStereoWidth * outputSample_l);
        }

        delayBuffer_l[writeIndex] = stmlib::SoftLimit(out_l);
        delayBuffer_r[writeIndex] = stmlib::SoftLimit(out_r);
        writeIndex = (writeIndex + 1) % delayBufferSizeMax;

        // Mix the dry (input) and wet (delayed) signal
        dly_buf_l[i] = outputSample_l;
        dly_buf_r[i] = outputSample_r;
        rev_buf_l[i] = buf_fx2[i] + dly_buf_l[i] * fDelayReverbSend;
        rev_buf_r[i] = buf_fx2[i] + dly_buf_r[i] * fDelayReverbSend;
    }

    // Pre-delay before the reverb tank. fx2_predelay maps wire 0..4064 →
    // 0..200 ms via a mono ring buffer (the tank sums L+R internally).
    // Bypassed when < 2 samples to avoid one-sample latency at zero.
    {
        int predly_raw = fx2_predelay;        // 0..4064
        if (predly_raw < 0) predly_raw = 0;
        if (predly_raw > 4095) predly_raw = 4095;
        int preDelaySamples = (predly_raw * 200 * 441) / (4095 * 10);  // ms × 44.1
        if (preDelaySamples < 2) {
            // bypass — rev_buf_l/r already carries the FX2 bus; nothing to do.
        } else {
            if (preDelaySamples >= preDelayBufSize) preDelaySamples = preDelayBufSize - 1;
            for (int i = 0; i < bufSz; i++) {
                // Mono sum of the current FX2 contribution (delay + direct FX2 send)
                float preIn = 0.5f * (rev_buf_l[i] + rev_buf_r[i]);
                preDelayBuf[preDelayWriteIdx] = preIn;
                int readIdx = preDelayWriteIdx - preDelaySamples;
                if (readIdx < 0) readIdx += preDelayBufSize;
                float preOut = preDelayBuf[readIdx];
                // Replace both channels with the pre-delayed mono; reverb tank
                // will take it from here (it sums L+R on input anyway).
                rev_buf_l[i] = preOut;
                rev_buf_r[i] = preOut;
                preDelayWriteIdx++;
                if (preDelayWriteIdx >= preDelayBufSize) preDelayWriteIdx = 0;
            }
        }
    }

    // HP shelf on the reverb input, applied per-sample before the tank.
    // Catches FX2 sends, the FX1→FX2 cross-send, and the pre-delayed
    // signal when fx2_predelay > 1. Independent of the in-loop LP fx2_lp.
    for (int i = 0; i < bufSz; i++) {
        rev_buf_l[i] = rev_hp_l.Process<stmlib::FILTER_MODE_HIGH_PASS>(rev_buf_l[i]);
        rev_buf_r[i] = rev_hp_r.Process<stmlib::FILTER_MODE_HIGH_PASS>(rev_buf_r[i]);
    }

    // reverb
    reverb.Process(rev_buf_l, rev_buf_r, bufSz);

    // add fx to sum
    fRevAmount *= fRevAmount;
    fDelayAmount *= fDelayAmount;
    for (int i = 0; i < bufSz; i++) {
        data.buf[i * 2] += rev_buf_l[i] * fRevAmount + dly_buf_l[i] * fDelayAmount;
        data.buf[i * 2 + 1] += rev_buf_r[i] * fRevAmount + dly_buf_r[i] * fDelayAmount;
    }

    // sum_drive: variable-depth SoftLimit soft saturation on the master bus.
    // Skipped entirely at wire 0 to keep the default path bit-identical to
    // the pre-Drive code (no regression on existing material). At Drive>0
    // the cubic clipper progressively saturates; 1/sqrt(drive) makeup
    // partially compensates the perceived loudness bump.
    MK_FLT_PAR_ABS_NOCV(fDriveNorm, sum_drive, 4095.f, 1.f)
    if (fDriveNorm > 1e-4f) {
        float fDrive = 1.f + fDriveNorm * 3.f;          // 1× .. 4×
        float fDriveMakeup = 1.f / sqrtf(fDrive);
        for (int i = 0; i < bufSz; i++) {
            data.buf[i * 2 + 0] = stmlib::SoftLimit(data.buf[i * 2 + 0] * fDrive) * fDriveMakeup;
            data.buf[i * 2 + 1] = stmlib::SoftLimit(data.buf[i * 2 + 1] * fDrive) * fDriveMakeup;
        }
    }
}

void ctagSoundProcessorPicoSeqRack::Process(const ProcessData& data){
    framecounter ++;

    // TODO: process midi in data.midibytes here
    if (data.midi_bytes_length > 0) {
        parseIncomingMidiMessages(data.midi_bytes, data.midi_bytes_length);
    }

    memcpy(audio_in, data.buf, bufSz * 2 * sizeof(float));
    std::fill_n(combined_out, bufSz * 2, 0.f);
    std::fill_n(send1_out, bufSz * 2, 0.f);
    std::fill_n(send2_out, bufSz * 2, 0.f);

	struct PicoSeqRackProcessData idata;
    idata.firstNonWtSlice = sampleRom.GetFirstNonWaveTableSlice();
    idata.sampleRom = &sampleRom;
    idata.tempo = data.sequencer_tempo;
    idata.quantum = data.sequencer_quantum;
    idata.msPerBeat = last_msPerBeat;
    idata.inputbuffer = audio_in;

    // process input first

    int64_t T2 = esp_timer_get_time();
    int64_t Tstart = T2;
    ch16.PreProcess(idata);
    if (ch16.enabled) {
        ch16_in.Process(idata); // - it does nothing...
        if (ch16_in.enabled) {
            mixRenderOutputStereo(ch16_in.out, ch16.level, ch16.pan, ch16.send1, ch16.send2);
        }
    }
    // Snapshot combined_out right after ch16 mixes — this is the
    // INPUT TRACK contribution to the master bus, before any other
    // tracks render. Used by the OLED Input/Output meters: input
    // meter = peak of this snapshot, output meter = peak of (final
    // combined_out − this snapshot) so synth tracks alone register
    // on the output meter and the input never bleeds into it.
    {
        float p = 0.f;
        for (int i = 0; i < bufSz * 2; i++) {
            ch16_combined_snapshot[i] = combined_out[i];
            float a = combined_out[i] < 0 ? -combined_out[i] : combined_out[i];
            if (a > p) p = a;
        }
        float prev = g_peakInputTrack;
        g_peakInputTrack = (p > prev) ? p : (0.85f * prev + 0.15f * p);
    }
    std::fill_n(data.buf, bufSz * 2, 0.f);

    // int64_t T = esp_timer_get_time();
    // ch16_render_time = T - T2;

    ch1.PreProcess(idata);
    if (ch1.enabled) {
        ch1_db.Process(idata);
        if (ch1_db.enabled) {
            mixRenderOutputMono(ch1_db.out, ch1.level, ch1.pan, ch1.send1, ch1.send2);
        }

        ch1_ab.Process(idata);
        if (ch1_ab.enabled) {
            mixRenderOutputMono(ch1_ab.out, ch1.level, ch1.pan, ch1.send1, ch1.send2);
        }

        ch1_smp.track_length = ch1.track_length;
        ch1_smp.Process(idata);
        if (ch1_smp.enabled) {
            mixRenderOutputMono(ch1_smp.s1_out, ch1.level, ch1.pan, ch1.send1, ch1.send2);
        }
    }

    // T2 = esp_timer_get_time();
    // ch1_render_time = T2 - T;

    ch2.PreProcess(idata);
    if (ch2.enabled) {
        ch2_fmb1.Process(idata);
        if (ch2_fmb1.enabled) {
            mixRenderOutputMono(ch2_fmb1.out, ch2.level, ch2.pan, ch2.send1, ch2.send2);
        }

        ch2_smp.track_length = ch2.track_length;
        ch2_smp.Process(idata);
        if (ch2_smp.enabled) {
            mixRenderOutputMono(ch2_smp.s1_out, ch2.level, ch2.pan, ch2.send1, ch2.send2);
        }
    }

    // T = esp_timer_get_time();
    // ch2_render_time = T - T2;

    ch3.PreProcess(idata);
    if (ch3.enabled) {
        ch3_ds.Process(idata);
        if (ch3_ds.enabled) {
            mixRenderOutputMono(ch3_ds.out, ch3.level, ch3.pan, ch3.send1, ch3.send2);
        }

        ch3_as.Process(idata);
        if (ch3_as.enabled) {
            mixRenderOutputMono(ch3_as.out, ch3.level, ch3.pan, ch3.send1, ch3.send2);
        }

        ch3_smp.track_length = ch3.track_length;
        ch3_smp.Process(idata);
        if (ch3_smp.enabled) {
            mixRenderOutputMono(ch3_smp.s1_out, ch3.level, ch3.pan, ch3.send1, ch3.send2);
        }
    }

    // T2 = esp_timer_get_time();
    // ch3_render_time = T2 - T;

    ch4.PreProcess(idata);
    if (ch4.enabled) {
        ch4_hh1.Process(idata);
        if (ch4_hh1.enabled) {
            mixRenderOutputMono(ch4_hh1.out, ch4.level, ch4.pan, ch4.send1, ch4.send2);
        }

        ch4_hh2.Process(idata);
        if (ch4_hh2.enabled) {
            mixRenderOutputMono(ch4_hh2.out, ch4.level, ch4.pan, ch4.send1, ch4.send2);
        }

        ch4_smp.track_length = ch4.track_length;
        ch4_smp.Process(idata);
        if (ch4_smp.enabled) {
            mixRenderOutputMono(ch4_smp.s1_out, ch4.level, ch4.pan, ch4.send1, ch4.send2);
        }
    }

    // T = esp_timer_get_time();
    // ch4_render_time = T - T2;

    ch5.PreProcess(idata);
    if (ch5.enabled) {
        ch5_rs.Process(idata);
        if (ch5_rs.enabled) {
            mixRenderOutputMono(ch5_rs.rs_out, ch5.level, ch5.pan, ch5.send1, ch5.send2);
        }

        ch5_smp.track_length = ch5.track_length;
        ch5_smp.Process(idata);
        if (ch5_smp.enabled) {
            mixRenderOutputMono(ch5_smp.s1_out, ch5.level, ch5.pan, ch5.send1, ch5.send2);
        }
    }

    // T2 = esp_timer_get_time();
    // ch5_render_time = T2 - T;

    ch6.PreProcess(idata);
    if (ch6.enabled) {
        ch6_cl.Process(idata);
        if (ch6_cl.enabled) {
            mixRenderOutputMono(ch6_cl.out, ch6.level, ch6.pan, ch6.send1, ch6.send2);
        }

        ch6_smp.track_length = ch6.track_length;
        ch6_smp.Process(idata);
        if (ch6_smp.enabled) {
            mixRenderOutputMono(ch6_smp.s1_out, ch6.level, ch6.pan, ch6.send1, ch6.send2);
        }
    }

    // T = esp_timer_get_time();
    // ch6_render_time = T - T2;

    ch7.PreProcess(idata);
    if (ch7.enabled) {
        ch7_smp.track_length = ch7.track_length;
        ch7_smp.Process(idata);
        if (ch7_smp.enabled) {
            mixRenderOutputMono(ch7_smp.s1_out, ch7.level, ch7.pan, ch7.send1, ch7.send2);
        }
    }

    // T2 = esp_timer_get_time();
    // ch7_render_time = T2 - T;

    ch8.PreProcess(idata);
    if (ch8.enabled) {
        ch8_smp.track_length = ch8.track_length;
        ch8_smp.Process(idata);
        if (ch8_smp.enabled) {
            mixRenderOutputMono(ch8_smp.s1_out, ch8.level, ch8.pan, ch8.send1, ch8.send2);
        }
    }

    // T = esp_timer_get_time();
    // ch8_render_time = T - T2;

    ch9.PreProcess(idata);
    if (ch9.enabled) {
        ch9_td3.Process(idata);
        if (ch9_td3.enabled) {
            mixRenderOutputMono(ch9_td3.td3_out, ch9.level, ch9.pan, ch9.send1, ch9.send2);
        }

        ch9_smp.track_length = ch9.track_length;
        ch9_smp.Process(idata);
        if (ch9_smp.enabled) {
            mixRenderOutputMono(ch9_smp.s1_out, ch9.level, ch9.pan, ch9.send1, ch9.send2);
        }
    }

    // T2 = esp_timer_get_time();
    // ch9_render_time = T2 - T;

    ch10.PreProcess(idata);
    if (ch10.enabled) {
        ch10_td3.Process(idata);
        if (ch10_td3.enabled) {
            mixRenderOutputMono(ch10_td3.td3_out, ch10.level, ch10.pan, ch10.send1, ch10.send2);
        }

        ch10_smp.track_length = ch10.track_length;
        ch10_smp.Process(idata);
        if (ch10_smp.enabled) {
            mixRenderOutputMono(ch10_smp.s1_out, ch10.level, ch10.pan, ch10.send1, ch10.send2);
        }
    }

    // T = esp_timer_get_time();
    // ch10_render_time = T - T2;

    ch11.PreProcess(idata);
    if (ch11.enabled) {
        ch11_mo.Process(idata);
        if (ch11_mo.enabled) {
            mixRenderOutputMono(ch11_mo.mo_out, ch11.level, ch11.pan, ch11.send1, ch11.send2);
        }

        ch11_smp.track_length = ch11.track_length;
        ch11_smp.Process(idata);
        if (ch11_smp.enabled) {
            mixRenderOutputMono(ch11_smp.s1_out, ch11.level, ch11.pan, ch11.send1, ch11.send2);
        }
    }

    // T2 = esp_timer_get_time();
    // ch11_render_time = T2 - T;

    ch12.PreProcess(idata);
    if (ch12.enabled) {
        ch12_wtosc.Process(idata);
        if (ch12_wtosc.enabled) {
            mixRenderOutputMono(ch12_wtosc.out, ch12.level, ch12.pan, ch12.send1, ch12.send2);
        }

        ch12_mo.Process(idata);
        if (ch12_mo.enabled) {
            mixRenderOutputMono(ch12_mo.mo_out, ch12.level, ch12.pan, ch12.send1, ch12.send2);
        }

        ch12_smp.track_length = ch12.track_length;
        ch12_smp.Process(idata);
        if (ch12_smp.enabled) {
            mixRenderOutputMono(ch12_smp.s1_out, ch12.level, ch12.pan, ch12.send1, ch12.send2);
        }
    }

    // T = esp_timer_get_time();
    // ch12_render_time = T - T2;

    ch13.PreProcess(idata);
    if (ch13.enabled) {
        ch13_smp.track_length = ch13.track_length;
        ch13_smp.Process(idata);
        if (ch13_smp.enabled) {
            mixRenderOutputMono(ch13_smp.s1_out, ch13.level, ch13.pan, ch13.send1, ch13.send2);
        }
    }

    // T2 = esp_timer_get_time();
    // ch13_render_time = T2 - T;

    ch14.PreProcess(idata);
    if (ch14.enabled) {
        ch14_smp.track_length = ch14.track_length;
        ch14_smp.Process(idata);
        if (ch14_smp.enabled) {
            mixRenderOutputMono(ch14_smp.s1_out, ch14.level, ch14.pan, ch14.send1, ch14.send2);
        }
    }

    // T = esp_timer_get_time();
    // ch14_render_time = T - T2;

    ch15.PreProcess(idata);
    if (ch15.enabled) {
        ch15_pp.Process(idata);
        if (ch15_pp.enabled) {
            mixRenderOutputStereo(ch15_pp.pp_out_stereo, ch15.level, ch15.pan, ch15.send1, ch15.send2);
        }

        ch15_smp.track_length = ch15.track_length;
        ch15_smp.Process(idata);
        if (ch15_smp.enabled) {
            mixRenderOutputMono(ch15_smp.s1_out, ch15.level, ch15.pan, ch15.send1, ch15.send2);
        }
    }

    // T2 = esp_timer_get_time();
    // ch15_render_time = T2 - T;

    // Synth-only peak — combined_out at this point holds (ch16 + every
    // synth track). Subtract the ch16 snapshot taken right after ch16
    // mixed; what remains is purely the synth tracks' contribution to
    // the bus. Used by the OLED Output meter so input audio routed
    // through ch16 never bleeds into it.
    {
        float p = 0.f;
        for (int i = 0; i < bufSz * 2; i++) {
            float diff = combined_out[i] - ch16_combined_snapshot[i];
            float a = diff < 0 ? -diff : diff;
            if (a > p) p = a;
        }
        float prev = g_peakSynthOnly;
        g_peakSynthOnly = (p > prev) ? p : (0.85f * prev + 0.15f * p);
    }

    // Process effects
    preprocessFX1(data); // delay

    // T = esp_timer_get_time();
    // fx_delay_render_time = T - T2;

    preprocessFX2(data); // reverb

    // T2 = esp_timer_get_time();
    // fx_reverb_render_time = T2 - T;

    preprocessMaster(data); // sum compressor

    // T = esp_timer_get_time();
    // fx_master_render_time = T - T2;

    MK_BOOL_PAR_NOCV(bSumMute, sum_mute)
    if (bSumMute){
        memset(data.buf, 0, bufSz * 2 * sizeof(float));
        return;
    }

    // T = esp_timer_get_time();
    // int64_t Ttotal = T - Tstart;

    renderMasterOutput(data);

    // if audio somehow ends up in NaN, reset buffers...
    if (data.buf[0] != data.buf[0]) {
        std::fill_n(data.buf, bufSz * 2, 0.f);
        std::fill_n(combined_out, bufSz * 2, 0.f);
        std::fill_n(send1_out, bufSz * 2, 0.f);
        std::fill_n(send2_out, bufSz * 2, 0.f);
        std::fill_n(delayBuffer_l, delayBufferSizeMax, 0.f);
        std::fill_n(delayBuffer_r, delayBufferSizeMax, 0.f);
        std::fill_n(reverbBuffer, 32768, 0.f);
        std::fill_n(preDelayBuf, preDelayBufSize, 0.f);
    }

    // if (framecounter % 5000 == 0) {
    //     printf("PicoSeqRack CPU time %d uS\n", (int)Ttotal);
        // printf("PicoSeqRack CPU times (us): Ch1:%d Ch2:%d Ch3:%d Ch4:%d Ch5:%d Ch6:%d Ch7:%d Ch8:%d FX1:%d FX2:%d Master:%d\n",
        // (int)ch1_render_time, (int)ch2_render_time, (int)ch3_render_time, (int)ch4_render_time,
        // (int)ch5_render_time, (int)ch6_render_time, (int)ch7_render_time, (int)ch8_render_time,
        // (int)fx_delay_render_time, (int)fx_reverb_render_time, (int)fx_master_render_time);
        // } else if (framecounter % 5000 == 2500) {
        // printf("PicoSeqRack CPU times (us): Ch9:%d Ch10:%d Ch11:%d Ch12:%d Ch13:%d Ch14:%d Ch15:%d Ch16:%d FX1:%d FX2:%d Master:%d\n",
        // (int)ch9_render_time, (int)ch10_render_time, (int)ch11_render_time, (int)ch12_render_time,
        // (int)ch13_render_time, (int)ch14_render_time, (int)ch15_render_time, (int)ch16_render_time,
        // (int)fx_delay_render_time, (int)fx_reverb_render_time, (int)fx_master_render_time);
    // }
}

void ctagSoundProcessorPicoSeqRack::registerParamAndCC(const PickSeqRackInitData *initdata, const char *suffix, int cc, function<DrumRackParameterSetter> setter) {
    // string fullId = string(initdata->prefix) + string(suffix);
    uint16_t key = CC_TO_MAP_KEY(initdata->midi_channel, initdata->cc_base + cc);
    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Registering param %s//%s, key %d for CC %d+%d", 
    //     string(initdata->prefix).c_str(), suffix, key, initdata->cc_base, cc);
    pMapParCC.emplace(key, PsramVector<function<void(const int)>>());
    pMapParCC[key].push_back(setter);
}

void ctagSoundProcessorPicoSeqRack::handleMidiControlChange(const uint8_t channel, const uint8_t control, const uint8_t value) {
    int32_t cv_value = ((int32_t)value * 4096) / 128;
    int key = CC_TO_MAP_KEY(channel, control);

    auto it = pMapParCC.find(key);
    if (it != pMapParCC.end()) {
        // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "MIDI: CC %d, %d, %d (cv %d) (Set)", channel, control, value, cv_value);
        for(auto& listener : it->second){
            listener(cv_value);
        }
    // } else {
    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "MIDI: CC %d, %d, %d (Unhandled)", channel, control, value);
    }
};

void ctagSoundProcessorPicoSeqRack::handleMidiControlChangeNRPM(const uint8_t channel, const uint8_t control, const uint16_t value) {
    int32_t cv_value = ((int32_t)value * 4096) / 16384;
    int key = CC_TO_MAP_KEY(channel, control);

    auto it = pMapParCC.find(key);
    if (it != pMapParCC.end()) {
        // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "MIDI: nrpm CC %d, %d, %d (cv %d) (Set)", channel, control, value, cv_value);
        for(auto& listener : it->second){
            listener(cv_value);
        }
    // } else {
    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "MIDI: nrpm CC %d, %d, %d (Unhandled)", channel, control, value);
    }
};

static void dumpMemoryUsage() {
    uint32_t freeSize = esp_get_free_heap_size();
	printf("The available total size of heap:%" PRIu32 "\n", freeSize);

	printf("\tDescription\tInternal\tSPIRAM\n");
	printf("Current Free Memory\t%d\t\t%d\n",
			heap_caps_get_free_size(MALLOC_CAP_8BIT) - heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
			heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
	printf("Largest Free Block\t%d\t\t%d\n",
			heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
			heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
	printf("Min. Ever Free Size\t%d\t\t%d\n",
			heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
			heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM));
}

void ctagSoundProcessorPicoSeqRack::Init(std::size_t blockSize, void* blockPtr){
    // construct internal data model

	printf("ctagSoundProcessorPicoSeqRack::Init(%zu, %x)\n", blockSize, (uintptr_t) blockPtr);

    dumpMemoryUsage();

    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Before know yourself");
    knowYourself();
    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "After know yourself");

    framecounter = 0;

    PickSeqRackInitData dri;
    dri.rack = this;

    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Dummy -2");

    dri.track_index = 0;
    dri.midi_channel = 9;
    dri.cc_base = 0;
    dri.prefix = "ch1_"; ch1.Init(&dri);
    dri.prefix = "ch1_db_"; ch1_db.Init(&dri);
    dri.prefix = "ch1_ab_"; ch1_ab.Init(&dri);
    dri.prefix = "ch1_smp_"; ch1_smp.Init(&dri);
    ch1_render_time = 0;

    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Dummy 0");
    // dumpMemoryUsage();

    dri.track_index = 1;
    dri.midi_channel = 9;
    dri.cc_base = 40;
    dri.prefix = "ch2_"; ch2.Init(&dri);
    dri.prefix = "ch2_fmb1_"; ch2_fmb1.Init(&dri);
    dri.prefix = "ch2_smp_"; ch2_smp.Init(&dri);
    ch2_render_time = 0;

    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Dummy 1");
    // dumpMemoryUsage();

    dri.track_index = 2;
    dri.midi_channel = 9;
    dri.cc_base = 80;
    dri.prefix = "ch3_"; ch3.Init(&dri);
    dri.prefix = "ch3_ds_"; ch3_ds.Init(&dri);
    dri.prefix = "ch3_as_"; ch3_as.Init(&dri);
    dri.prefix = "ch3_smp_"; ch3_smp.Init(&dri);
    ch3_render_time = 0;

    // dumpMemoryUsage();

    dri.track_index = 3;
    dri.midi_channel = 10;
    dri.cc_base = 0;
    dri.prefix = "ch4_"; ch4.Init(&dri);
    dri.prefix = "ch4_hh1_"; ch4_hh1.Init(&dri);
    dri.prefix = "ch4_hh2_"; ch4_hh2.Init(&dri);
    dri.prefix = "ch4_smp_"; ch4_smp.Init(&dri);
    ch4_render_time = 0;

    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Dummy 2");
    // dumpMemoryUsage();

    dri.track_index = 4;
    dri.midi_channel = 10;
    dri.cc_base = 40;
    dri.prefix = "ch5_"; ch5.Init(&dri);
    dri.prefix = "ch5_rs_"; ch5_rs.Init(&dri);
    dri.prefix = "ch5_smp_"; ch5_smp.Init(&dri);
    ch5_render_time = 0;

    // dumpMemoryUsage();

    dri.track_index = 5;
    dri.midi_channel = 10;
    dri.cc_base = 80;
    dri.prefix = "ch6_"; ch6.Init(&dri);
    dri.prefix = "ch6_cl_"; ch6_cl.Init(&dri);
    dri.prefix = "ch6_smp_"; ch6_smp.Init(&dri);
    ch6_render_time = 0;

    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Dummy 3");
    // dumpMemoryUsage();

    dri.track_index = 6;
    dri.midi_channel = 11;
    dri.cc_base = 0;
    dri.prefix = "ch7_"; ch7.Init(&dri);
    dri.prefix = "ch7_smp_"; ch7_smp.Init(&dri);
    ch7_render_time = 0;

    // dumpMemoryUsage();

    dri.track_index = 7;
    dri.midi_channel = 11;
    dri.cc_base = 40;
    dri.prefix = "ch8_"; ch8.Init(&dri);
    dri.prefix = "ch8_smp_"; ch8_smp.Init(&dri);
    ch8_render_time = 0;

    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Dummy 4");
    // dumpMemoryUsage();

    dri.track_index = 8;
    dri.midi_channel = 0;
    dri.cc_base = 0;
    dri.prefix = "ch9_"; ch9.Init(&dri);
    dri.prefix = "ch9_tbd03_"; ch9_td3.Init(&dri);
    dri.prefix = "ch9_smp_"; ch9_smp.Init(&dri);
    ch9_render_time = 0;

    // dumpMemoryUsage();

    dri.track_index = 9;
    dri.midi_channel = 1;
    dri.cc_base = 0;
    dri.prefix = "ch10_"; ch10.Init(&dri);
    dri.prefix = "ch10_tbd03_"; ch10_td3.Init(&dri);
    dri.prefix = "ch10_smp_"; ch10_smp.Init(&dri);
    ch10_render_time = 0;

    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Dummy 5");
    // dumpMemoryUsage();

    dri.track_index = 10;
    dri.midi_channel = 2;
    dri.cc_base = 0;
    dri.prefix = "ch11_"; ch11.Init(&dri);
    dri.prefix = "ch11_mo_"; ch11_mo.Init(&dri);
    dri.prefix = "ch11_smp_"; ch11_smp.Init(&dri);
    ch11_render_time = 0;
    // dumpMemoryUsage();

    dri.track_index = 11;
    dri.midi_channel = 3;
    dri.cc_base = 0;
    dri.prefix = "ch12_"; ch12.Init(&dri);
    dri.prefix = "ch12_wtosc_"; ch12_wtosc.Init(&dri);
    dri.prefix = "ch12_mo_"; ch12_mo.Init(&dri);
    dri.prefix = "ch12_smp_"; ch12_smp.Init(&dri);
    ch12_render_time = 0;

    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Dummy 6");
    // dumpMemoryUsage();

    dri.track_index = 12;
    dri.midi_channel = 4;
    dri.cc_base = 0;
    dri.prefix = "ch13_"; ch13.Init(&dri);
    dri.prefix = "ch13_smp_"; ch13_smp.Init(&dri);
    ch13_render_time = 0;
    // dumpMemoryUsage();

    dri.track_index = 13;
    dri.midi_channel = 5;
    dri.cc_base = 0;
    dri.prefix = "ch14_"; ch14.Init(&dri);
    dri.prefix = "ch14_smp_"; ch14_smp.Init(&dri);
    ch14_render_time = 0;

    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Dummy 7");

    dri.track_index = 14;
    dri.midi_channel = 6;
    dri.cc_base = 0;
    dri.prefix = "ch15_"; ch15.Init(&dri);
    dri.prefix = "ch15_pp_"; ch15_pp.Init(&dri);
    dri.prefix = "ch15_smp_"; ch15_smp.Init(&dri);
    ch15_render_time = 0;
    // dumpMemoryUsage();

    dri.track_index = 15;
    dri.midi_channel = 7;
    dri.cc_base = 0;
    dri.prefix = "ch16_"; ch16.Init(&dri);
    ch16.level = 0;
    dri.prefix = "ch16_in_"; ch16_in.Init(&dri); // audio input, no prefix
    ch16_render_time = 0;

    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Dummy 8");
    // dumpMemoryUsage();

    dri.prefix = "fx1_";
    fx_delay.Init(&dri);
    fx_delay_render_time = 0;

    dri.prefix = "fx2_";
    fx_reverb.Init(&dri);
    fx_reverb_render_time = 0;

    dri.prefix = "mmm_";
    fx_master.Init(&dri);
    fx_master_render_time = 0;

    // print out some stats.
    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "DrumRack: number of parameters registered %d", pMapPar.size());
    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "DrumRack: number of CC's registered %d", pMapParCC.size());
    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "DrumRack: number of macro CC's registered %d", pMapMacroParCC.size());
    dumpMemoryUsage();

#ifdef TBD_SIM
    // do not load a preset
#else
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);
#endif

    // delay
    delayBuffer_l = static_cast<float*>(heap_caps_malloc(delayBufferSizeMax * sizeof(float), MALLOC_CAP_SPIRAM));
    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Allocate: delayBuffer_l=0x%x", (unsigned int)delayBuffer_l);
    assert(delayBuffer_l != nullptr);
    std::fill_n(delayBuffer_l, delayBufferSizeMax, 0.f);

    delayBuffer_r = static_cast<float*>(heap_caps_malloc(delayBufferSizeMax * sizeof(float), MALLOC_CAP_SPIRAM));
    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Allocate: delayBuffer_r=0x%x", (unsigned int)delayBuffer_r);
    assert(delayBuffer_r != nullptr);
    std::fill_n(delayBuffer_r, delayBufferSizeMax, 0.f);

    // reverb
    reverbBuffer = static_cast<float*>(heap_caps_malloc(32768 * sizeof(float), MALLOC_CAP_SPIRAM));
    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Allocate: reverbBuffer=0x%x", (unsigned int)reverbBuffer);
    assert(reverbBuffer != nullptr);
    std::fill_n(reverbBuffer, 32768, 0.f);

    // Pre-delay ring buffer ahead of the reverb tank.
    preDelayBuf = static_cast<float*>(heap_caps_malloc(preDelayBufSize * sizeof(float), MALLOC_CAP_SPIRAM));
    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Allocate: preDelayBuf=0x%x", (unsigned int)preDelayBuf);
    assert(preDelayBuf != nullptr);
    std::fill_n(preDelayBuf, preDelayBufSize, 0.f);
    preDelayWriteIdx = 0;

    // assert(blockSize >= 32768 * 4);
    reverb.Init(reverbBuffer); // requires 32768*4 bytes = 128KB
    reverb.Clear();
    // blockPtr = static_cast<void*>(static_cast<uint8_t*>(blockPtr) + 32768 * 4);
    // blockSize -= 32768 * 4;
    reverb.set_diffusion(0.7f);
    reverb.set_input_gain(.5f); // left and right are summed
    reverb.set_amount(1.f);
    reverb.set_lp(0.5f);
    reverb.set_time(0.4f);

    // init compressor
    sumCompressor.setSampleRate(44100.f);
    sumCompressor.initRuntime();

    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "After Init()");
    dumpMemoryUsage();
}

ctagSoundProcessorPicoSeqRack::~ctagSoundProcessorPicoSeqRack(){
}

#define DEFINE_GLOBAL_PARAM(name, channel, cc, parametername) \
    pMapParCC.emplace(CC_TO_MAP_KEY(channel, cc), PsramVector<function<void(const int)>>{[&](const int val){ parametername = val;}});

void ctagSoundProcessorPicoSeqRack::knowYourself(){
    // autogenerated code here
    // sectionCpp0

    DEFINE_GLOBAL_PARAM("fx1_time_ms", 13, 20, fx1_time_ms);
    DEFINE_GLOBAL_PARAM("fx1_sync", 13, 21, fx1_sync);
    DEFINE_GLOBAL_PARAM("fx1_freeze", 13, 22, fx1_freeze);
    DEFINE_GLOBAL_PARAM("fx1_tape_digital", 13, 23, fx1_tape_digital);
    DEFINE_GLOBAL_PARAM("fx1_st_width", 13, 24, fx1_st_width);
    DEFINE_GLOBAL_PARAM("fx1_fx_send", 13, 25, fx1_fx_send);
    DEFINE_GLOBAL_PARAM("fx1_feedback", 13, 26, fx1_feedback);
    DEFINE_GLOBAL_PARAM("fx1_base", 13, 27, fx1_base);
    DEFINE_GLOBAL_PARAM("fx1_width", 13, 28, fx1_width);
    DEFINE_GLOBAL_PARAM("fx1_amount", 13, 29, fx1_amount);

    DEFINE_GLOBAL_PARAM("fx2_time", 13, 40, fx2_time);
    DEFINE_GLOBAL_PARAM("fx2_lp", 13, 41, fx2_lp);
    DEFINE_GLOBAL_PARAM("fx2_amount", 13, 42, fx2_amount);
    DEFINE_GLOBAL_PARAM("fx2_diffuse",  13, 43, fx2_diffuse);
    DEFINE_GLOBAL_PARAM("fx2_predelay", 13, 44, fx2_predelay);
    DEFINE_GLOBAL_PARAM("fx2_modulation", 13, 45, fx2_modulation);
    DEFINE_GLOBAL_PARAM("fx2_input_gain", 13, 46, fx2_input_gain);
    DEFINE_GLOBAL_PARAM("fx2_tank_level", 13, 47, fx2_tank_level);
    DEFINE_GLOBAL_PARAM("fx1_input_hp", 13, 30, fx1_input_hp);
    DEFINE_GLOBAL_PARAM("fx2_hp",       13, 48, fx2_hp);

    DEFINE_GLOBAL_PARAM("c_thres", 13, 60, c_thres);
    DEFINE_GLOBAL_PARAM("c_ratio", 13, 61, c_ratio);
    DEFINE_GLOBAL_PARAM("c_atk", 13, 62, c_atk);
    DEFINE_GLOBAL_PARAM("c_rel", 13, 63, c_rel);
    DEFINE_GLOBAL_PARAM("c_lpf", 13, 64, c_lpf);
    DEFINE_GLOBAL_PARAM("c_gain", 13, 65, c_gain);
    DEFINE_GLOBAL_PARAM("c_mix", 13, 66, c_mix);
    // CCs 67/68 retired (c_dly_level / c_rev_level had no DSP referent).

    DEFINE_GLOBAL_PARAM("sum_mute", 13, 80, sum_mute);
    DEFINE_GLOBAL_PARAM("sum_lev", 13, 81, sum_lev);
    DEFINE_GLOBAL_PARAM("sum_drive", 13, 82, sum_drive);

    isStereo = true;
	id = "PicoSeqRack";
	// sectionCpp0
}


void ctagSoundProcessorPicoSeqRack::parseIncomingMidiMessages(const uint8_t *buf, const size_t len) {
    // if (len > 0) {
    //     ESP_LOGI("ctagSoundProcessorPicoSeqRack",
    //         "parseIncomingMidiMessages: %02X %02X %02X %02X %02X %02X %02X %02X %02X (%d)",
    //             buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], len);
    // }

    if (buf == nullptr || len < 1)  {
        return;
    }

    int left = len;
    int o = 0;

    while(left > 0) {
        uint8_t b0 = buf[o++];
        left --;

        if (b0 == 0) {
            // probably end of event stream
            return;
        }

        uint8_t channel = (b0 & 0x0F);
        uint8_t cmd = (b0 & 0xF0);

        switch(cmd) {
            case 0x80: // note off
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++];
                left -= 2;
                // _handleMidiNoteOff(channel, b1, b2);
                break;
            }
            case 0x90: // note on
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++];
                left -= 2;
                break;
            }
            case 0xA0: // aftertouch
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++];
                left -= 2;
                break;
            }
            case 0xB0: // control change
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++];
                left -= 2;
                handleMidiControlChange(channel, b1, b2);
                break;
            }
            case 0xC0: // program change
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++]; // not used?
                left -= 2;
                break;
            }
            case 0xE0: // pitch bend
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++];
                left -= 2;
                break;
            }
            case 0xF0: // system common / real time
                // TODO handle sysex, clock, start, stop, continue, ...
                switch(b0) {
                    case 0xF0: // sysex start
                        return; // just ignore and bail out for now

                    case 0xF8: // timing clock
                    case 0xFA: // start
                    case 0xFB: // continue
                    case 0xFC: // stop
                    case 0xFE: // active sensing
                    case 0xFF: // system reset
                        // ignore for now
                        break;
                }
                break; // fail for now
            default:
                // for anything else, just stop parsing.
                return;
        }
    }
}

void ctagSoundProcessorPicoSeqRack::setTrackMachine(const uint8_t trackIndex, const std::string machineId, float volumeMultiplier) {
    printf("PicoSeqRack: setTrackMachine(%d, \"%s\", %f)\n", trackIndex, machineId.c_str(), volumeMultiplier);

    if (trackIndex == 0) {
        ch1.enabled = !machineId.empty();
        ch1.volumeMultiplier = volumeMultiplier;
        ch1_db.enabled = machineId == "db";
        ch1_ab.enabled = machineId == "ab";
        ch1_smp.enabled = machineId == "ro";
        // printf("  ch1=%d, ch1_db=%d, ch1_ab=%d, ch1_ro=%d\n", ch1.enabled, ch1_db.enabled, ch1_ab.enabled, ch1_smp.enabled);
    }
    else if (trackIndex == 1) {
        ch2.enabled = !machineId.empty();
        ch2.volumeMultiplier = volumeMultiplier;
        ch2_fmb1.enabled = machineId == "fmb";
        ch2_smp.enabled = machineId == "ro";
        // printf("  ch2=%d, ch2_fmb1=%d, ch2_ro=%d\n", ch2.enabled, ch2_fmb1.enabled, ch2_smp.enabled);
    }
    else if (trackIndex == 2) {
        ch3.enabled = !machineId.empty();
        ch3.volumeMultiplier = volumeMultiplier;
        ch3_ds.enabled = machineId == "ds";
        ch3_as.enabled = machineId == "as";
        ch3_smp.enabled = machineId == "ro";
        // printf("  ch3=%d, ch3_ds=%d, ch3_as=%d, ch3_ro=%d\n", ch3.enabled, ch3_ds.enabled, ch3_as.enabled, ch3_smp.enabled);
    }
    else if (trackIndex == 3) {
        ch4.enabled = !machineId.empty();
        ch4.volumeMultiplier = volumeMultiplier;
        ch4_hh1.enabled = machineId == "hh1";
        ch4_hh2.enabled = machineId == "hh2";
        ch4_smp.enabled = machineId == "ro";
        // printf("  ch4=%d, ch4_hh1=%d, ch4_hh2=%d, ch4_ro=%d\n", ch4.enabled, ch4_hh1.enabled, ch4_hh2.enabled, ch4_smp.enabled);
    }
    else if (trackIndex == 4) {
        ch5.enabled = !machineId.empty();
        ch5.volumeMultiplier = volumeMultiplier;
        ch5_rs.enabled = machineId == "rs";
        ch5_smp.enabled = machineId == "ro";
        // printf("  ch5=%d, ch5_rs=%d, ch5_ro=%d\n", ch5.enabled, ch5_rs.enabled, ch5_smp.enabled);
    }
    else if (trackIndex == 5) {
        ch6.enabled = !machineId.empty();
        ch6.volumeMultiplier = volumeMultiplier;
        ch6_cl.enabled = machineId == "cl";
        ch6_smp.enabled = machineId == "ro";
        // printf("  ch6=%d, ch6_cl=%d\n", ch6.enabled, ch6_cl.enabled);
    }
    else if (trackIndex == 6) {
        ch7.enabled = !machineId.empty();
        ch7.volumeMultiplier = volumeMultiplier;
        ch7_smp.enabled = machineId == "ro";
        // printf("  ch7=%d, ch7_ro=%d\n", ch7.enabled, ch7_smp.enabled);
    }
    else if (trackIndex == 7) {
        ch8.enabled = !machineId.empty();
        ch8.volumeMultiplier = volumeMultiplier;
        ch8_smp.enabled = machineId == "ro";
        // printf("  ch8=%d, ch8_ro=%d\n", ch8.enabled, ch8_smp.enabled);
    }
    else if (trackIndex == 8) {
        ch9.enabled = !machineId.empty();
        ch9.volumeMultiplier = volumeMultiplier;
        ch9_td3.enabled = machineId == "td3";
        ch9_smp.enabled = machineId == "ro";
        // printf("  ch9=%d, ch9_td3=%d, ch9_ro=%d\n", ch9.enabled, ch9_td3.enabled, ch9_smp.enabled);
    }
    else if (trackIndex == 9) {
        ch10.enabled = !machineId.empty();
        ch10.volumeMultiplier = volumeMultiplier;
        ch10_td3.enabled = machineId == "td3";
        ch10_smp.enabled = machineId == "ro";
        // printf("  ch10=%d, ch10_td3=%d, ch10_smp=%d\n", ch10.enabled, ch10_td3.enabled, ch10_smp.enabled);
    }
    else if (trackIndex == 10) {
        ch11.enabled = !machineId.empty();
        ch11.volumeMultiplier = volumeMultiplier;
        ch11_mo.enabled = machineId == "mo";
        ch11_smp.enabled = machineId == "ro";
        // printf("  ch11=%d, ch11_mo=%d, ch11_ro=%d\n", ch11.enabled, ch11_mo.enabled, ch11_smp.enabled);
    }
    else if (trackIndex == 11) {
        ch12.enabled = !machineId.empty();
        ch12.volumeMultiplier = volumeMultiplier;
        ch12_wtosc.enabled = machineId == "wtosc";
        ch12_mo.enabled = machineId == "mo";
        ch12_smp.enabled = machineId == "ro";
        // printf("  ch12=%d, ch12_wtosc=%d, ch12_mo=%d, ch12_ro=%d\n", ch12.enabled, ch12_wtosc.enabled, ch12_mo.enabled, ch12_smp.enabled);
    }
    else if (trackIndex == 12) {
        ch13.enabled = !machineId.empty();
        ch13.volumeMultiplier = volumeMultiplier;
        ch13_smp.enabled = machineId == "ro";
        // printf("  ch13=%d, ch13_ro=%d\n", ch13.enabled, ch13_smp.enabled);
    }
    else if (trackIndex == 13) {
        ch14.enabled = !machineId.empty();
        ch14.volumeMultiplier = volumeMultiplier;
        ch14_smp.enabled = machineId == "ro";
        // printf("  ch14=%d, ch14_ro=%d\n", ch14.enabled, ch14_smp.enabled);
    }
    else if (trackIndex == 14) {
        ch15.enabled = !machineId.empty();
        ch15.volumeMultiplier = volumeMultiplier;
        ch15_pp.enabled = machineId == "pp";
        ch15_smp.enabled = machineId == "ro";
        // printf("  ch15=%d, ch15_pp=%d, ch15_ro=%d\n", ch15.enabled, ch15_pp.enabled, ch15_smp.enabled);
    }
    else if (trackIndex == 15) {
        ch16.enabled = !machineId.empty();
        ch16.volumeMultiplier = volumeMultiplier;
        // Machine id is "inp" per synthdefinitions.json ({id: "inp", name:
        // "Audio Input"}). The previous "in" string check never matched,
        // leaving ch16_in.enabled=false permanently — which caused
        // RackInput::Process() to early-return and the audio passthrough
        // to never copy input → output. Input track 16 was silent.
        ch16_in.enabled = (machineId == "inp");
        // printf("  ch16=%d, ch16_in=%d\n", ch16.enabled, ch16_in.enabled);
    }
}

// Pico-side user mute → rack mixer muted flag. RackChannelMixer::PreProcess
// evaluates `enabled = (level > minVolume) && !muted`, so toggling this
// silences the track regardless of LEVEL. Essential for the Input track
// (ch16, continuous passthrough audio) and cuts synth tails instantly on
// the other 15 tracks.
void ctagSoundProcessorPicoSeqRack::setTrackMute(const uint8_t trackIndex, bool muted) {
    switch (trackIndex) {
        case  0: ch1.muted  = muted; break;
        case  1: ch2.muted  = muted; break;
        case  2: ch3.muted  = muted; break;
        case  3: ch4.muted  = muted; break;
        case  4: ch5.muted  = muted; break;
        case  5: ch6.muted  = muted; break;
        case  6: ch7.muted  = muted; break;
        case  7: ch8.muted  = muted; break;
        case  8: ch9.muted  = muted; break;
        case  9: ch10.muted = muted; break;
        case 10: ch11.muted = muted; break;
        case 11: ch12.muted = muted; break;
        case 12: ch13.muted = muted; break;
        case 13: ch14.muted = muted; break;
        case 14: ch15.muted = muted; break;
        case 15: ch16.muted = muted; break;
        default: break;
    }
}

void ctagSoundProcessorPicoSeqRack::setTrackBank(const uint8_t trackIndex, const uint16_t bankIndex) {
    printf("PicoSeqRack: setTrackBank(%d, %d)\n", trackIndex, bankIndex);

    if (trackIndex == 0) {
        ch1_smp.bank_index = bankIndex;
    }
    else if (trackIndex == 1) {
        ch2_smp.bank_index = bankIndex;
    }
    else if (trackIndex == 2) {
        ch3_smp.bank_index = bankIndex;
    }
    else if (trackIndex == 3) {
        ch4_smp.bank_index = bankIndex;
    }
    else if (trackIndex == 4) {
        ch5_smp.bank_index = bankIndex;
    }
    else if (trackIndex == 5) {
        ch6_smp.bank_index = bankIndex;
    }
    else if (trackIndex == 6) {
        ch7_smp.bank_index = bankIndex;
    }
    else if (trackIndex == 7) {
        ch8_smp.bank_index = bankIndex;
    }
    else if (trackIndex == 8) {
        ch9_smp.bank_index = bankIndex;
    }
    else if (trackIndex == 9) {
        ch10_smp.bank_index = bankIndex;
    }
    else if (trackIndex == 10) {
        ch11_smp.bank_index = bankIndex;
    }
    else if (trackIndex == 11) {
        ch12_smp.bank_index = bankIndex;
    }
    else if (trackIndex == 12) {
        ch13_smp.bank_index = bankIndex;
    }
    else if (trackIndex == 13) {
        ch14_smp.bank_index = bankIndex;
    }
    else if (trackIndex == 14) {
        ch15_smp.bank_index = bankIndex;
    }
    else if (trackIndex == 15) {
        // ch16_smp.bank_index = bankIndex;
    }
}

void ctagSoundProcessorPicoSeqRack::handleMidiNoteOn(const uint8_t channel, uint8_t note, uint8_t velocity) {
    // printf("PicoSeqRack: handleMidiNoteOn(channel=%d, note=%d, velocity=%d)\n", channel, note, velocity);

    if (channel == 9) {
        if (note == 36) {
            if (ch1_ab.enabled) {
                // printf("ch1_ab triggered by note %d, velocity %d\n", note, velocity);
                if (velocity > 0) {
                    ch1_ab.trigger();
                }
            }
            if (ch1_db.enabled) {
                // printf("ch1_db triggered by note %d, velocity %d\n", note, velocity);
                if (velocity > 0) {
                    ch1_db.trigger();
                }
            }
            if (ch1_smp.enabled) {
                // printf("ch1_ro triggered by note %d, velocity %d\n", note, velocity);
                if (velocity > 0) {
                    ch1_smp.noteOn(36, velocity);
                } else {
                    ch1_smp.noteOff(36, 0);
                }
            }
        }
        else if (note == 37) {
            if (ch2_fmb1.enabled) {
                // printf("ch2_fmb1 triggered by note %d, velocity %d\n", note, velocity);
                if (velocity > 0) {
                    ch2_fmb1.trigger();
                }
            }
            if (ch2_smp.enabled) {
                // printf("ch2_ro triggered by note %d, velocity %d\n", note, velocity);
                if (velocity > 0) {
                    ch2_smp.noteOn(36, velocity);
                } else {
                    ch2_smp.noteOff(36, 0);
                }
            }
        }
        else if (note == 38) {
            if (ch3_as.enabled) {
                // printf("ch3_as triggered by note %d, velocity %d\n", note, velocity);
                if (velocity > 0) {
                    ch3_as.trigger();
                }
            }
            if (ch3_ds.enabled) {
                // printf("ch3_ds triggered by note %d, velocity %d\n", note, velocity);
                if (velocity > 0) {
                    ch3_ds.trigger();
                }
            }
            if (ch3_smp.enabled) {
                // printf("ch3_ro triggered by note %d, velocity %d\n", note, velocity);
                if (velocity > 0) {
                    ch3_smp.noteOn(36, velocity);
                } else {
                    ch3_smp.noteOff(36, 0);
                }
            }
        }
    } else if (channel == 10) {
        if (note == 36) {
            if (ch4_hh1.enabled) {
                // printf("ch4_hh1 triggered by note %d, velocity %d\n", note, velocity);
                if (velocity > 0) {
                    ch4_hh1.trigger();
                }
            }
            if (ch4_hh2.enabled) {
                // printf("ch4_hh2 triggered by note %d, velocity %d\n", note, velocity);
                if (velocity > 0) {
                    ch4_hh2.trigger();
                }
            }
            if (ch4_smp.enabled) {
                    // printf("ch4_ro triggered by note %d, velocity %d\n", note, velocity);
                if (velocity > 0) {
                    ch4_smp.noteOn(36, velocity);
                } else {
                    ch4_smp.noteOff(36, 0);
                }
            }
        }
        else if (note == 37) {
            if (ch5_rs.enabled) {
                // printf("ch5_rs triggered by note %d, velocity %d\n", note, velocity);
                if (velocity > 0) {
                    ch5_rs.trigger();
                }
            }
            if (ch5_smp.enabled) {
                // printf("ch5_ro triggered by note %d, velocity %d\n", note, velocity);
                if (velocity > 0) {
                    ch5_smp.noteOn(36, velocity);
                } else {
                    ch5_smp.noteOff(36, 0);
                }
            }
        }
        else if (note == 38) {
            if (ch6_cl.enabled) {
                if (velocity > 0) {
                    // printf("ch6_cl triggered by note %d, velocity %d\n", note, velocity);
                    ch6_cl.trigger();
                }
            }
            if (ch6_smp.enabled) {
                // printf("ch6_ro triggered by note %d, velocity %d\n", note, velocity);
                if (velocity > 0) {
                    ch6_smp.noteOn(36, velocity);
                } else {
                    ch6_smp.noteOff(36, 0);
                }
            }
        }
    } else if (channel == 11) {
        if (note == 36) {
            if (ch7_smp.enabled) {
                // printf("ch7_ro triggered by note %d, velocity %d\n", note, velocity);
                if (velocity > 0) {
                    ch7_smp.noteOn(36, velocity);
                } else {
                    ch7_smp.noteOff(36, 0);
                }
            }
        }
        else if (note == 37) {
            if (ch8_smp.enabled) {
                // printf("ch8_ro triggered by note %d, velocity %d\n", note, velocity);
                if (velocity > 0) {
                    ch8_smp.noteOn(36, velocity);
                } else {
                    ch8_smp.noteOff(36, 0);
                }
            }
        }
    }
    else if (channel == 0) {
        if (ch9_td3.enabled) {
            //  printf("ch9    _td3 triggered by note %d, velocity %d\n", note, velocity);
            if (velocity > 0) {
                ch9_td3.noteOn(note, velocity);
            } else {
                ch9_td3.noteOff(note, 0);
            }
        }
        if (ch9_smp.enabled) {
            // printf("ch9_ro triggered by note %d, velocity %d\n", note, velocity);
            if (velocity > 0) {
                ch9_smp.noteOn(note, velocity);
            } else {
                ch9_smp.noteOff(note, 0);
            }
        }
    }
    else if (channel == 1) {
        if (ch10_td3.enabled) {
            // printf("ch10_td3 triggered by note %d, velocity %d\n", note, velocity);
            if (velocity > 0) {
                ch10_td3.noteOn(note, velocity);
            } else {
                ch10_td3.noteOff(note, 0);
            }
        }
        if (ch10_smp.enabled) {
            // printf("ch10_smp triggered by note %d, velocity %d\n", note, velocity);
            if (velocity > 0) {
                ch10_smp.noteOn(note, velocity);
            } else {
                ch10_smp.noteOff(note, 0);
            }
        }
    }
    else if (channel == 2) {
        if (ch11_mo.enabled) {
            // printf("ch11_mo triggered by note %d, velocity %d\n", note, velocity);
            if (velocity > 0) {
                ch11_mo.noteOn(note, velocity);
            } else {
                ch11_mo.noteOff(note, 0);
            }
        }
        if (ch11_smp.enabled) {
            // printf("ch11_ro triggered by note %d, velocity %d\n", note, velocity);
            if (velocity > 0) {
                ch11_smp.noteOn(note, velocity);
            } else {
                ch11_smp.noteOff(note, 0);
            }
        }
    }
    else if (channel == 3) {
        if (ch12_mo.enabled) {
            // printf("ch12_mo triggered by note %d, velocity %d\n", note, velocity);
            if (velocity > 0) {
                ch12_mo.noteOn(note, velocity);
            } else {
                ch12_mo.noteOff(note, 0);
            }
        }
        if (ch12_smp.enabled) {
            // printf("ch12_ro triggered by note %d, velocity %d\n", note, velocity);
            if (velocity > 0) {
                ch12_smp.noteOn(note, velocity);
            } else {
                ch12_smp.noteOff(note, 0);
            }
        }
    }
    else if (channel == 4) {
        if (ch13_smp.enabled) {
            // printf("ch13_ro triggered by note %d, velocity %d\n", note, velocity);
            if (velocity > 0) {
                ch13_smp.noteOn(note, velocity);
            } else {
                ch13_smp.noteOff(note, 0);
            }
        }
    }
    else if (channel == 5) {
        if (ch14_smp.enabled) {
            // printf("ch14_ro triggered by note %d, velocity %d\n", note, velocity);
            if (velocity > 0) {
                ch14_smp.noteOn(note, velocity);
            } else {
                ch14_smp.noteOff(note, 0);
            }
        }
    }
    else if (channel == 6) {
        if (ch15_pp.enabled) {
            // printf("ch15_pp triggered by note %d, velocity %d\n", note, velocity);
            if (velocity > 0) {
                ch15_pp.noteOn(note, velocity);
            } else {
                ch15_pp.noteOff(note, 0);
            }
        }
        if (ch15_smp.enabled) {
            // printf("ch15_ro triggered by note %d, velocity %d\n", note, velocity);
            if (velocity > 0) {
                ch15_smp.noteOn(note, velocity);
            } else {
                ch15_smp.noteOff(note, 0);
            }
        }
    }
}

void ctagSoundProcessorPicoSeqRack::handleMidiNoteOff(const uint8_t channel, uint8_t note, uint8_t velocity) {
    // printf("PicoSeqRack: handleMidiNoteOff(channel=%d, note=%d, velocity=%d)\n", channel, note, velocity);
    if (channel == 9) {
        if (note == 36) {
            if (ch1_smp.enabled) {
                ch1_smp.noteOff(36, 0);
            }
        }
        if (note == 37) {
            if (ch2_smp.enabled) {
                ch2_smp.noteOff(36, 0);
            }
        }
        if (note == 38) {
            if (ch3_smp.enabled) {
                ch3_smp.noteOff(36, 0);
            }
        }
    } else if (channel == 10) {
        if (note == 36) {
            if (ch4_smp.enabled) {
                ch4_smp.noteOff(36, 0);
            }
        }
        if (note == 37) {
            if (ch5_smp.enabled) {
                ch5_smp.noteOff(36, 0);
            }
        }
        if (note == 38) {
            if (ch6_smp.enabled) {
                ch6_smp.noteOff(36, 0);
            }
        }
    } else if (channel == 11) {
        if (note == 36) {
            if (ch7_smp.enabled) {
                ch7_smp.noteOff(36, 0);
            }
        }
        if (note == 37) {
            if (ch8_smp.enabled) {
                ch8_smp.noteOff(36, 0);
            }
        }
    }
    else if (channel == 0) {
        if (ch9_td3.enabled) {
            ch9_td3.noteOff(note, 0);
        }
        if (ch9_smp.enabled) {
            ch9_smp.noteOff(note, 0);
        }
    }
    else if (channel == 1) {
        if (ch10_td3.enabled) {
            ch10_td3.noteOff(note, 0);
        }
        if (ch10_smp.enabled) {
            ch10_smp.noteOff(note, 0);
        }
    }
    else if (channel == 2) {
        if (ch11_mo.enabled) {
            ch11_mo.noteOff(note, 0);
        }
        if (ch11_smp.enabled) {
            ch11_smp.noteOff(note, 0);
        }
    }
    else if (channel == 3) {
        if (ch12_mo.enabled) {
            ch12_mo.noteOff(note, 0);
        }
        if (ch12_smp.enabled) {
            ch12_smp.noteOff(note, 0);
        }
    }
    else if (channel == 4) {
        if (ch13_smp.enabled) {
            ch13_smp.noteOff(note, 0);
        }
    }
    else if (channel == 5) {
        if (ch14_smp.enabled) {
            ch14_smp.noteOff(note, 0);
        }
    }
    else if (channel == 6) {
        if (ch15_pp.enabled) {
            ch15_pp.noteOff(note, 0);
        }
        if (ch15_smp.enabled) {
            ch15_smp.noteOff(note, 0);
        }
    }
}

