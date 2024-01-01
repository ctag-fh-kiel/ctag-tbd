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

#pragma once

#include <cstdint>
#include <array>
#include "MiSuperSawOsc.hpp"
#include "braids/svf.h"
#include "helpers/ctagADSREnv.hpp"
#include "helpers/ctagSineSource.hpp"

using namespace std;

namespace CTAG {
    namespace SP {
        // from plaits chord engine augmented with inversions
        static const int kChordNumNotes = 8;
        static const int kChordNumChords = 18;
        // more are here: http://www.thecipher.com/chords_tbl-2.html
        static const int8_t chords[kChordNumChords][kChordNumNotes] = {
                {-24,     -12,      0, 12, 24, 36, 48,     60}, // OCT
                {-12 + 7, -12 + 7,  0, 7,  7,  12, 12 + 7, 12 + 7},  // 5
                {-12 + 5, -12 + 7,  0, 5,  7,  12, 12 + 5, 12 + 7},  // sus4
                {-12 + 3, -12 + 7,  0, 3,  7,  12, 12 + 3, 12 + 7},  // m
                {-12 + 3, -12 + 7,  0, 3,  7,  9,  12,     12 + 3},    // m6
                {-12 + 3, -12 + 7,  0, 3,  7,  10, 12,     12 + 3},  // m7
                {-12 + 3, -12 + 10, 0, 3,  10, 14, 12,     12 + 3},  // m9 3
                {-12 + 3, -12 + 10, 0, 3,  10, 17, 12,     12 + 10},  // m11
                {-12 + 2, -12 + 9,  0, 2,  9,  16, 12,     12 + 2},  // 69
                {-12 + 4, -12 + 11, 0, 4,  11, 14, 12,     12 + 4},  // M9
                {-12 + 4, -12 + 7,  0, 4,  7,  11, 12,     12 + 4},   // M7
                {-12 + 4, -12 + 7,  0, 4,  7,  9,  12,     12 + 4},     // M6
                {-12 + 4, -12 + 7,  0, 4,  7,  12, 12 + 4, 12 + 7},  // M
                {-12 + 4, -12 + 7,  0, 4,  7,  10, 12,     12 + 4},  // D7
                {-12 + 4, -12 + 8,  0, 4,  8,  10, 12,     12 + 4}, // A7
                {-12 + 3, -12 + 6,  0, 3,  6,  10, 12,     12 + 3}, // HDim7
                {-12 + 3, -12 + 6,  0, 3,  6,  9,  12,     12 + 3}, // Dim7
                {-12 + 5, -12 + 7,  0, 5,  7,  10, 12,     12 + 5}  // Dom7 sus
        };

        class ChordSynth {
        public:
            struct ChordParams {
                int16_t pitch;
                int16_t chord;
                int16_t nnotes;
                int16_t inversion;
                int16_t detune;
                float attack, decay, release, sustain;
                float lfo1_freq, lfo2_freq, lfo1_amt, lfo2_amt;
                float eg_filt_amt;
                uint16_t filter_freq, filter_reso, filter_type;
                bool lfo2_random_phase;
            };

            void Init(const ChordParams &params);

            void Hold();

            void NoteOff();

            void SetDetune(const uint32_t &detune);

            void SetCutoff(const uint32_t &cutoff);

            void SetResonance(const uint32_t &resonance);

            void SetFilterType(const braids::SvfMode &mode);

            void Process(float *buf, const uint32_t &ofs);

            float GetTTL();

            bool IsDead();

        private:
            int16_t buffer[32];
            int8_t scale[4];

            HELPERS::ctagADSREnv adsr;
            braids::Svf svf;
            array<MiSuperSawOsc, 4> v_osc;
            braids::SvfMode mode_ = braids::SvfMode::SVF_MODE_LP;
            CTAG::SP::HELPERS::ctagSineSource lfo1, lfo2;
            ChordParams params_;

            void calcInversion(int8_t *ht_steps, const int16_t &chord, const int16_t &inversion, const int16_t &nnotes);

        };
    }
}
