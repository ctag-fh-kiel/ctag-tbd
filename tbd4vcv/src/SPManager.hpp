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
#include <string>
#include "ctagSoundProcessorFactory.hpp"
#include "ctagSoundProcessor.hpp"
#include "SPManagerDataModel.hpp"
#include <atomic>

using namespace CTAG::SP;
using namespace std;

namespace CTAG {
    namespace AUDIO {
        class SPManager {
        public:
            void Start();
            void Stop();
            void Process(const CTAG::SP::ProcessData &);

            const char *GetCStrJSONSoundProcessors() {
                return model->GetCStrJSONSoundProcessors();
            }

            const char *GetCStrJSONActivePluginParams(const int chan) {
                return sp[chan]->GetCStrJSONParamSpecs();
            }

            const char *GetCStrJSONGetPresets(const int chan) { // names of all available presets
                return sp[chan]->GetCStrJSONPresets();
            }

            const char *GetCStrJSONAllPresetData(const int chan) { // current preset as JSON
                return sp[chan]->GetCStrJSONAllPresetData();
            }

            const char *GetCStrJSONConfiguration() {
                return model->GetCStrJSONConfiguration();
            }

            const char *GetCStrJSONSoundProcessorPresets(const string &id) {
                return model->GetCStrJSONSoundProcessorPresets(id);
            }

            void SetJSONSoundProcessorPreset(const string &id, const string &data) {
                model->SetJSONSoundProcessorPreset(id, data);
            }

            void SetConfigurationFromJSON(const string &data);

            string GetStringID(const int chan);

            void SetSoundProcessorChannel(const int chan, const string &id);

            void SetChannelParamValue(const int chan, const string &id, const string &key, const int val);

            void ChannelSavePreset(const int chan, const string &name, const int number);

            void ChannelLoadPreset(const int chan, const int number);

            void SetProcessParams(const string &params);
            bool GetBlueStatus();
            string GetSPManagerDataModel();
            void SetSPManagerDataModel(const string &);


        private:

            void updateConfiguration();
            bool blue {false};

            std::unique_ptr<ctagSoundProcessor> sp[2];
            std::unique_ptr<SPManagerDataModel> model;
        };
    }
}
