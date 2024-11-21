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
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include <tbd/config_base.hpp>


namespace tbd::audio {

struct SPManagerDataModel final : config::ConfigBase {

    SPManagerDataModel();

    ~SPManagerDataModel();

    const char *GetCStrJSONSoundProcessors();

    const char *GetCStrJSONConfiguration();

    const char *GetCStrJSONSoundProcessorPresets(const std::string &id);

    void SetCStrJSONSoundProcessorPreset(const char *id, const char* data);

    std::string GetActiveProcessorID(const int chan);

    void SetConfigurationFromJSON(const std::string &data);

    std::string GetConfigurationData(const std::string &id);

    void SetActivePluginID(const std::string &id, const int chan);

    int GetActivePatchNum(const int chan);

    void SetActivePatchNum(const int patchNum, const int chan);

    bool IsStereo(const std::string &id);

    void PrintSelf();

    bool HasPluginID(std::string const &id);

private:
    void getSoundProcessors();

    void validateActiveProcessors();

    void validatePatches();

    rapidjson::Document m;
#ifndef TBD_SIM
            const std::string MODELJSONFN = "/spiffs/data/spm-config.jsn";
#else
            const std::string MODELJSONFN = "../../spiffs_image/data/spm-config.jsn";
#endif
};

}

