/***************
TBD-16 — Macro/Preset System & PicoSeqRack
RackTBDaits: Plaits (Mutable Instruments) extended-engine wrapper for the rack.

(c) 2026 dadamachines / Johannes Lohbihler. https://dadamachines.com
Licensed under the GNU Lesser General Public License (LGPL 3.0).
***************/

#include "RackSynth.hpp"
#include "RackTBDaits.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"
#include "stmlib/dsp/dsp.h"
#include "stmlib/utils/buffer_allocator.h"
#include "esp_heap_caps.h"
#include <cmath>
#include <new>

using namespace CTAG::SP;

// Plaits Voice::Init expects a 16 KB scratch workspace via stmlib::BufferAllocator.
// All 24 engines share this buffer (allocator->Free() between Init calls).
static constexpr size_t kPlaitsAllocatorBytes = 16384;

// Silence-gate tail. Same reasoning as RackTBDings: ~5 s at 1378 Hz block rate
// covers the longest LPG decay tail before we hard-mute. Plaits' drone-style
// engines (Noise / Speech / Chord) never naturally fall silent without LPG, so
// this is the only thing keeping an idle-but-enabled track from humming.
static constexpr int kSilenceTailBlocks = 7000;

void RackTBDaits::Init(const PickSeqRackInitData *initdata) {
    // Init runs once at PicoSeqRack construction, before the audio task starts.
    // heap_caps_malloc here is safe.
    if (!shared_buffer) {
        shared_buffer = (char*)heap_caps_malloc(kPlaitsAllocatorBytes, MALLOC_CAP_SPIRAM);
        assert(shared_buffer != nullptr);
    }
    if (!voice) {
        void *mem = heap_caps_malloc(sizeof(plaits::Voice), MALLOC_CAP_SPIRAM);
        assert(mem != nullptr);
        voice = new (mem) plaits::Voice();
    }

    {
        stmlib::BufferAllocator allocator(shared_buffer, kPlaitsAllocatorBytes);
        voice->Init(&allocator);
    }

    // Sensible patch defaults (mid-range Plaits, DX7A engine).
    patch.engine = 2;       // Six-Op DX7 instance A
    patch.note = 60.f;
    patch.harmonics = 0.5f;
    patch.timbre = 0.5f;
    patch.morph = 0.5f;
    patch.frequency_modulation_amount = 0.f;
    patch.timbre_modulation_amount = 0.f;
    patch.morph_modulation_amount = 0.f;
    patch.decay = 0.6f;
    patch.lpg_colour = 0.5f;

    // Modulation source patching: leave all *_patched=false so Plaits uses its
    // internal decay envelope as the modulation source for FMod/TMod/MMod —
    // which is the musically expected behavior (envelope opens / closes the
    // modulation per note, scaled by the *_amount knobs).
    modulations.engine = 0.f;
    modulations.note = 0.f;
    modulations.frequency = 0.f;
    modulations.harmonics = 0.f;
    modulations.timbre = 0.f;
    modulations.morph = 0.f;
    modulations.trigger = 0.f;
    modulations.level = 0.7f;
    modulations.frequency_patched = false;
    modulations.timbre_patched = false;
    modulations.morph_patched = false;
    modulations.trigger_patched = true;   // we drive the trigger explicitly
    modulations.level_patched = true;     // we drive level via the Level knob

    // ---- CC registrations (8..18) ----
    // Sequential ctrls; idx == ctrl - 8 invariant holds across all 11 params.
    initdata->rack->registerParamAndCC(initdata, "model",  8,  [&](int v){ model_par = v; });
    initdata->rack->registerParamAndCC(initdata, "freq",   9,  [&](int v){ freq_par = v; });
    initdata->rack->registerParamAndCC(initdata, "harm",   10, [&](int v){ harm_par = v; });
    initdata->rack->registerParamAndCC(initdata, "timbre", 11, [&](int v){ timbre_par = v; });
    initdata->rack->registerParamAndCC(initdata, "morph",  12, [&](int v){ morph_par = v; });
    initdata->rack->registerParamAndCC(initdata, "decay",  13, [&](int v){ decay_par = v; });
    initdata->rack->registerParamAndCC(initdata, "color",  14, [&](int v){ color_par = v; });
    initdata->rack->registerParamAndCC(initdata, "level",  15, [&](int v){ level_par = v; });
    initdata->rack->registerParamAndCC(initdata, "fmod",   16, [&](int v){ fmod_par = v; });
    initdata->rack->registerParamAndCC(initdata, "tmod",   17, [&](int v){ tmod_par = v; });
    initdata->rack->registerParamAndCC(initdata, "mmod",   18, [&](int v){ mmod_par = v; });

    enabled = false;
}

void RackTBDaits::noteOn(uint8_t note, uint8_t vel) {
    midi_note = note;
    pending_velocity = vel ? vel : 100;
    note_held = true;
    // Force a 0→1 trigger edge over 2 blocks. Even if note_held was already
    // true (repeated same-note triggers from sequencer), the 1-block low
    // floor guarantees Plaits' internal trigger detector sees a fresh edge.
    trigger_pulse_state = 2;
}

void RackTBDaits::noteOff(uint8_t note, uint8_t vel) {
    (void)note;
    (void)vel;
    note_held = false;
}

void RackTBDaits::Process(const PicoSeqRackProcessData &data) {
    (void)data;
    if (!enabled) return;

    // Snapshot atomics once per block.
    const int i_model  = model_par;
    const int i_freq   = freq_par;
    const int i_harm   = harm_par;
    const int i_timbre = timbre_par;
    const int i_morph  = morph_par;
    const int i_decay  = decay_par;
    const int i_color  = color_par;
    const int i_level  = level_par;
    const int i_fmod   = fmod_par;
    const int i_tmod   = tmod_par;
    const int i_mmod   = mmod_par;

    // ---- SILENCE GATE -----------------------------------------------------
    // Active iff: note held, pulse pending, or recent trigger still ringing.
    // Without this, Plaits drone-style engines (Noise / Chord / Speech /
    // Modal at high decay) keep humming forever even when the sequencer is
    // silent — defeats the sequencer's role as the audible articulator.
    const bool any_activity = note_held
                           || (trigger_pulse_state != 0);
    if (any_activity) {
        silence_tail_blocks = 0;
    } else {
        silence_tail_blocks++;
        if (silence_tail_blocks > kSilenceTailBlocks) {
            std::fill_n(aits_out_stereo, BUF_SZ * 2, 0.f);
            return;
        }
    }
    // ----------------------------------------------------------------------

    // Engine selection — 24 engines mapped 1:1 to encoder positions 0..23.
    // The macro layer fans encoder pos via mul=5 → wire 0..115 → cv 0..3680
    // (not full 0..4095). The standard (cv*N)/4096 bucketing would lose the
    // top two engines because cv never reaches 4064. Recover the encoder
    // position directly: cv = encoder_pos * 32 (macro res scaling) * 5
    // (macro mul) = encoder_pos * 160. Divide by 160 to invert.
    //   ⚠ This divisor is paired with the macro JSON's mul=5. If you change
    //   one, change the other. See tbdait-allparams.json mapping ctrl 8.
    {
        int e = i_model / 160;
        if (e < 0) e = 0;
        if (e >= plaits::kMaxEngines) e = plaits::kMaxEngines - 1;
        patch.engine = e;
    }

    // Note pitch — fine-tune detune ±2 semitones around the played MIDI note.
    // Mid-scale (cv ≈ 2048) = 0 detune.
    {
        float bias_semis = (i_freq / 4095.f - 0.5f) * 4.f;
        patch.note = (float)midi_note + bias_semis;
        CONSTRAIN(patch.note, 0.f, 96.f);
    }

    // Macro tone params — direct 0..1 mapping.
    patch.harmonics = i_harm / 4095.f;
    patch.timbre    = i_timbre / 4095.f;
    patch.morph     = i_morph / 4095.f;
    patch.decay     = i_decay / 4095.f;
    patch.lpg_colour = i_color / 4095.f;
    CONSTRAIN(patch.harmonics, 0.f, 1.f);
    CONSTRAIN(patch.timbre,    0.f, 1.f);
    CONSTRAIN(patch.morph,     0.f, 1.f);
    CONSTRAIN(patch.decay,     0.f, 1.f);
    CONSTRAIN(patch.lpg_colour, 0.f, 1.f);

    // Modulation amounts — Plaits expects -1..+1 (signed). Map our 0..1 cv
    // to 0..1 positive depth, leaving negative depths inaccessible (they
    // sound like inverted versions of the positive direction; one knob
    // direction is enough for sequencer-driven use).
    patch.frequency_modulation_amount = i_fmod / 4095.f;
    patch.timbre_modulation_amount    = i_tmod / 4095.f;
    patch.morph_modulation_amount     = i_mmod / 4095.f;
    CONSTRAIN(patch.frequency_modulation_amount, -1.f, 1.f);
    CONSTRAIN(patch.timbre_modulation_amount,    -1.f, 1.f);
    CONSTRAIN(patch.morph_modulation_amount,     -1.f, 1.f);

    // Trigger pulse state machine — see noteOn for rationale.
    //   2 → emit trigger=0 this block, decrement to 1
    //   1 → emit trigger=1 this block, decrement to 0
    //   0 → steady state: trigger = note_held ? 1 : 0
    float trig_value;
    if (trigger_pulse_state == 2) {
        trig_value = 0.f;
        trigger_pulse_state = 1;
    } else if (trigger_pulse_state == 1) {
        trig_value = 1.f;
        trigger_pulse_state = 0;
    } else {
        trig_value = note_held ? 1.f : 0.f;
    }
    modulations.trigger = trig_value;

    // Level knob — scaled by note-on velocity. Velocity defaults to 100/127
    // when no explicit note has fired (e.g. sequencer triggers without vel
    // info), so the patch is audible at first strike.
    const float level_norm = (i_level / 4095.f) * (pending_velocity / 127.f);
    modulations.level = level_norm;
    CONSTRAIN(modulations.level, 0.f, 1.f);

    // Render one block.
    plaits::Voice::Frame frames[BUF_SZ];
    voice->Render(patch, modulations, frames, BUF_SZ);

    // Convert int16 frames to float stereo, soft-clip, NaN/Inf guard.
    // Frame::out / Frame::aux are int16 in [-32767, +32767]; divide by 32768.
    for (int i = 0; i < BUF_SZ; i++) {
        float l = frames[i].out / 32768.f;
        float r = frames[i].aux / 32768.f;
        if (!std::isfinite(l)) l = 0.f;
        if (!std::isfinite(r)) r = 0.f;
        aits_out_stereo[i * 2 + 0] = stmlib::SoftClip(l);
        aits_out_stereo[i * 2 + 1] = stmlib::SoftClip(r);
    }
}
