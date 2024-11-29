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
#include <tbd/sound_manager/sound_params.hpp>

namespace tbd::audio {

/** @brief central audio processing module
 *
 *  This class is the main central module of all TBD applications.
 *
 */
class SoundProcessorManager final {
public:
    SoundProcessorManager() = delete;

    /*  @brief start audio processing
     *
     *  When the in our output is live (not files) account for a small startup delay.
     */
    static void begin(SoundParams&& sound_params = {});


    static void end();

    static const char *GetCStrJSONSoundProcessors();

    static const char *GetCStrJSONActivePluginParams(int chan);

    static const char *GetCStrJSONGetPresets(int chan); // names of all available presets


    static const char *GetCStrJSONAllPresetData(int chan); // current preset as JSON

    static const char *GetCStrJSONConfiguration();

    static const char *GetCStrJSONSoundProcessorPresets(const std::string& id);

    static void SetCStrJSONSoundProcessorPreset(const char* id, const char* data);

    static void SetConfigurationFromJSON(const std::string& data);

    static std::string GetStringID(int chan);

    static void SetSoundProcessorChannel(int chan, const std::string& id);

    static void SetChannelParamValue(int chan, const std::string& id, const std::string& key, int val);

    static void ChannelSavePreset(int chan, const std::string& name, int number);

    static void ChannelLoadPreset(int chan, int number);

    static void KillAudioTask();

    static void DisablePluginProcessing();
    static void EnablePluginProcessing();
    static void RefreshSampleRom();

private:
    static void updateConfiguration();
};
    
}
