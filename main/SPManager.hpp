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


#pragma once

#include <atomic>
#include <memory>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "ctagSoundProcessor.hpp"
#include "ctagSoundProcessorFactory.hpp"
#include "SPManagerDataModel.hpp"
#include <atomic>
#if CONFIG_TBD_USE_SD_CARD
// #include "SynthDefinitionDataModel.hpp"
// #include "MacroSoundPresetDataModel.hpp"
// #include "MacroDeviceDefinitionDataModel.hpp"
// #include "MacroTranslator.hpp"
#endif

using namespace CTAG::SP;

namespace CTAG {
    namespace AUDIO {
        class SoundProcessorManager final {
        public:
            SoundProcessorManager() = delete;
            static void StartSoundProcessor();

            static const char *GetCStrJSONSoundProcessors() {
                ledBlink = 1;
                return model->GetCStrJSONSoundProcessors();
            }

            static const char *GetCStrJSONActivePluginParams(const int chan) {
                ledBlink = 1;
                return sp[chan]->GetCStrJSONParamSpecs();
            }

            static const char *GetCStrJSONGetPresets(const int chan) { // names of all available presets
                ledBlink = 1;
                return sp[chan]->GetCStrJSONPresets();
            }

            static const char *GetCStrJSONAllPresetData(const int chan) { // current preset as JSON
                ledBlink = 1;
                return sp[chan]->GetCStrJSONAllPresetData();
            }

            static const char *GetCStrJSONConfiguration() {
                ledBlink = 1;
                return model->GetCStrJSONConfiguration();
            }

            static const char *GetCStrJSONSoundProcessorPresets(const string &id) {
                ledBlink = 1;
                return model->GetCStrJSONSoundProcessorPresets(id);
            }

            // ── Thread-safe variants ──────────────────────────────
            // These take the processMutex, copy the JSON string to a
            // SPIRAM buffer, and return the copy. Caller MUST free()
            // the returned pointer after use.
            // This prevents the audio task from invalidating the
            // internal StringBuffer while the HTTP handler sends.
            static char *GetSafeJSONActivePluginParams(const int chan);
            static char *GetSafeJSONGetPresets(const int chan);
            static char *GetSafeJSONAllPresetData(const int chan);
            static char *GetSafeJSONConfiguration();
            static char *GetSafeJSONSoundProcessors();
            static char *GetSafeJSONSoundProcessorPresets(const string &id);

            static void SetCStrJSONSoundProcessorPreset(const char* id, const char *data) {
                ledBlink = 1;
                model->SetCStrJSONSoundProcessorPreset(id, data);
            }

            static void SetChannelParamsCstrJSON(const int chan, const char *data) {
                ledBlink = 1;
                return sp[chan]->SetChannelParamsFromCStrJSON(data);
            }

            static void SetConfigurationFromJSON(const string &data);

            static string GetStringID(const int chan);

            static void SetSoundProcessorChannel(const int chan, const string &id);

            static void SetChannelParamValue(const int chan, const string &id, const string &key, const int val);

            static void ChannelSavePreset(const int chan, const string &name, const int number);

            static void ChannelLoadPreset(const int chan, const int number);

            static void KillAudioTask();

            static void DisablePluginProcessing();
            static void EnablePluginProcessing();
            static void RefreshSampleRom();

            static void SetTrackMachine(const int trackIndex, const string &synthID, float volumeMultiplier);
#if CONFIG_TBD_USE_SD_CARD
            static void SetTrackMacro(const int trackIndex, const string &macroDefinitionID);
            static void SetTrackParametersFromJSON(const string &parametersJSON);
            static void SetTrackParameter(const int trackIndex, int parameterIndex, int32_t value);
            static void PutSamplePresetJSON(const string &presetJSON);

            static std::string GetMacroSoundPresetListJSON();
            static std::string GetMacroSoundPresetJSON(const std::string &soundPresetId);
            static std::string GetMacroDefinitionJSON(const std::string &soundPresetId);
            static void ActivateTrackMachine(const int trackIndex, const std::string machineId);
            static void LoadTrackMacro(const int trackIndex, const std::string macroId);
            static void LoadTrackMacroAndPreset(const int trackIndex, const std::string soundPresetId);

            static void MarkTracksChangedFromWebui();
            static void MarkMacrosChangedFromWebui();
            static void MarkDefinitionsChangedFromWebui();

            static void RefreshMacros();
            static void RefreshSingleMacro(const string &defId);
            static void RefreshSoundPresets();
            static void InitMacroSystem();
#endif // CONFIG_TBD_USE_SD_CARD

            // Audio health monitoring — returns JSON with lock errors, slow process count, memory stats
            static string GetAudioHealthJSON();
            static void ResetAudioHealthCounters();

            static std::string GetKitIndexJSON();
            static std::string GetActiveKitBankIndexJSON();

        private:
            static void audio_task(void *pvParams);

            static void led_task(void *pvParams);

            static void updateConfiguration();

            static TaskHandle_t audioTaskH, ledTaskH;
            static ctagSoundProcessor *sp[2];
            static std::unique_ptr<SPManagerDataModel> model;
            static SemaphoreHandle_t processMutex;
            static atomic<uint32_t> ledBlink;
            static atomic<uint32_t> ledStatus;
            static atomic<uint32_t> ch01Daisy;
            static atomic<uint32_t> toStereoCH0;
            static atomic<uint32_t> toStereoCH1;
            static atomic<uint32_t> runAudioTask;
            static atomic<uint32_t> ch0_outputSoftClip;
            static atomic<uint32_t> ch1_outputSoftClip;
            static volatile uint32_t audioLockErrors;
            static volatile uint32_t slowProcessCounter;
            static volatile uint32_t sentSynthMidiBytes;
            static volatile uint32_t receivedUsbDeviceMidiBytes;
            static volatile uint32_t requestCounterErrors;

            static atomic<uint32_t> parameterChangeCounter;
            static atomic<uint32_t> macroChangeCounter;
            static atomic<uint32_t> trackMachineChangeCounter;
            static atomic<uint32_t> definitionChangeCounter;

        public:
            static atomic<uint32_t> webuiChangeCounter;
            static atomic<uint32_t> screenshotRequestCounter;
            static atomic<uint8_t> injectedButton;
            static atomic<uint8_t> injectedButtonEvent;
        };
    }
}
