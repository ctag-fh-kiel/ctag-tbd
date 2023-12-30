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

#define MK_BOOL_PAR(outname, inname) \
    bool outname = inname;\
    if(trig_##inname != -1) outname = data.trig[trig_##inname] == 1 ? false : true;

#define MK_FLT_PAR_ABS(outname, inname, norm, scale) \
    float outname = inname / norm * scale;\
    if(cv_##inname != -1) outname = fabsf(data.cv[cv_##inname]) * scale;

#define MK_FLT_PAR_ABS_ADD(outname, inname, norm, scale) \
    float outname = inname / norm * scale;\
    if(cv_##inname != -1) outname += fabsf(data.cv[cv_##inname]) * scale;    

#define MK_FLT_PAR_ABS_SFT(outname, inname, norm, scale) \
    float outname = inname / norm * scale;\
    if(cv_##inname != -1) outname = (fabsf(data.cv[cv_##inname]) - 0.5f) * 2.f * scale;

#define MK_FLT_PAR(outname, inname, norm, scale) \
    float outname = inname / norm * scale;\
    if(cv_##inname != -1) outname = data.cv[cv_##inname] * scale;

#define MK_INT_PAR_ABS(outname, inname, scale) \
    int outname = inname;\
    if(cv_##inname != -1) outname = static_cast<int>(fabsf(data.cv[cv_##inname]) * scale);

#define MK_INT_PAR(outname, inname, scale) \
    int outname = inname;\
    if(cv_##inname != -1) outname = static_cast<int>(data.cv[cv_##inname] * scale);
    
 #define MK_FLT_PAR_ABS_MIN_MAX(outname, inname, norm, out_min, out_max) \
    float outname = inname/norm * (out_max-out_min)+out_min; \
    if(cv_##inname != -1) outname = fabsf(data.cv[cv_##inname]) * (out_max-out_min)+out_min;   

#define MK_FLT_PAR_ABS_PAN(outname, inname, norm, scale)  \
    float outname = (inname/norm+1.f)/2.f * scale; \
    if(cv_##inname != -1) outname = fabsf(data.cv[cv_##inname]) * scale; 


#include <stdint.h>
#include <string>
#include <memory>
#include <map>
#include <functional>
#include "ctagSPDataModel.hpp"

using namespace std;

namespace CTAG {
    namespace SP {
        struct ProcessData {
            float *buf;
            float *cv;
            uint8_t *trig;
        };

        class ctagSoundProcessor {
        public:
            virtual void Process(const ProcessData &) = 0; // pure virtual --> must be implemented by derived

            virtual void Init() = 0;

            virtual ~ctagSoundProcessor() {};

            int GetAudioBufferSize() { return bufSz; }

            void SetProcessChannel(int ch) { processCh = ch; }

            const char *GetCStrJSONParamSpecs() const { return model->GetCStrJSONParams(); }

            virtual const char *GetCStrID() { return id.c_str(); }
            virtual const string& GetID() { return id; }

            void SetParamValue(const string &id, const string &key, const int val) {
                setParamValueInternal(id, key, val); // as immediate as possible
                model->SetParamValue(id, key, val);
            }

            const char *GetCStrJSONPresets() { return model->GetCStrJSONPresets(); }

            const char *GetCStrJSONAllPresetData() { return model->GetCStrJSONAllPresetData(); }

            bool GetIsStereo() const { return isStereo; }

            void SavePreset(const string &name, const int number) { model->SavePreset(name, number); }

            void LoadPreset(const int number) {
                model->LoadPreset(number); // first get the data into the model
                loadPresetInternal();
            }

            std::string GetActivePluginParameters() { return model->GetActivePluginParameters(); }
            void SetActivePluginParameters(std::string const& p) {
                model->SetActivePluginParameters(p);
                loadPresetInternal();
            }

        protected:

            virtual void knowYourself() = 0;

            virtual void setParamValueInternal(const string &id, const string &key, const int val) {
                //printf("%s, %s, %d\n", id.c_str(), key.c_str(), val);
                if (key.compare("current") == 0) {
                    auto it = pMapPar.find(id);
                    if (it != pMapPar.end()) {
                        (it->second)(val);
                    }
                    return;
                }
                if (key.compare("cv") == 0) {
                    if (val >= -1 && val < N_CVS) {
                        auto it = pMapCv.find(id);
                        if (it != pMapCv.end()) {
                            (it->second)(val);
                        }
                    }
                    return;
                }
                if (key.compare("trig") == 0) {
                    if (val >= -1 && val < N_TRIGS) {
                        auto it = pMapTrig.find(id);
                        if (it != pMapTrig.end()) {
                            (it->second)(val);
                        }
                    }
                    return;
                }
            };

            virtual void loadPresetInternal() {
                // iterate all parameters, take names from parameter map (first element)
                for (const auto &kv: pMapPar) {
                    setParamValueInternal(kv.first, "current", model->GetParamValue(kv.first, "current"));
                    // check if cv and trig are set in preset, if so set in processor param
                    if (model->IsParamCV(kv.first)) {
                        //ESP_LOGW("MOdel", "IntParam %s, %d", name.c_str(), model->GetParamValue(name, "cv"));
                        setParamValueInternal(kv.first, "cv", model->GetParamValue(kv.first, "cv"));
                    } else if (model->IsParamTrig(kv.first)) {
                        //ESP_LOGW("MOdel", "BoolParam %s, %d", name.c_str(), model->GetParamValue(name, "trig"));
                        setParamValueInternal(kv.first, "trig", model->GetParamValue(kv.first, "trig"));
                    }
                }
            };

            bool isStereo = false;
            int const bufSz = 32;
            int processCh = 0;
            int instance {0};
            std::unique_ptr<ctagSPDataModel> model = nullptr;
            string id = "";
            map<string, function<void(const int)>> pMapPar;
            map<string, function<void(const int)>> pMapCv;
            map<string, function<void(const int)>> pMapTrig;
        };
    }
}