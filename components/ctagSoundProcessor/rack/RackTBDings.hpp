/***************
TBD-16 — Macro/Preset System & PicoSeqRack
RackTBDings: FaseAcht-inspired Rings (Mutable Instruments) wrapper for the rack.

(c) 2026 dadamachines / Johannes Lohbihler. https://dadamachines.com

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

Provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#pragma once

#include "RackSynth.hpp"
#include "rings/dsp/part.h"
#include "rings/dsp/strummer.h"
#include "helpers/ctagADEnv.hpp"

using namespace CTAG::SP;

class RackTBDings {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
    void noteOn(uint8_t note, uint8_t vel);
    void noteOff(uint8_t note, uint8_t vel);

    bool enabled {false};
    // FaseAcht §4.6 Mod Polyphonic mode reads its own midi_note for pitch-relative AM.
    // Stereo output: out = main, out+1 = aux (Rings string-synth secondary).
    float tbd_out_stereo[BUF_SZ * 2] {};
    // The host AIR master CC fan-out writes here directly. Atomic so writes from
    // the host CC handler (audio thread) are visible to Process (audio thread).
    atomic<int16_t> air_blend {0};

private:
    // Rings DSP allocated in Init.
    //   - reverb_buffer: 64 KB SPIRAM
    //   - part / strummer: heap-allocated in PSRAM via placement-new so they
    //     don't bloat the PicoSeqRack object (which lives in a fixed-size
    //     ctagSPAllocator block — adding rings::Part inline overflows it).
    uint16_t *reverb_buffer {nullptr};
    rings::Part *part {nullptr};
    rings::Strummer *strummer {nullptr};
    rings::Patch patch {};
    rings::PerformanceState performance_state {};
    rings::ResonatorModel last_model {rings::RESONATOR_MODEL_MODAL};
    int last_polyphony {1};

    HELPERS::ctagADEnv paramAD;

    // Note state — only midi_note is needed cross-block; pending_strum + velocity
    // are consumed once per Process call.
    int midi_note {60};
    float midi_freq_hz {261.63f};
    bool note_held {false};
    bool pending_strum {false};
    // pending_velocity defaults to 0 so the wavefolder is fully bypassed
    // until the first real noteOn. Without this, vel_amount default + a
    // non-zero default velocity would drive Rings transients into the
    // fold path before any audio is musically expected.
    uint8_t pending_velocity {0};
    int16_t prev_pluck_cc {0}; // edge detector for the Pluck CC

    // Tremolo / AM phase — single sine accumulator in [0, 2π).
    float mod_phase {0.f};
    // §4.6 AM Pitch Env state — on each strum, pitch_env_ratio jumps to 2.0
    // and decays exponentially toward 1.0. Decay rate scales with mod_rate.
    float pitch_env_ratio {1.f};

    // ---- FaseAcht-inspired params (CCs 8..24) ----
    // Defaults are set to the cv-space values the wrapper sees AFTER the preset
    // pushes (preset value × 4096 / 16384 for nrpm). This way, even if the
    // macro engine hasn't pushed preset values yet at first audio block, the
    // wrapper's initial Process call produces musically-correct output.
    atomic<int16_t> reson_model {0};
    atomic<int16_t> freq_par {2048};      // preset 8192 → cv 2048 → no detune
    atomic<int16_t> structure {2048};     // preset 8192 → cv 2048 → harmonic-notch mid
    atomic<int16_t> brightness {2500};    // preset 10000 → cv 2500 → past LP transition (musical)
    atomic<int16_t> damping {1700};       // preset 7000 → cv 1700 → ~1.5 s decay (sequencer-friendly)
    atomic<int16_t> position {1225};      // preset 4900 → cv 1225 → off-center (rich harmonics)
    atomic<int16_t> chord_par {0};
    atomic<int16_t> poly_par {1920};      // preset 1 (cc src) × mul 60 → wire 60 → cv 1920 → 2-voice duo
    atomic<int16_t> easter_par {0};
    atomic<int16_t> env_shape {1000};     // preset 4000 → cv 1000 → short envelope
    atomic<int16_t> vel_amount {2000};    // preset 8000 → cv 2000 → moderate drive
    // air_blend is public so the host AIR master CC fans into it.
    atomic<int16_t> mod_type {0};         // 0=Tremolo, 1=AM Pitch Env, 2=AM Poly
    atomic<int16_t> mod_depth {0};        // §4.6 global DEPTH
    atomic<int16_t> mod_rate {2000};      // preset 8000 → cv 2000 → mid
    atomic<int16_t> mod_snap {0};         // §4.6 QUANTIZE harmonic snap toggle
    atomic<int16_t> pluck_cc {0};         // §5.4 PLUCK trigger CC (rising edge)

    // Per-instance LCG for AIR noise (RT-safe — no shared state, no malloc).
    uint32_t rng_state {0x9E3779B9};

    // Silence gate — counts blocks since the engine last had any reason to
    // produce audio (note held, recent strum, active envelope, AIR turned up).
    // After kSilenceTailBlocks of pure silence we skip Rings entirely and
    // emit zeros, so the plugin can't drone or hum without the sequencer
    // explicitly asking for audio.
    int silence_tail_blocks {0};
};
