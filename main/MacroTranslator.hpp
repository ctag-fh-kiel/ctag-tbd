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
#include <vector>
#include <string>
#include <ctagSoundProcessor.hpp>
#include "SynthDefinitionDataModel.hpp"
#include "MacroSoundPresetDataModel.hpp"
#include "MacroDeviceDefinition.hpp"
#include "MacroDeviceDefinitionDataModel.hpp"

// #define MAX_TRACKS 24

namespace CTAG {
    namespace MACROPRESETS {
        class MacroTranslator {
            private:
                int8_t trackToMidiChannel[16];
                uint8_t trackBaseCC[16];
                uint16_t trackParameterValues[16][32];
                uint16_t outputValues[16][32];
                bool trackDirty[16];
                bool bankDirty;
                std::string trackMachineId[16];
                std::string trackSampleBankName[16];
                uint16_t trackSampleBankIndex[16];
                MacroDeviceDefinition *definition[16];

                void parseIncomingMidiMessages(const uint8_t *buf, const size_t len);

            public:
                MacroTranslator();
                ~MacroTranslator();

                std::shared_ptr<SynthDefinitionDataModel> synthDefinitionModel;
                std::shared_ptr<MacroSoundPresetDataModel> macroSoundDefinitionModel;
                std::shared_ptr<MacroDeviceDefinitionDataModel> macroDeviceDefinitionModel;
                CTAG::SP::ctagSoundProcessor *soundProcessor;

                // void SetTrackSampleBank(const int trackIndex, const std::string bankName);
                void SetTrackMachine(const int trackIndex, const std::string synthID);
                void SetTrackMacroDefinition(const int trackIndex, MacroDeviceDefinition *def);
                void SetTrackParameter(const int trackIndex, int parameterIndex, int32_t value);
                void SetTrackParametersFromJSON(const std::string &parametersJSON);

                void TranslateInput(CTAG::SP::ProcessData *pd);

                void SerializeStateJSON(std::string *output);
                bool SerializeStateInto(rapidjson::Document &doc);
        };
    }
}