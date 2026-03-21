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

#include <string>
#include <vector>
#include "ctagDataModelBase.hpp"

namespace CTAG {
    namespace MACROPRESETS {
        class MacroDeviceDefinition;

        class MacroDeviceDefinitionDataModel final : public CTAG::SP::ctagDataModelBase{
            private:
                std::vector<MacroDeviceDefinition*> definitions;
            public:
                MacroDeviceDefinitionDataModel();
                ~MacroDeviceDefinitionDataModel();
                void ReloadMachineDefinitions();
                bool ReloadSingleDefinition(const std::string &id);
                int GetNumberOfDefinitions();
                // void GetMacroDeviceDefinitionId(int index, char *buffer, int bufferSize);
                MacroDeviceDefinition *LoadMacroDeviceDefinition(const std::string id);
                void SerializeListJSON(std::string *output);
                void SerializeItemJSON(const std::string &id, std::string *output);
                bool UpdateDefinition(const std::string &jsonString);
                void DeleteItem(const std::string &id);
        };
    }
}