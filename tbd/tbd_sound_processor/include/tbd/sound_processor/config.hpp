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

#include <tbd/config_base.hpp>
#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <rapidjson/document.h>


namespace CTAG::SP {

struct SoundProcessorParams : tbd::config::ConfigBase {

    SoundProcessorParams(const std::string &id, const bool isStereo);

    ~SoundProcessorParams();

    const char *GetCStrJSONParams();

    void LoadPreset(const int num);

    // set a preset value, "which" can be current, cv, trig
    void SetParamValue(const std::string &id, const std::string &key, const int val);

    int GetParamValue(const std::string &id, const std::string &key);

    void SavePreset(const std::string &name, const int number);

    const char *GetCStrJSONPresets(); // all presets
    const char *GetCStrJSONAllPresetData(); // current preset

    std::string GetActivePluginParameters(); // active preset which contains non stored values
    void SetActivePluginParameters(const std::string &preset);

    bool IsParamTrig(const std::string &id);

    bool IsParamCV(const std::string &id);

    void PrintSelf();

private:
    void recursiveFindAndInsert(const rapidjson::Value &paramF, rapidjson::Value &paramI);

    // merge ui and preset models
    void mergeModels();

    rapidjson::Document mui, mp;
    std::string mpFileName, muiFileName;
    rapidjson::Document activePreset;
};

}

