/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020-2026 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

// Macro system implementation for SoundProcessorManager.
// This file is only compiled when CONFIG_TBD_USE_SD_CARD is enabled
// (excluded by CMake via the "Macro.*" filter when SD card is disabled).

#include "SPManager.hpp"
#include "EngineDefinitionDataModel.hpp"
#include "MacroTranslator.hpp"
#include "MacroDeviceDefinition.hpp"
#include "MacroSoundPreset.hpp"
#include "esp_log.h"
#include "esp_heap_caps.h"

using namespace CTAG::AUDIO;
using namespace CTAG::MACROPRESETS;

// ─── Macro system initialization ─────────────────────────────────
// Called from StartSoundProcessor() via this helper.

void SoundProcessorManager::InitMacroSystem() {
    ESP_LOGI("InitMacroSystem", "Before InitMacroSystem: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
        heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

    // initialize tracks
    EngineDefinitionDataModel::instance()->Init();
    MacroDeviceDefinitionDataModel::instance().Init();
    MacroSoundPresetDataModel::instance().Init();
    MacroTranslator::instance().Init();

    MacroDeviceDefinitionDataModel::instance().ReloadMachineDefinitions();
    MacroSoundPresetDataModel::instance().ReloadSoundPresets();

    ESP_LOGI("InitMacroSystem", "After InitMacroSystem reloads: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
        heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
}

// ─── Macro method implementations ────────────────────────────────

void SoundProcessorManager::SetTrackMacro(const int trackIndex, const string &macroDefinitionID) {
    MacroDeviceDefinition *def = MacroDeviceDefinitionDataModel::instance()
        .GetMacroDeviceDefinition(macroDefinitionID.c_str());

    if (def == nullptr) {
        ESP_LOGI("SPManager", "Macro definition %s not found, cannot load macro",
            macroDefinitionID.c_str());
        return;
    }

    xSemaphoreTake(processMutex, portMAX_DELAY);
    MacroTranslator::instance().SetTrackMacroDefinition(trackIndex, def);
    xSemaphoreGive(processMutex);
    delete def;
}

void SoundProcessorManager::SetTrackParametersFromJSON(const string &parametersJSON) {
    xSemaphoreTake(processMutex, portMAX_DELAY);
    MacroTranslator::instance().SetTrackParametersFromJSON(parametersJSON);
    xSemaphoreGive(processMutex);
}

void SoundProcessorManager::SetTrackParameter(const int trackIndex, int parameterIndex, int32_t value) {
    MacroTranslator::instance().SetTrackParameter(trackIndex, parameterIndex, value);
}

void SoundProcessorManager::RefreshMacros() {
    ESP_LOGW("SPManager", ">>> RefreshMacros called");
    xSemaphoreTake(processMutex, portMAX_DELAY);
    // SynthDefinitionDataModel::instance()->ReloadSynthDefinitions();
    MacroDeviceDefinitionDataModel::instance().ReloadMachineDefinitions();
    MacroTranslator::instance().RefreshActiveDefinitions();
    xSemaphoreGive(processMutex);
}

void SoundProcessorManager::RefreshSingleMacro(const string &defId) {
    ESP_LOGI("SPManager", ">>> RefreshSingleMacro id=%s", defId.c_str());
    xSemaphoreTake(processMutex, portMAX_DELAY);
    MacroDeviceDefinitionDataModel::instance().ReloadSingleDefinition(defId);
    MacroTranslator::instance().RefreshDefinitionById(defId);
    xSemaphoreGive(processMutex);
}

void SoundProcessorManager::RefreshSoundPresets() {
    xSemaphoreTake(processMutex, portMAX_DELAY);
    MacroSoundPresetDataModel::instance().ReloadSoundPresets();
    xSemaphoreGive(processMutex);
}

std::string SoundProcessorManager::GetMacroSoundPresetListJSON() {
    std::string output;
    MacroSoundPresetDataModel::instance().SerializeListJSON(&output);
    return output;
}

std::string SoundProcessorManager::GetMacroSoundPresetJSON(const std::string &soundPresetId){
    std::string output;
    xSemaphoreTake(processMutex, portMAX_DELAY);
    MacroSoundPresetDataModel::instance().SerializeItemJSON(soundPresetId, &output);
    xSemaphoreGive(processMutex);
    return output;
}

std::string SoundProcessorManager::GetMacroDefinitionJSON(const std::string &soundPresetId){
    std::string output;
    xSemaphoreTake(processMutex, portMAX_DELAY);
    MacroDeviceDefinitionDataModel::instance().SerializeItemJSON(soundPresetId, &output);
    xSemaphoreGive(processMutex);
    return output;
}

void SoundProcessorManager::ActivateTrackMachine(const int trackIndex, const std::string machineId) {
}

void SoundProcessorManager::LoadTrackMacro(const int trackIndex, const std::string macroId) {
    xSemaphoreTake(processMutex, portMAX_DELAY);
    MacroDeviceDefinition *def =
        MacroDeviceDefinitionDataModel::instance().GetMacroDeviceDefinition(macroId.c_str());
    xSemaphoreGive(processMutex);
    if (def != nullptr) {
        xSemaphoreTake(processMutex, portMAX_DELAY);
        MacroTranslator::instance().SetTrackMachine(trackIndex, def->synthId, def->volumeMultiplier);
        MacroTranslator::instance().SetTrackMacroDefinition(trackIndex, def);
        xSemaphoreGive(processMutex);
        // delete def;
    }
}

void SoundProcessorManager::LoadTrackMacroAndPreset(const int trackIndex, const std::string soundPresetId) {
    ESP_LOGI("SPManager", "Loading sound preset \"%s\" for track %d", soundPresetId.c_str(), trackIndex);

    ESP_LOGI("SPManager", "Mem 1 freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
        heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

    xSemaphoreTake(processMutex, portMAX_DELAY);
    MacroSoundPreset preset;
    MacroSoundPresetDataModel::instance().LoadMacroSoundPreset(&preset, soundPresetId);
    xSemaphoreGive(processMutex);
    if (preset.id[0] == '\0') {
        ESP_LOGI("SPManager", "Preset %s not found, loading macro without preset",
            soundPresetId.c_str());
        return;
    }

    ESP_LOGI("SPManager", "Loaded sound preset \"%s\" name \"%s\" and macro \"%s\"",
        preset.id, preset.displayName, preset.macroDeviceId);

    xSemaphoreTake(processMutex, portMAX_DELAY);
    MacroDeviceDefinition *def =
        MacroDeviceDefinitionDataModel::instance().GetMacroDeviceDefinition(preset.macroDeviceId);
    xSemaphoreGive(processMutex);
    if (def == nullptr) {
        ESP_LOGI("SPManager", "Macro definition %s not found, cannot load macro or preset",
            preset.macroDeviceId);
        return;
    }

    ESP_LOGD("SPManager", "Loaded macro def \"%s\" named \"%s\", applying to track %d", def->id, def->name, trackIndex);

    ESP_LOGI("SPManager", "Mem 2 freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
        heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

    // LoadTrackMacro(trackIndex, def->synthId);
    xSemaphoreTake(processMutex, portMAX_DELAY);
    MacroTranslator::instance().SetTrackMachine(trackIndex, def->synthId, def->volumeMultiplier);
    MacroTranslator::instance().SetTrackMacroDefinition(trackIndex, def);
    xSemaphoreGive(processMutex);

    // preset.parameterValues is int32_t[MaxMacroSoundPresetParameters=24]; iterate all of
    // them. trackParameterValues is now [16][32] so idx 0..31 have real storage — no wrap
    // into the adjacent track. Performance macros like td3-acidbass with source knobs at
    // idx 18..21 get their preset values written to the correct slots.
    int pidx = 0;
    for(const auto& param : preset.parameterValues) {
        MacroTranslator::instance().SetTrackParameter(trackIndex, pidx, param);
        pidx ++;
    }

    ESP_LOGD("SPManager", "Mem 4 freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
        heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

}

void SoundProcessorManager::MarkTracksChangedFromWebui(){
    trackMachineChangeCounter ++;
}

void SoundProcessorManager::MarkMacrosChangedFromWebui(){
    macroChangeCounter ++;
}

void SoundProcessorManager::MarkDefinitionsChangedFromWebui(){
    definitionChangeCounter ++;
}

void SoundProcessorManager::PutSamplePresetJSON(const string &presetJSON) {
    MacroSoundPresetDataModel::instance().PutSamplePresetJSON(presetJSON);
}
