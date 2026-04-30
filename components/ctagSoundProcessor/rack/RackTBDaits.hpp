/***************
TBD-16 — Macro/Preset System & PicoSeqRack
RackTBDaits: Plaits (Mutable Instruments) extended-engine wrapper for the rack.

(c) 2026 dadamachines / Johannes Lohbihler. https://dadamachines.com

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

Provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#pragma once

#include "RackSynth.hpp"
#include "plaits/dsp/voice.h"

using namespace CTAG::SP;

class RackTBDaits {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
    void noteOn(uint8_t note, uint8_t vel);
    void noteOff(uint8_t note, uint8_t vel);

    bool enabled {false};
    // Stereo output: out = main engine output, out+1 = aux (Plaits aux channel —
    // most engines mirror or provide a related secondary signal).
    float aits_out_stereo[BUF_SZ * 2] {};

private:
    // Plaits Voice + its 16 KB shared allocator workspace are PSRAM-allocated
    // in Init. The Voice has 22 engine instances inline (incl. 3 Six-Op DX7
    // voicings) — too large to keep as an inline member without overflowing
    // PicoSeqRack's ctagSPAllocator block (300 KB).
    plaits::Voice *voice {nullptr};
    char *shared_buffer {nullptr};
    plaits::Patch patch {};
    plaits::Modulations modulations {};

    // Note state.
    int midi_note {60};
    bool note_held {false};
    uint8_t pending_velocity {100};
    // 2-block trigger pulse state machine. noteOn loads 2; Process emits one
    // block of trigger=0 then one block of trigger=1, guaranteeing a 0→1
    // edge for Plaits' internal trigger detector even on repeated same-note
    // sequencer triggers (where note_held stays true between calls).
    int trigger_pulse_state {0};

    // Block counter for silence gate: counts up since the last trigger /
    // active note. After kSilenceTailBlocks of pure idle we mute the output
    // and skip Voice::Render entirely (CPU + correctness — drone engines
    // like Speech/Chord/Noise never naturally fall silent without LPG).
    int silence_tail_blocks {0};

    // ---- Plaits-mapped params (CCs 8..18) ----
    // Defaults pre-set to the cv-space values that match the preset's
    // post-load state, so the very first Process block produces musical
    // output even before the macro engine pushes preset values.
    //
    // Default model = 2 (Six-Op DX7 instance A) so the user immediately
    // hears DX7-style FM out of the box — which is the headline feature.
    atomic<int16_t> model_par {320};      // preset 2 (cc src) × mul 5 → wire 10 → cv 320 → engine 2 (DX7A) via wrapper / 160
    atomic<int16_t> freq_par {2048};      // preset 8192 → cv 2048 → no detune
    atomic<int16_t> harm_par {2048};      // preset 8192 → cv 2048 → 0.5
    atomic<int16_t> timbre_par {2048};    // preset 8192 → cv 2048 → 0.5
    atomic<int16_t> morph_par {2048};     // preset 8192 → cv 2048 → 0.5
    atomic<int16_t> decay_par {2400};     // preset 9600 → cv 2400 → ~0.6 LPG decay
    atomic<int16_t> color_par {2048};     // preset 8192 → cv 2048 → 0.5 LPG colour
    atomic<int16_t> level_par {2900};     // preset 11600 → cv 2900 → ~0.7 level
    atomic<int16_t> fmod_par {0};         // preset 0 → no FM mod by default
    atomic<int16_t> tmod_par {0};         // preset 0 → no Timbre mod by default
    atomic<int16_t> mmod_par {0};         // preset 0 → no Morph mod by default
};
