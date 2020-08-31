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
#include <RtAudio.h>
#include <atomic>

using namespace CTAG::SP;
using namespace std;

namespace CTAG {
    namespace AUDIO {
        class SimSPManager {
        public:
            static void StartSoundProcessor(int iSoundCardID, bool bOutOnly);
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

            static const char * GetCStrJSONSoundProcessorPresets(const string &id){
                return model->GetCStrJSONSoundProcessorPresets(id);
            }

            static void SetJSONSoundProcessorPreset(const string &id, const string &data){
                model->SetJSONSoundProcessorPreset(id, data);
            }

            static void SetConfigurationFromJSON(const string &data);

            static string GetStringID(const int chan);

            static void SetSoundProcessorChannel(const int chan, const string &id);

            static void SetChannelParamValue(const int chan, const string &id, const string &key, const int val);

            static void ChannelSavePreset(const int chan, const string &name, const int number);

            static void ChannelLoadPreset(const int chan, const int number);


        private:

            static void updateConfiguration();

            //static TaskHandle_t audioTaskH, ledTaskH;
            static std::unique_ptr<ctagSoundProcessor> sp[2];
            static std::unique_ptr<SPManagerDataModel> model;
            static RtAudio audio;
            static int inout( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                                            double streamTime, RtAudioStreamStatus status, void *userData );
        };
    }
}
