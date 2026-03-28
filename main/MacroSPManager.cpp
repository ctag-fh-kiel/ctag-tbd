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
#include "MacroTranslator.hpp"
#include "MacroDeviceDefinition.hpp"
#include "MacroSoundPreset.hpp"
#include "esp_log.h"
#include "esp_heap_caps.h"

using namespace CTAG::AUDIO;
using namespace CTAG::MACROPRESETS;

// ─── Static member definitions (macro system) ────────────────────

std::shared_ptr<SynthDefinitionDataModel> SoundProcessorManager::synthDefinitionModel = nullptr;
std::shared_ptr<MacroSoundPresetDataModel> SoundProcessorManager::macroSoundDefinitionModel = nullptr;
std::shared_ptr<MacroDeviceDefinitionDataModel> SoundProcessorManager::macroDeviceDefinitionModel = nullptr;
std::shared_ptr<MacroTranslator> SoundProcessorManager::macroTranslator = nullptr;

// ─── Macro system initialization ─────────────────────────────────
// Called from StartSoundProcessor() via this helper.

void SoundProcessorManager::InitMacroSystem() {
    synthDefinitionModel = std::make_shared<SynthDefinitionDataModel>();
    macroSoundDefinitionModel = std::make_shared<MacroSoundPresetDataModel>();
    macroDeviceDefinitionModel = std::make_shared<MacroDeviceDefinitionDataModel>();
    macroTranslator = std::make_shared<MacroTranslator>();

    synthDefinitionModel->ReloadSynthDefinitions();
    macroDeviceDefinitionModel->ReloadMachineDefinitions();
    macroSoundDefinitionModel->ReloadSoundPresets(macroDeviceDefinitionModel.get(), synthDefinitionModel.get());

    macroTranslator->synthDefinitionModel = synthDefinitionModel;
    macroTranslator->macroDeviceDefinitionModel = macroDeviceDefinitionModel;
    macroTranslator->macroSoundDefinitionModel = macroSoundDefinitionModel;
}

// ─── Macro method implementations ────────────────────────────────

void SoundProcessorManager::SetTrackMacro(const int trackIndex, const string &macroDefinitionID) {
    if (macroTranslator == nullptr) {
        return;
    }

    MacroDeviceDefinition *def = macroDeviceDefinitionModel
        ->LoadMacroDeviceDefinition(macroDefinitionID);

    if (def == nullptr) {
        ESP_LOGI("SPManager", "Macro definition %s not found, cannot load macro",
            macroDefinitionID.c_str());
        return;
    }

    xSemaphoreTake(processMutex, portMAX_DELAY);
    macroTranslator->SetTrackMacroDefinition(trackIndex, def);
    xSemaphoreGive(processMutex);
    delete def;
}

void SoundProcessorManager::SetTrackParametersFromJSON(const string &parametersJSON) {
    if (macroTranslator == nullptr) {
        return;
    }

    xSemaphoreTake(processMutex, portMAX_DELAY);
    macroTranslator->SetTrackParametersFromJSON(parametersJSON);
    xSemaphoreGive(processMutex);
}

void SoundProcessorManager::SetTrackParameter(const int trackIndex, int parameterIndex, int32_t value) {
    if (macroTranslator == nullptr) {
        return;
    }

    macroTranslator->SetTrackParameter(trackIndex, parameterIndex, value);
}

void SoundProcessorManager::RefreshMacros() {
    ESP_LOGW("SPManager", ">>> RefreshMacros called");
    xSemaphoreTake(processMutex, portMAX_DELAY);
    synthDefinitionModel->ReloadSynthDefinitions();
    macroDeviceDefinitionModel->ReloadMachineDefinitions();
    macroTranslator->RefreshActiveDefinitions();
    xSemaphoreGive(processMutex);
}

void SoundProcessorManager::RefreshSingleMacro(const string &defId) {
    ESP_LOGI("SPManager", ">>> RefreshSingleMacro id=%s", defId.c_str());
    xSemaphoreTake(processMutex, portMAX_DELAY);
    macroDeviceDefinitionModel->ReloadSingleDefinition(defId);
    macroTranslator->RefreshDefinitionById(defId);
    xSemaphoreGive(processMutex);
}

void SoundProcessorManager::RefreshSoundPresets() {
    xSemaphoreTake(processMutex, portMAX_DELAY);
    macroSoundDefinitionModel->ReloadSoundPresets(macroDeviceDefinitionModel.get(), synthDefinitionModel.get());
    xSemaphoreGive(processMutex);
}

std::string SoundProcessorManager::GetMacroSoundPresetListJSON(){
    std::string output;
    macroSoundDefinitionModel->SerializeListJSON(&output);
    return output;
}

std::string SoundProcessorManager::GetMacroSoundPresetJSON(const std::string &soundPresetId){
    std::string output;
    xSemaphoreTake(processMutex, portMAX_DELAY);
    macroSoundDefinitionModel->SerializeItemJSON(soundPresetId, &output);
    xSemaphoreGive(processMutex);
    return output;
}

std::string SoundProcessorManager::GetMacroDefinitionJSON(const std::string &soundPresetId){
    std::string output;
    xSemaphoreTake(processMutex, portMAX_DELAY);
    macroDeviceDefinitionModel->SerializeItemJSON(soundPresetId, &output);
    xSemaphoreGive(processMutex);
    return output;
}

void SoundProcessorManager::ActivateTrackMachine(const int trackIndex, const std::string machineId) {
}

void SoundProcessorManager::LoadTrackMacro(const int trackIndex, const std::string macroId) {
    xSemaphoreTake(processMutex, portMAX_DELAY);
    MacroDeviceDefinition *def =
        macroDeviceDefinitionModel->LoadMacroDeviceDefinition(macroId);
    xSemaphoreGive(processMutex);
    if (def != nullptr) {
        xSemaphoreTake(processMutex, portMAX_DELAY);
        macroTranslator->SetTrackMachine(trackIndex, def->synthId, def->volumeMultiplier);
        macroTranslator->SetTrackMacroDefinition(trackIndex, def);
        xSemaphoreGive(processMutex);
        delete def;
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
    MacroSoundPreset *preset =
        macroSoundDefinitionModel->LoadMacroSoundPreset(soundPresetId);
    xSemaphoreGive(processMutex);
    if (preset == nullptr) {
        ESP_LOGI("SPManager", "Preset %s not found, loading macro without preset",
            soundPresetId.c_str());
        return;
    }

    ESP_LOGI("SPManager", "Loaded sound preset \"%s\" name \"%s\" and macro \"%s\"",
        preset->id.c_str(), preset->displayName.c_str(), preset->macroDeviceId.c_str());

    xSemaphoreTake(processMutex, portMAX_DELAY);
    MacroDeviceDefinition *def =
        macroDeviceDefinitionModel->LoadMacroDeviceDefinition(preset->macroDeviceId);
    xSemaphoreGive(processMutex);
    if (def == nullptr) {
        ESP_LOGI("SPManager", "Macro definition %s not found, cannot load macro or preset",
            preset->macroDeviceId.c_str());
        delete preset;
        return;
    }

    ESP_LOGD("SPManager", "Loaded macro def \"%s\" named \"%s\", applying to track %d", def->id.c_str(), def->name.c_str(), trackIndex);

    ESP_LOGI("SPManager", "Mem 2 freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
        heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

    // LoadTrackMacro(trackIndex, def->synthId);
    xSemaphoreTake(processMutex, portMAX_DELAY);
    macroTranslator->SetTrackMachine(trackIndex, def->synthId, def->volumeMultiplier);
    macroTranslator->SetTrackMacroDefinition(trackIndex, def);
    xSemaphoreGive(processMutex);

    int pidx = 0;
    for(const auto& param : preset->parameterValues) {
        // ESP_LOGI("SPManager", "  Setting track %d param %d to value %f",
        //     trackIndex, pidx, param);
        macroTranslator->SetTrackParameter(trackIndex, pidx, param);
        pidx ++;
    }

    delete preset;
    delete def;

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
    // ctagSampleRom::PutSamplePresetJSON(presetJSON);
    macroSoundDefinitionModel->PutSamplePresetJSON(presetJSON);
}
