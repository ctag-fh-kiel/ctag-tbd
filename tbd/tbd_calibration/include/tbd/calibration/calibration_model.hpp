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


#include <vector>
#include <tbd/config_base.hpp>


namespace tbd::calibration {

class CalibrationModel final : public tbd::config::ConfigBase {
public:
    CalibrationModel();

    void CreateMatrix();

    void PushRow(const std::vector<uint32_t> data);

    void StoreMatrix(const std::string &id);
    void StoreMatrix(const std::string &id, const std::vector<std::vector<float>> mat);

    std::vector<std::vector<uint32_t>> GetMatrix(const std::string &id);

    void PrintSelf();

    void LoadMatrix(const std::string &id, float *data);

    bool GetCalibrateOnReboot();

    void SetCalibrateOnReboot(bool val);

    const char *GetCStrJSONCalibration();

    void SetJSONCalibration(const std::string &calData);

private:
    rapidjson::Document m;
    const std::string MODELJSONFN = "/spiffs/data/calibration.jsn";
    rapidjson::Document matrix;
};

}