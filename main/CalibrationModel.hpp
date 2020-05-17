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

#include <string>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "SPManagerDataModel.hpp"
#include <iostream>
#include <memory>
#include <vector>

using namespace std;
using namespace rapidjson;

namespace CTAG{
    namespace CAL{
        class CalibrationModel : public CTAG::SP::ctagDataModelBase{
        public:
            CalibrationModel();
            ~CalibrationModel();
            void CreateMatrix();
            void PushRow(const vector<uint32_t> data);
            void StoreMatrix(const string &id);
            void StoreMatrix(const string &id, const vector<vector<float>> mat);
            vector<vector<uint32_t >> GetMatrix(const string &id);
            void PrintSelf();
            void LoadMatrix(const string &id, float* data);
            bool GetCalibrateOnReboot();
            void SetCalibrateOnReboot(bool val);
            const char *GetCStrJSONCalibration();
        private:
            Document m;
            const string MODELJSONFN = "/spiffs/data/calibration.jsn";
            Document matrix;
        };
    }
}