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

#include <vector>
#include <set>
#include <string>

namespace CTAG {
    namespace MACROPRESETS {
        class MacroSoundPresetGroup {
            public:
                std::string id;
                std::string displayName;
                std::set<uint8_t> validTracks;
                std::vector<std::string> fileIds;
                MacroSoundPresetGroup();
                ~MacroSoundPresetGroup();
        };
    }
}