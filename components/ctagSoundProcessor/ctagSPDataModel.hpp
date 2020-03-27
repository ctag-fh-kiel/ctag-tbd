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


#pragma  once

#include <string>
#include "ctagDataModelBase.hpp"
#include <iostream>
#include <memory>
#include <vector>

using namespace std;
using namespace rapidjson;

namespace CTAG{
    namespace SP{
        class ctagSPDataModel : public ctagDataModelBase {
        public:
            ctagSPDataModel(const string &id, const bool isStereo);
            ~ctagSPDataModel();
            const char * GetCStrJSONParams();
            void LoadPreset(const int num);
            // set a preset value, "which" can be current, cv, trig
            void SetParamValue(const string &id, const string &key, const int val);
            int GetParamValue(const string &id, const string &key);
            void SavePreset(const string &name, const int number);
            const char * GetCStrJSONPresets(); // all presets
            const char * GetCStrJSONAllPresetData(); // current preset
            void PrintSelf();
        private:
            void recursiveFindAndInsert(const Value &paramF, Value &paramI);
            // merge ui and preset models
            void mergeModels();
            Document mui, mp;
            string mpFileName, muiFileName;
            Value activePreset;
        };
    }
}

