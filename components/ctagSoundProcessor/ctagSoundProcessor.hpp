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


// Base class for sound processors + interface

#pragma once
#include <stdint.h>
#include <string>
#include <memory>
#include "ctagSPDataModel.hpp"

using namespace std;

namespace CTAG{
    namespace SP{
        struct ProcessData{
            float *buf;
            const float *cv;
            const uint8_t *trig;
        };
        class ctagSoundProcessor{
            public:
                virtual void Process(const ProcessData &) = 0; // pure virtual --> must be implemented by derived
                virtual ~ctagSoundProcessor(){};
                void SetAudioBufferSize(int sz){bufSz = sz;}
                int GetAudioBufferSize(){return bufSz;}
                void SetProcessChannel(int ch){processCh = ch;}
                const char * GetCStrJSONParamSpecs() const {return model->GetCStrJSONParams();}
                virtual const char * GetCStrID() const = 0;
                void SetParamValue(const string id, const string key, const int val){
                    setParamValueInternal(id, key, val); // as immediate as possible
                    model->SetParamValue(id, key, val);
                }
                const char * GetCStrJSONPresets(){return model->GetCStrJSONPresets();}
                bool GetIsStereo() const {return isStereo;}
                void SavePreset(const string name, const int number){model->SavePreset(name, number);}
                void LoadPreset(const int number){
                    model->LoadPreset(number); // first get the data into the model
                    loadPresetInternal();
                }
            protected:
                virtual void setParamValueInternal(const string id, const string key, const int val){};
                virtual void loadPresetInternal(){};
                bool isStereo = false;
                int bufSz = 32;
                int processCh = 0;
                std::unique_ptr<ctagSPDataModel> model = nullptr;
        };
    }
}