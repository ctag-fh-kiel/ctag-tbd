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

    // Modulation source patching:
    //   *_patched=false for FM/Timbre/Morph → Plaits' ApplyModulations falls
    //   back to its internal decay envelope as the modulation source, scaled
    //   by the *_amount knobs (musically expected envelope-driven mod).
    //   trigger_patched=true → we drive trigger explicitly per note.
    //   level_patched is set per-block in Process based on the active engine
    //   (DX7 instances get level_patched=true so the LEVEL knob acts as
    //   per-patch velocity per the Plaits manual; all other engines get
    //   level_patched=false so the trigger strikes the LPG and Decay knob
    //   audibly shapes the note tail).
    modulations.engine = 0.f;
    modulations.note = 0.f;
    modulations.frequency = 0.f;
    modulations.harmonics = 0.f;
    modulations.timbre = 0.f;
    modulations.morph = 0.f;
    modulations.trigger = 0.f;
    modulations.level = 0.f;
    modulations.frequency_patched = false;
    modulations.timbre_patched = false;
    modulations.morph_patched = false;
    modulations.trigger_patched = true;
    modulations.level_patched = false;    // overwritten per-block for DX7 engines

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
    // Re-attack the AHR envelope to the new velocity unconditionally.
    // Without this snap-to-velocity, a forte note followed by a piano note
    // (held key, sequencer step, or repeated keyboard strike) would
    // continue at the higher amplitude — masking the user's velocity
    // intent. Critical for expressive keyboard play and per-step velocity
    // dynamics in the sequencer.
    env_value = pending_velocity / 127.f;
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
    // Active iff: note held, pulse pending, or AHR envelope still ringing
    // above -60 dB. The env_value > 0.001 check keeps audio flowing during
    // the natural release tail (sequencer-fit and keyboard release feel)
    // AND keeps drone mode (Decay at max → release_factor=1) sustaining
    // indefinitely until the user releases the key + the env actually
    // decays. Without it, drone-style engines (Noise / Chord / Speech /
    // Modal at high decay) keep humming and the silence gate could prematurely
    // mute audible release tails.
    const bool any_activity = note_held
                           || (trigger_pulse_state != 0)
                           || (env_value > 0.001f);
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
    //   one, change the other. See tbdait-params.json mapping ctrl 8.
    int active_engine;
    {
        int e = i_model / 160;
        if (e < 0) e = 0;
        if (e >= plaits::kMaxEngines) e = plaits::kMaxEngines - 1;
        patch.engine = e;
        active_engine = e;
    }

    // AHR-envelope-driven modulations.level (level_patched=true).
    // Plaits hardware default = "LEVEL CV unpatched" → ProcessPing → trigger
    // strikes LPG → Decay shapes tail. That model assumes an external CV
    // shapes the note when LEVEL IS patched, and treats the Level knob as
    // a static post-Render multiplier when not.
    //
    // Our environment is different from Plaits hardware:
    //   - No CV inputs — Level is a static knob.
    //   - Users include both step-sequencer and live-keyboard players.
    //   - Keyboard players expect held notes to sustain and release on
    //     key-up (ProcessPing alone ignores note length).
    //   - DX7 patches use accent (= compressed_level when level_patched=true)
    //     for per-patch velocity-to-timbre routing — a real expressive
    //     feature for keyboard play that's lost without ProcessLP.
    //
    // Solution: synthesize a wrapper-side AHR envelope (instant attack,
    // hold while note_held, exponential release per Decay knob) and feed
    // it into modulations.level. Then ProcessLP follows it → LPG audibly
    // shapes the tail per Decay, AND the engine accent sees envelope-
    // shaped velocity → DX7 patches respond expressively. Single code path
    // works for all 24 engines.
    (void)active_engine;
    modulations.level_patched = true;
    {
        // Target: peak (velocity-scaled) while held, zero on release.
        const float target = note_held ? (pending_velocity / 127.f) : 0.f;
        if (env_value < target) {
            // Instant attack — sequencer / keyboard expect immediate response.
            env_value = target;
        } else {
            // Exponential release. tau (seconds) maps from Decay knob via
            // the same Plaits curve as PT_PLAITS_DECAY: 5 ms..8 s.
            //   tau = 0.005 * 1600^(decay_norm)
            //   per-block decay factor = exp(-block_time / tau)
            //   block_time = BUF_SZ / sample_rate = 32 / 44100 ≈ 0.000726 s
            // Drone mode: at the very top of the Decay knob (top ~0.5 %),
            // release_factor is forced to 1.0 → infinite sustain. Lets the
            // user explicitly opt into drone / pad textures without limiting
            // the released-note tail elsewhere on the knob. The silence
            // gate's env_value > threshold check keeps audio flowing in
            // drone mode (handled in the gate block above-block).
            float release_factor;
            if (i_decay >= 4080) {
                release_factor = 1.f;
            } else {
                const float decay_norm = i_decay / 4095.f;
                const float tau_s = 0.005f * powf(1600.f, decay_norm);
                release_factor = expf(-0.000726f / tau_s);
            }
            env_value = target + (env_value - target) * release_factor;
        }
        if (env_value < 0.f) env_value = 0.f;
        if (env_value > 1.f) env_value = 1.f;
    }
    modulations.level = env_value;

    // Note pitch — bias ±12 semitones (1 octave) around the played MIDI note.
    // Mid-scale (cv ≈ 2048) = 0 detune. The Plaits manual documents an
    // 8-octave default range; ±12 is the sequencer-friendly middle ground —
    // wide enough for octave transposition / creative repitching, fine
    // enough that hi-res NRPM (16384 steps) gives ~0.0015 semi precision.
    {
        float bias_semis = (i_freq / 4095.f - 0.5f) * 24.f;
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

    // Level knob → post-Render master output gain. Velocity is NOT applied
    // here — it's already inside env_value (which feeds modulations.level
    // → LPG, which scales the engine output). Applying velocity twice would
    // square the dynamic range.
    const float master_gain = i_level / 4095.f;

    // Render one block.
    plaits::Voice::Frame frames[BUF_SZ];
    voice->Render(patch, modulations, frames, BUF_SZ);

    // Convert int16 frames to float stereo, apply master gain, soft-clip,
    // NaN/Inf guard. Frame::out / Frame::aux are int16 in [-32767, +32767];
    // divide by 32768.
    for (int i = 0; i < BUF_SZ; i++) {
        float l = (frames[i].out / 32768.f) * master_gain;
        float r = (frames[i].aux / 32768.f) * master_gain;
        if (!std::isfinite(l)) l = 0.f;
        if (!std::isfinite(r)) r = 0.f;
        aits_out_stereo[i * 2 + 0] = stmlib::SoftClip(l);
        aits_out_stereo[i * 2 + 1] = stmlib::SoftClip(r);
    }
}
