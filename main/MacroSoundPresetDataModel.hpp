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
#include <string>
#include "ctagDataModelBase.hpp"

namespace CTAG {
    namespace MACROPRESETS {
        class MacroSoundPreset;
        class MacroSoundPresetGroup;

        class SynthDefinitionDataModel;
        class MacroDeviceDefinitionDataModel;

        class MacroSoundPresetDataModel final : public CTAG::SP::ctagDataModelBase{
            private:
                std::vector<MacroSoundPreset*> presets;
                std::vector<MacroSoundPresetGroup*> groups;
            public:
                MacroSoundPresetDataModel();
                ~MacroSoundPresetDataModel();
                void ReloadSoundPresets( MacroDeviceDefinitionDataModel *macromodel,  SynthDefinitionDataModel *synthmodel);
                int GetNumberOfSoundPresetGroups();
                void GetMacroSoundPresetGroupId(int index, std::string *idOutput);
                int GetNumberOfSoundPresets();
                void GetPresetIndexJson(int trackIndex, std::string *output);
                void SerializeListJSON(std::string *output);
                void SerializeItemJSON(const std::string &id, std::string *output);
                MacroSoundPreset *LoadMacroSoundPreset(const std::string id);
                bool UpdatePreset(const std::string &jsonString);
                void DeleteItem(const std::string &id);
                bool SerializeListInto(int trackIndex, rapidjson::Document &doc);
                bool PutSamplePresetJSON(const string &presetJSON);
        };
    }
}