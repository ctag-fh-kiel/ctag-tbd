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
#include "FavoritesModel.hpp"
#include <RtAudio.h>
#include <atomic>
#include "SimDataModel.hpp"
#include "SimStimulus.hpp"

using namespace CTAG::SP;
using namespace std;

namespace CTAG {
    namespace AUDIO {
        class SimSPManager {
        public:
            static void StartSoundProcessor(int iSoundCardID, string wavFile, string sromFile, bool bOutOnly);

            static void StopSoundProcessor();

            static void ListSoundCards();

            static const char *GetCStrJSONSoundProcessors() {
                return model->GetCStrJSONSoundProcessors();
            }

            static const char *GetCStrJSONActivePluginParams(const int chan) {
                return sp[chan]->GetCStrJSONParamSpecs();
            }

            static const char *GetCStrJSONGetPresets(const int chan) { // names of all available presets
                return sp[chan]->GetCStrJSONPresets();
            }

            static const char *GetCStrJSONAllPresetData(const int chan) { // current preset as JSON
                return sp[chan]->GetCStrJSONAllPresetData();
            }

            static const char *GetCStrJSONConfiguration() {
                return model->GetCStrJSONConfiguration();
            }

            static const char *GetCStrJSONSoundProcessorPresets(const string &id) {
                return model->GetCStrJSONSoundProcessorPresets(id);
            }

            static void SetCStrJSONSoundProcessorPreset(const char* id, const char* data) {
                model->SetCStrJSONSoundProcessorPreset(id, data);
            }

            static void SetConfigurationFromJSON(const string &data);

            static string GetStringID(const int chan);

            static bool IsStereo(const int chan);

            static void SetSoundProcessorChannel(const int chan, const string &id);

            static void SetChannelParamValue(const int chan, const string &id, const string &key, const int val);

            static void ChannelSavePreset(const int chan, const string &name, const int number);

            static void ChannelLoadPreset(const int chan, const int number);

            static void SetProcessParams(const string &params);

            static const char *GetProcessParams() {
                return simModel->GetModelJSONCString();
            }

            // favorites api
            static string GetAllFavorites();
            static void StoreFavorite(int const &id, const string &fav);
            static void ActivateFavorite(const int &id);

        private:

            static void updateConfiguration();

            static ctagSoundProcessor* sp[2];
            static std::unique_ptr<SPManagerDataModel> model;
            static std::unique_ptr<FAV::FavoritesModel> favModel;
            static RtAudio audio;

            static int inout(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                             double streamTime, RtAudioStreamStatus status, void *userData);

            static std::unique_ptr<SimDataModel> simModel;
            static SimStimulus stimulus;
        };
    }
}
