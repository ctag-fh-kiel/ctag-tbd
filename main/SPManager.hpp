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

using namespace CTAG::SP;

namespace CTAG {
    namespace AUDIO {
        class SoundProcessorManager {
        public:
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

            static void SetJSONSoundProcessorPreset(const string &id, const string &data) {
                ledBlink = 1;
                model->SetJSONSoundProcessorPreset(id, data);
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

        private:
            static void audio_task(void *pvParams);

            static void led_task(void *pvParams);

            static void updateConfiguration();

            static TaskHandle_t audioTaskH, ledTaskH;
            static std::unique_ptr<ctagSoundProcessor> sp[2];
            static std::unique_ptr<SPManagerDataModel> model;
            static SemaphoreHandle_t processMutex;
            static atomic<uint32_t> ledBlink;
            static atomic<uint32_t> ledStatus;
            static atomic<uint32_t> noiseGateCfg;
            static atomic<uint32_t> toStereoCH0;
            static atomic<uint32_t> toStereoCH1;
            static atomic<uint32_t> runAudioTask;
            static atomic<uint32_t> ch0_outputSoftClip;
            static atomic<uint32_t> ch1_outputSoftClip;
        };
    }
}
