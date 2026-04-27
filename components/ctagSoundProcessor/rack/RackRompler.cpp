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
#include "RackRompler.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"

using namespace CTAG::SP;

void RackRompler::Init(const PickSeqRackInitData *initdata) {
    rompler.Init(44100.f);

    initdata->rack->registerParamAndCC(initdata, "bank", 8, [&](const int val){ s1_bank = val;});
    initdata->rack->registerParamAndCC(initdata, "slice", 9, [&](const int val) { s1_slice = val; });
    initdata->rack->registerParamAndCC(initdata, "start", 10, [&](const int val){ s1_start = val;});
    initdata->rack->registerParamAndCC(initdata, "end", 11, [&](const int val) { s1_end = val; });

    initdata->rack->registerParamAndCC(initdata, "fc", 12, [&](const int val){ s1_fc = val;});
    initdata->rack->registerParamAndCC(initdata, "fq", 13, [&](const int val){ s1_fq = val;});
    initdata->rack->registerParamAndCC(initdata, "ft", 14, [&](const int val){ s1_ft = val;});
    initdata->rack->registerParamAndCC(initdata, "brr", 15, [&](const int val){ s1_brr = val;});

    initdata->rack->registerParamAndCC(initdata, "atk", 16, [&](const int val){ s1_atk = val;});
    initdata->rack->registerParamAndCC(initdata, "dcy", 17, [&](const int val){ s1_dcy = val;});
    initdata->rack->registerParamAndCC(initdata, "speed", 18, [&](const int val){ s1_speed = val;});
    initdata->rack->registerParamAndCC(initdata, "pitch", 19, [&](const int val){ s1_pitch = val;});

    initdata->rack->registerParamAndCC(initdata, "lp", 20, [&](const int val){ s1_lp = val;});
    initdata->rack->registerParamAndCC(initdata, "lp_pp", 21, [&](const int val){ s1_lp_pp = val;});
    initdata->rack->registerParamAndCC(initdata, "lp_pos", 22, [&](const int val){ s1_lp_pos = val;});
    initdata->rack->registerParamAndCC(initdata, "eg2fm", 23, [&](const int val){ s1_eg2fm = val;});

    initdata->rack->registerParamAndCC(initdata, "tsmode", 24, [&](const int val){ s1_tsmode = val;});
    initdata->rack->registerParamAndCC(initdata, "tsamount", 25, [&](const int val){ s1_tsamount = val;});

    s1_lp = 0;
    s1_lp_pp = 0;

    this->enabled = false;
}

void RackRompler::noteOn(uint8_t note, uint8_t vel) {
    midi_trig = true;
    midi_note = note;
    midi_freq = 440.f * powf(2.f, (note - 69) / 12.f);
    note_held = true;
}

void RackRompler::noteOff(uint8_t note, uint8_t vel) {
    // Just track the held state. Don't Reset() the voice here —
    // for short step gates the noteOff fires only milliseconds
    // after noteOn, which would silence the loop before the user
    // can hear it. The actual loop-stop decision is made in
    // Process() via the trig-age safety net (see below).
    note_held = false;
}

void RackRompler::Process(const PicoSeqRackProcessData &data) {
    if (!this->enabled) {
        return;
    }

    std::fill_n(s1_out, BUF_SZ, 0.f);

    // Timestretch enable — read the atomic directly so a wire value of 1
    // (the natural macro `max: 1, ui: onoff` toggle) engages timestretch.
    // The previous MK_INT_PAR_ABS_NOCV / 4096 * 2 path required wire ≥ 64
    // to produce bTSMode ≥ 1, so onoff toggles never engaged. See
    // tbd-pico-seq3/docs/architecture/rompler-april-2026.md § 2.3.
    rompler.params.timeStretchEnable = (s1_tsmode.load() > 0);

    // timestretch target length
    // MK_INT_PAR_NOCV(ts_track_length, track_length, 128);

    uint32_t firstNonWtSlice = data.firstNonWtSlice;
    MK_INT_PAR_ABS_NOCV(iS1Bank, s1_bank, 128.f)
    CONSTRAIN(iS1Bank, 0, 31)
    MK_INT_PAR_ABS_NOCV(iS1Slice, s1_slice, 128.f) // midi cc
    CONSTRAIN(iS1Slice, 0, 31)
    iS1Slice = iS1Bank * 32 + iS1Slice + firstNonWtSlice;
    rompler.params.slice = iS1Slice;

    // Periodic diagnostic
    static uint32_t _romplerDiagCtr = 0;
    if ((_romplerDiagCtr++ % 50000) == 0) {
        printf("DIAG RackRompler: bank=%d slice=%d abs=%d firstNonWt=%lu s1_bank=%d s1_slice=%d\n",
               iS1Bank, (int)(iS1Slice - iS1Bank * 32 - firstNonWtSlice), iS1Slice, firstNonWtSlice,
               (int)s1_bank.load(), (int)s1_slice.load());
    }

    MK_FLT_PAR_ABS_NOCV(fS1Speed, s1_speed, 4095.f, 2.f)
    CONSTRAIN(fS1Speed, 0.f, 2.f)
    // playbackSpeed is only set at note trigger to preserve it between frames

    // Pitch knob is bipolar around wire 64. Per the lead-dev spec the
    // upstream s1_pitch range is -120..+120 tenths-of-semitones (= ±1
    // octave). Our wire is unsigned 0..127 → cv 0..4064 → centre 2048.
    // Map (cv - 2048) / 2048 * 12 → -12..+12 semitones.
    //
    // DO NOT add midi_note: that produced the lead-dev-reported
    // "values >59 are always max pitch" clip (RomplerVoiceMinimal's
    // phaseIncrement clamp at 6× ≈ 31 ST means wire ≥ 95 - midi_note
    // always clipped, with default midi_note=36 giving the threshold
    // at wire 59). See tbd-pico-seq3/docs/architecture/rompler-april-2026.md
    // § 2.1.
    //
    // Negative pitch (wire < 64) routes through Speed's sign at note-
    // trigger to drive reverse playback. RomplerVoiceMinimal already
    // supports BWD via `phaseIncrement < 0`; the wrapper just never
    // sent negative values. See § 2.2.
    float fS1PitchSemi = ((float)s1_pitch.load() - 2048.f) / 2048.f * 12.f;
    CONSTRAIN(fS1PitchSemi, -12.f, 12.f)
    if (rompler.params.timeStretchEnable) {
        rompler.params.pitch = fS1PitchSemi / 10.0;
    } else {
        rompler.params.pitch = fabsf(fS1PitchSemi);   // magnitude → SemitonesToRatio
    }

    MK_FLT_PAR_ABS_NOCV(fS1Start, s1_start, 4095.f, 1.f)
    rompler.params.startOffsetRelative = fS1Start;
    MK_FLT_PAR_ABS_NOCV(fS1Length, s1_end, 4095.f, 1.f)
    rompler.params.lengthRelative = fS1Length;
    MK_FLT_PAR_ABS_NOCV(fS1LoopPos, s1_lp_pos, 4095.f, 1.f)
    rompler.params.loopMarker = fS1LoopPos;
    MK_BOOL_PAR_NOCV(bS1Loop, s1_lp)
    rompler.params.loop = bS1Loop;
    MK_BOOL_PAR_NOCV(bS1LoopPipo, s1_lp_pp)
    rompler.params.loopPiPo = bS1LoopPipo;
    MK_FLT_PAR_ABS_NOCV(fS1Attack, s1_atk, 4095.f, 2.f)
    if (fS1Attack < 0.001f) fS1Attack = 0.001f; // prevent div-by-zero in AD envelope
    rompler.params.a = fS1Attack;
    MK_FLT_PAR_ABS_NOCV(fS1Decay, s1_dcy, 4095.f, 50.f)
    if (fS1Decay < 0.01f) fS1Decay = 0.01f; // prevent div-by-zero in AD envelope
    rompler.params.d = fS1Decay;
    MK_FLT_PAR_ABS_SFT_NOCV(fS1EGFM, s1_eg2fm, 4095.f, 12.f)
    rompler.params.egFM = fS1EGFM;
    MK_INT_PAR_ABS_NOCV(iS1Brr, s1_brr, 16)
    CONSTRAIN(iS1Brr, 0, 14)
    rompler.params.bitReduction = iS1Brr;
    // filter params
    MK_FLT_PAR_ABS_NOCV(fS1Cut, s1_fc, 4095.f, 1.f)
    rompler.params.cutoff = fS1Cut;
    MK_FLT_PAR_ABS_NOCV(fS1Reso, s1_fq, 4095.f, 10.f)
    rompler.params.resonance = fS1Reso;
    MK_INT_PAR_ABS_NOCV(iS1FType, s1_ft, 4.f)
    CONSTRAIN(iS1FType, 0, 3);
    // timestretch stuff


    MK_FLT_PAR_ABS_NOCV(fTSAmount, s1_tsamount, 4095.f, 1.f)
    float fTS1Amount = 0.001f + fTSAmount * 0.998f;
    rompler.params.timeStretchWindowSize = fTS1Amount;

    // MK_BOOL_PAR_NOCV(bGateS1, s1_gate)
    rompler.params.gate = midi_trig;
    if (midi_trig && !trig_prev) {
        uint32_t sliceLength = 0;
        uint32_t stepsLengthMs = 0;
        uint32_t sliceLengthMs = 0;

        // When time-stretch is ON the engine overrides playbackSpeed with the
        // slice-to-step-length ratio so the sample fills N steps regardless of
        // its natural duration. When time-stretch is OFF, honour the Speed
        // knob (fS1Speed ∈ [0..2]; 1.0 = natural pitch). Previously the
        // non-timestretch branch hardcoded 1.0 and the Speed knob was a dead
        // control — TODO.md bug fix.
        if (rompler.params.timeStretchEnable
            && data.sampleRom->HasSlice(rompler.params.slice)) {
            sliceLength = data.sampleRom->GetSliceSize(rompler.params.slice);
            stepsLengthMs = track_length * data.msPerBeat / 4;
            sliceLengthMs = (sliceLength * 1000) / 44100;
            rompler.params.playbackSpeed = (float)sliceLengthMs / (float)stepsLengthMs;
        } else {
            // Pitch sign drives playback direction. RomplerVoiceMinimal
            // chooses FWD vs BWD from the sign of phaseIncrement
            // (= playbackSpeed * pitchFactor). pitchFactor is always
            // positive (SemitonesToRatio of |fS1PitchSemi|), so we
            // route the sign through Speed.
            rompler.params.playbackSpeed = (fS1PitchSemi < 0.f) ? -fS1Speed : fS1Speed;
        }

        // printf("S1 sl=%ld ps=%1.3f pitch=%1.3f, ts=%d>%1.1f, slicelen=%ld,msperbeat=%ld,slicelenms=%ld, tempo=%ld,tracklen=%d\n",
        //     rompler.params.slice,
        //     rompler.params.playbackSpeed,
        //     rompler.params.pitch,
        //     rompler.params.timeStretchEnable,
        //     rompler.params.timeStretchWindowSize,
        //     sliceLength,
        //     data.msPerBeat,
        //     sliceLengthMs,
        //     data.tempo,
        //     track_length
        // );

        // printf("S2 slice=%ld ps=%1.1f pitch=%1.1f startoffrel=%1.1f lengthrel=%1.1f\n",
        //     rompler.params.slice,
        //     rompler.params.playbackSpeed,
        //     rompler.params.pitch,
        //     rompler.params.startOffsetRelative,
        //     rompler.params.lengthRelative);

        // printf("S3 a=%1.1f d=%1.1f fc=%1.1f fq=%1.1f ft=%d\n",
        //     (float)rompler.params.a,
        //     (float)rompler.params.d,
        //     (float)rompler.params.cutoff,
        //     (float)rompler.params.resonance,
        //     (int)rompler.params.filterType);

        // printf("S4 lo=%d pipo=%d lm=%1.1f egfm=%1.1f bitred=%ld gate=%d\n",
        //     rompler.params.loop,
        //     rompler.params.loopPiPo,
        //     (float)rompler.params.loopMarker,
        //     (float)rompler.params.egFM,
        //     rompler.params.bitReduction,
        //     rompler.params.gate);
    }
    // Track time since the last rising-edge trigger. Reset to 0
    // on any tick that produced a trig, increments otherwise.
    if (midi_trig) {
        trig_age_ticks = 0;
    } else if (trig_age_ticks < UINT32_MAX) {
        trig_age_ticks++;
    }
    trig_prev = midi_trig;
    midi_trig = false;

    // Loop safety net: when loop or ping-pong is active, kill the
    // voice if it hasn't seen a fresh rising-edge trig in
    // LOOP_STALE_TICKS Process iterations. This catches the
    // sequencer-stop case (trigs stop coming → trig_age grows
    // unbounded → Reset fires). LOOP_STALE_TICKS chosen to be
    // longer than any reasonable held-step duration so a user
    // holding a step on a looping rompler track can keep hearing
    // the loop. ~5.8 s at 44.1 kHz / BUF_SZ=32. Beyond that the
    // user almost certainly stopped or moved on. Non-loop samples
    // are never touched — they end naturally on their AD tail.
    // Reset() is idempotent so the redundant calls per tick after
    // first stop are cheap.
    constexpr uint32_t LOOP_STALE_TICKS = 8192;
    const bool loopActive = (s1_lp.load() != 0) || (s1_lp_pp.load() != 0);
    if (loopActive && trig_age_ticks > LOOP_STALE_TICKS) {
        rompler.Reset();
    }

    rompler.params.filterType = static_cast<CTAG::SYNTHESIS::RomplerVoiceMinimal::Params::FilterType>(iS1FType);
    rompler.Process(s1_out, BUF_SZ);
};
