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

#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <map>
#include <functional>
#include <atomic>

#include "../ctagSoundProcessor.hpp"
#include "helpers/ctagSampleRom.hpp"

using namespace std;

#define BUF_SZ 32

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorPicoSeqRack;

        struct PicoSeqRackProcessData {
            uint32_t firstNonWtSlice;
            HELPERS::ctagSampleRom *sampleRom;
            uint32_t msPerBeat;
            uint32_t tempo; // BPM * 100
            uint8_t quantum;
            float *inputbuffer;
        };

        struct PickSeqRackInitData {
            int track_index;
            const char *prefix;
            int midi_channel;
            int cc_base;
            ctagSoundProcessorPicoSeqRack *rack;
            HELPERS::ctagSampleRom *sampleRom;
        };
    }
}