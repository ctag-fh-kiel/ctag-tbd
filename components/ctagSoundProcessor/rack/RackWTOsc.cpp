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

#include "RackSynth.hpp"
#include "RackWTOsc.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"
#include "helpers/ctagNumUtil.hpp"
#include "plaits/dsp/engine/engine.h"
#include "braids/quantizer_scales.h"
#include "esp_heap_caps.h"

using namespace CTAG::SP;

void RackWTOsc::Init(const PickSeqRackInitData *initdata) {
    lfo.SetSampleRate(44100.f / BUF_SZ);
    lfo.SetFrequency(1.f);

    // Allocate ONE bank's worth of PSRAM (~33 KB) plus a 2 KB float
    // scratch for the integration math. Bank change in Process triggers
    // a one-shot prepareBank(currentBank) — single audio-block glitch on
    // change is acceptable for non-real-time bank selection.
    bankBuffer = static_cast<int16_t*>(heap_caps_malloc(kBankBytes, MALLOC_CAP_SPIRAM));
    memset(bankBuffer, 0, kBankBytes);
    fbufScratch = static_cast<float*>(heap_caps_malloc(512 * sizeof(float), MALLOC_CAP_SPIRAM));

    // Preload bank 0 so the first noteOn doesn't have to do the prep.
    prepareBank(currentBank, initdata->sampleRom);
    lastBank = currentBank;

    oscillator.Init();
    svf.Init();
    adsr.SetModeExp();
    adsr.SetSampleRate(44100.f / BUF_SZ);
    adsr.Reset();
    pitchQuantizer.Init();

    this->enabled = false;

    initdata->rack->registerParamAndCC(initdata, "wavebank", 8, [&](const int val){ wavebank = val;});
    initdata->rack->registerParamAndCC(initdata, "wave", 9, [&](const int val){ wave = val;});
    initdata->rack->registerParamAndCC(initdata, "tune", 10, [&](const int val){ tune = val;});

    initdata->rack->registerParamAndCC(initdata, "fmode", 11, [&](const int val){ fmode = val;});
    initdata->rack->registerParamAndCC(initdata, "fcut", 12, [&](const int val){ fcut = val;});
    initdata->rack->registerParamAndCC(initdata, "freso", 13, [&](const int val){ freso = val;});
    /*
     * q_scale (Braids quantizer scale select) is preserved in DSP code but
     * its CC registration is disabled — ctrl=14 is now the layout-correct
     * slot for Gain (macro idx=6, idx+8 invariant). The quantizer code in
     * Process() is currently commented out anyway, so the parameter has no
     * audible effect. Re-enable both lines together if pitch-quantization
     * is brought back, and choose a different ctrl free of macro conflict.
     */
    // initdata->rack->registerParamAndCC(initdata, "q_scale", 14, [&](const int val){ q_scale = val;});

    initdata->rack->registerParamAndCC(initdata, "attack", 15, [&](const int val){ attack = val;});
    initdata->rack->registerParamAndCC(initdata, "decay", 16, [&](const int val){ decay = val;});
    initdata->rack->registerParamAndCC(initdata, "sustain", 17, [&](const int val){ sustain = val;});
    initdata->rack->registerParamAndCC(initdata, "release", 18, [&](const int val){ release = val;});

    initdata->rack->registerParamAndCC(initdata, "eg2wave", 19, [&](const int val){ eg2wave = val;});
    initdata->rack->registerParamAndCC(initdata, "eg2fm", 20, [&](const int val){ eg2fm = val;});
    initdata->rack->registerParamAndCC(initdata, "eg2filtfm", 21, [&](const int val){ eg2filtfm = val;});

    initdata->rack->registerParamAndCC(initdata, "lfospeed", 22, [&](const int val){ lfospeed = val;});
    initdata->rack->registerParamAndCC(initdata, "lfosync", 23, [&](const int val){ lfosync = val;});

    // lfo2* ctrl numbers are 24..27 to satisfy the macro idx+8=ctrl
    // invariant (LFO Mod page = macro idx 16..19). ctrl=24 was previously
    // reserved for egfasl (Slow EG toggle) which is currently disabled
    // in DSP, so it's free to reuse.
    initdata->rack->registerParamAndCC(initdata, "lfo2wave",   24, [&](const int val){ lfo2wave = val;});
    initdata->rack->registerParamAndCC(initdata, "lfo2am",     25, [&](const int val){ lfo2am = val;});
    initdata->rack->registerParamAndCC(initdata, "lfo2fm",     26, [&](const int val){ lfo2fm = val;});
    initdata->rack->registerParamAndCC(initdata, "lfo2filtfm", 27, [&](const int val){ lfo2filtfm = val;});

    // Gain is exposed at TWO ctrl numbers:
    //  - ctrl=14 — the layout-correct slot for macro idx=6 (idx+8 invariant,
    //              docs/architecture/macro-system.md). Required so that
    //              MacroTranslator's storage slot ctrl-8=6 receives Gain
    //              writes, matching trackParameterValues[t][6].
    //  - ctrl=29 — kept for backward compatibility with the original WTOsc
    //              CC layout / any saved presets that still reference it.
    initdata->rack->registerParamAndCC(initdata, "gain",  14, [&](const int val){ gain = val;});
    initdata->rack->registerParamAndCC(initdata, "gain2", 29, [&](const int val){ gain = val;});
}

void RackWTOsc::prepareBank(int b, HELPERS::ctagSampleRom *sampleRom) {
    // Bank b occupies sub-slices [b*64 .. b*64+63] in the sample-rom
    // PSRAM region (each sub-slice = 256 raw int16 samples). If those
    // slices aren't loaded (active wt-bank descriptor has fewer entries),
    // mark the wavetable bad and bail — Process will skip rendering.
    if (!sampleRom->HasSliceGroup(b * 64, b * 64 + 63)) {
        isWaveTableGood = false;
        return;
    }
    if (sampleRom->GetSliceGroupSize(b * 64, b * 64 + 63) != 256 * 64) {
        isWaveTableGood = false;
        return;
    }

    int16_t *buf = bankBuffer;
    const int bufferOffset = 4 * 64;  // 256

    // ReadSlice clips at sliceSize=256, so the legacy upstream pattern
    // ReadSlice(buf, slice, 0, 256*64) returns only ONE wave's worth.
    // Walk the 64 sub-slices explicitly.
    for (int j = 0; j < 64; j++) {
        sampleRom->ReadSlice(&buf[bufferOffset + j * 256], b * 64 + j, 0, 256);
    }

    // Integrated-wavetable preprocessing
    // (https://www.dafx12.york.ac.uk/papers/dafx12_submission_69.pdf,
    // K=1, N=1). See the maths reference in the original ctagSound-
    // ProcessorWTOsc::prepareWavetables for the per-step rationale.
    int c = 0;
    for (int i = 0; i < 64; i++) {
        int startOffset = bufferOffset + i * 256;
        float sum4 = buf[startOffset] + buf[startOffset+1] + buf[startOffset+2] + buf[startOffset+3];
        for (int j = 0; j < 512; j++) {
            fbufScratch[j] = buf[startOffset + (j % 256)] + sum4;
        }
        removeMeanOfFloatArray(fbufScratch, 512);
        scaleFloatArrayToAbsMax(fbufScratch, 512);
        accumulateFloatArray(fbufScratch, 512);
        removeMeanOfFloatArray(fbufScratch, 512);
        for (int j = 512 - 256 - 4; j < 512; j++) {
            int16_t v = static_cast<int16_t>(roundf(fbufScratch[j] * 4.f * 32768.f / 256.f));
            buf[c++] = v;
        }
    }
    for (int i = 0; i < 64; i++) wavetables[i] = &buf[i * 260];
    isWaveTableGood = true;
}

void RackWTOsc::noteOn(uint8_t note, uint8_t vel) {
    midi_trig = true;
    midi_note = note;
    midi_freq = 440.f * powf(2.f, (note - 69) / 12.f);
    pitch = note * 128.0f;
    note_held = true;
    pending_velocity = vel ? vel : 100;
    pending_retrigger = true;  // see hpp comment — forces a clean re-attack
}

void RackWTOsc::noteOff(uint8_t note, uint8_t vel) {
    (void)note;
    (void)vel;
    midi_trig = false;
    note_held = false;
    // env_value continues to ring out via the AHR release in Process —
    // do NOT zero it here, otherwise the user gets a click instead of a
    // tail. The silence gate handles eventual full mute.
}

void RackWTOsc::Process(const PicoSeqRackProcessData &data) {
    if (!this->enabled) {
        return;
    }

    std::fill_n(out, BUF_SZ, 0.f);

    // ---- Stuck-note watchdog (RackRompler-style, line 273-296) -----------
    // The Pico sequencer's stop-path does not emit noteOff for currently-
    // held notes — without this safety net, voices on pitched instruments
    // hang at sustain level forever (user has to power-cycle to silence).
    // Reset trig_age_ticks on every fresh noteOn pulse; count blocks
    // otherwise; force-release after kStaleTriggerTicks (~6 s) of silence.
    if (pending_retrigger) {
        trig_age_ticks = 0;
    } else if (trig_age_ticks < UINT32_MAX) {
        trig_age_ticks++;
    }
    if (note_held && trig_age_ticks > kStaleTriggerTicks) {
        // No fresh trigger in ~6 s — assume dropped/missing noteOff,
        // force-release. ADSR.Gate(false) on the consume below moves
        // the envelope into env_release, fades naturally.
        note_held = false;
        midi_trig = false;
    }
    // ----------------------------------------------------------------------

    // ---- Silence gate ----------------------------------------------------
    // Activity = note held OR ADSR still ringing (covers attack→decay→
    // sustain→release tail). Inactive blocks past the tail emit zeros and
    // skip the engine entirely.
    const bool any_activity = note_held || !adsr.IsIdle();
    if (any_activity) {
        silence_tail_blocks = 0;
    } else {
        silence_tail_blocks++;
        if (silence_tail_blocks > kSilenceTailBlocks) {
            return;  // out[] is already zeroed above
        }
    }
    // ----------------------------------------------------------------------

    // wave select — wavebank is normalized to 0..4096 by registerParamAndCC.
    // Map across all 32 banks.
    currentBank = (wavebank * kMaxBanks) / 4096;
    if (currentBank < 0) currentBank = 0;
    if (currentBank >= kMaxBanks) currentBank = kMaxBanks - 1;

    fWave = wave / 4095.f;

    if (lastBank != currentBank) {
        // One-shot heavy preprocessing on bank change. ~10 ms blocking
        // work; produces an audible audio glitch at the moment of
        // change. Acceptable since bank changes are non-real-time.
        prepareBank(currentBank, data.sampleRom);
        lastBank = currentBank;
    }

    // Gain — clean linear 0..1 mapping (was 0..2 with quadratic squashing,
    // which made the upper half of the knob inaudible against the AM clamp).
    MK_FLT_PAR_ABS_NOCV(fGain, gain, 4095.f, 1.f)

    // ADSR drives master amplitude (and mod routings via valADSR).
    // Gate input is note_held — the sustained "is the user pressing
    // a key right now" state. The watchdog above can clear note_held
    // independently if no fresh trigger arrives for ~6 s (sequencer-
    // stop safety net).
    //
    // Force-retrigger pattern: ctagADSREnv::Gate(true) is a no-op when
    // currently in env_decay or env_sustain — so we must Reset() the
    // ADSR on every fresh noteOn to guarantee re-attack from zero,
    // otherwise back-to-back sequencer steps within the previous note's
    // decay tail produce silence. pending_retrigger crosses threads
    // (set in noteOn from SPI/MIDI task, consumed here on audio task)
    // — volatile read + single-write makes the consume race-safe.
    if (pending_retrigger) {
        pending_retrigger = false;
        adsr.Reset();
    }
    adsr.Gate(note_held);
    MK_FLT_PAR_ABS_NOCV(fAttack, attack, 4095.f, 10.f)
    MK_FLT_PAR_ABS_NOCV(fDecay, decay, 4095.f, 10.f)
    MK_FLT_PAR_ABS_NOCV(fSustain, sustain, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(fRelease, release, 4095.f, 10.f)
    adsr.SetAttack(fAttack);
    adsr.SetDecay(fDecay);
    adsr.SetSustain(fSustain);
    adsr.SetRelease(fRelease);

    // EG mod amounts are SEMANTICALLY BIPOLAR (knob mid = no mod,
    // knob full-down = max negative mod, knob full-up = max positive
    // mod). The MK_FLT_PAR_ABS_SFT_NOCV macro in the rack header is
    // actually NON-shifting (despite the _SFT_ name) — the original
    // CV-mode _SFT_ macro shifted, but the rack _NOCV_ variant in
    // ctagSoundProcessorPicoSeqRack.hpp is identical to ABS_NOCV.
    // Compute the bipolar shift manually here so knob mid (cv=2048)
    // → 0, knob ends (cv=0 / cv=4095) → ±scale.
    const float fEGFM     = (eg2fm     - 2048.f) / 2048.f * 12.f;
    const float fEGFMFilt = (eg2filtfm - 2048.f) / 2048.f * 1.f;
    const float fEGWave   = (eg2wave   - 2048.f) / 2048.f * 1.f;
    valADSR = adsr.Process();

    // modulation LFO
    MK_FLT_PAR_ABS_NOCV(fLFOSpeed, lfospeed, 4095.f, 20.f)
    MK_BOOL_PAR_NOCV(bLFOSync, lfosync)

    bool trigger = preGate != midi_trig && midi_trig;

    // LFO frequency / phase: in Sync mode, hard-reset phase on every
    // fresh noteOn (so the LFO is in lockstep with note attacks); in
    // free-running mode, follow the LFO Spd knob continuously so the
    // user can sweep speed while a note is held. This matches the
    // original ctagSoundProcessorWTOsc::Process pattern — my earlier
    // version only called SetFrequency inside the trigger branch,
    // which meant the LFO Spd knob had no effect during held notes.
    if (bLFOSync && trigger) {
        lfo.SetFrequencyPhase(fLFOSpeed, 0.f);
    } else {
        lfo.SetFrequency(fLFOSpeed);
    }

    preGate = midi_trig;

    MK_FLT_PAR_ABS_NOCV(fLFOAM, lfo2am, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(fLFOFM, lfo2fm, 4095.f, 12.f)
    MK_FLT_PAR_ABS_NOCV(fLFOFMFilt, lfo2filtfm, 4095.f, 1.f);
    MK_FLT_PAR_ABS_NOCV(fLFOWave, lfo2wave, 4095.f, 1.f)
    valLFO = lfo.Process();

    // pitch / tuning / FM
    int32_t ipitch = 0;
    ipitch += static_cast<int32_t>(midi_note * 128.0f);
    int32_t sc = q_scale * 48 / 4096;
    CONSTRAIN(sc, 0, 47);

    float fPitch = static_cast<float>(ipitch);
    fPitch /= 128.f;
    MK_FLT_PAR_ABS_SFT_NOCV(fTune, tune, 2048.f, 1.f)
    const float f0 = plaits::NoteToFrequency(fPitch + fTune * 12.f + fLFOFM * valLFO + fEGFM * valADSR) * 0.998f;

    // filter
    MK_FLT_PAR_ABS_NOCV(fCut, fcut, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(fReso, freso, 4095.f, 20.f)
    fCut = fCut + fEGFMFilt * valADSR + fLFOFMFilt * valLFO;
    CONSTRAIN(fCut, 0.f, 1.f)
    CONSTRAIN(fReso, 1.f, 20.f)
    fCut = 20.f * stmlib::SemitonesToRatio(fCut * 120.f);
    svf.set_f_q<stmlib::FREQUENCY_FAST>(fCut / 44100.f, fReso);
    MK_INT_PAR_ABS_NOCV(iFType, fmode, 4.f)
    CONSTRAIN(iFType, 0, 3);

    // Master amplitude: ADSR envelope is the single source of truth.
    // valADSR comes from the internal ctagADSREnv driven by Attack/
    // Decay/Sustain/Release knobs — full A/D/S/R shape on the audible
    // amplitude, exactly as the four labeled knobs imply. LFO AM
    // modulates it; Gain attenuates the result. The legacy bipolar
    // fEGAM blend (which made amplitude shape-independent of ADSR
    // when the EG-AM amount knob was at 0 — the original "sustains
    // forever" bug) is gone.
    //
    // Velocity scaling: pending_velocity (0..127) modulates amplitude
    // linearly so per-step velocity dynamics are honored.
    const float fVel = pending_velocity / 127.f;
    float fAM = valADSR * fVel;
    fAM *= (1.f - (valLFO + 1.f) * 0.5f * fLFOAM);
    fAM *= fGain;
    CONSTRAIN(fAM, 0.f, 1.f)

    float fWt = fWave + valADSR * fEGWave + valLFO * fLFOWave * 2.f;
    CONSTRAIN(fWt, 0.f, 1.f)

    // calc wave and apply filter
    if (isWaveTableGood) {
        oscillator.Render(trigger, f0, fAM, fWt, wavetables, out, BUF_SZ);

        switch (iFType) {
            case 1:
                svf.Process<stmlib::FILTER_MODE_LOW_PASS>(out, out, BUF_SZ);
                break;
            case 2:
                svf.Process<stmlib::FILTER_MODE_BAND_PASS>(out, out, BUF_SZ);
                break;
            case 3:
                svf.Process<stmlib::FILTER_MODE_HIGH_PASS>(out, out, BUF_SZ);
            default:
                break;
        }
    }
}
