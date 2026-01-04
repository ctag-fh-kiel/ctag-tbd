/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2025 by Robert Manzke. All rights reserved.

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
#include <cstdint>
#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_slave.h"

namespace CTAG::SPIAPI{
    class SpiAPI {
        enum class RequestType : uint8_t{
            GetPlugins = 0x01, // returns json cstring with all plugins
            GetActivePlugin = 0x02, // returns cstring with active plugin, args uint8_t for channel
            GetActivePluginParams = 0x03, // returns json cstring with all params of active plugin, args uint8_t for channel
            SetActivePlugin = 0x04, // sets active plugin, args [channel (uint8_t), pluginID (cstring)]
            SetPluginParam = 0x05, // sets a plugin parameter, args [channel (uint8_t), paramID (cstring), value (int32_t)]
            SetPluginParamCV = 0x06, // sets a plugin parameter for CV, args [channel (uint8_t), paramID (cstring), value (int8_t)]
            SetPluginParamTRIG = 0x07, // sets a plugin parameter for TRIG, args [channel (uint8_t), paramID (cstring), value (int8_t)]
            GetPresets = 0x08, // returns json cstring with all presets for a channel, args [channel (uint8_t)]
            GetPresetData = 0x09, // returns json cstring with all preset data for a plugin, args [pluginID (cstring)]
            SetPresetData = 0x0A, // sets a preset data for a plugin, args [pluginID (cstring), presetData (json cstring)]
            LoadPreset = 0x0B, // loads a preset for a channel, args [channel (uint8_t), presetID (int8_t)]
            SavePreset = 0x0C, // saves a preset for a channel, args [channel (uint8_t), presetID (int8_t), presetName (cstring)]
            GetAllFavorites = 0x0D, // returns json cstring with all favorites
            SaveFavorite = 0x0E, // saves a favorite, args [favorite number (int8_t), json cstring with favorite data]
            LoadFavorite = 0x0F, // loads a favorite, args [favorite number (int8_t)]
            GetConfiguration = 0x10, // returns json cstring with current configuration
            SetConfiguration = 0x11, // sets the configuration, args [json cstring with configuration data]
            GetIOCapabilities = 0x12, // returns json cstring with IO capabilities
            Reboot = 0x13, // reboots the device
            SetPluginParamsJSON = 0x14, // updates running plugin data, args [channel (uint8_t), presetData (json cstring)]
            RebootToOTA1 = 0x15, // reboots the device to OTA1
            GetSampleRomDescriptor = 0x16, // returns json cstring with sample rom descriptor
            SetActiveWaveTableBank = 0x17, // sets active wavetable bank, args [bank index (uint8_t)]
            SetActiveSampleRomBank = 0x18, // sets active sample rom bank, args [bank index (uint8_t)]
            GetFirmwareInfo = 0x19, // returns json {"HWV": hardware version, "FWV": firmware version, "OTA": active ota partition}
            SetAbletonLinkTempo = 0x20, // sets Ableton Link tempo, args [tempo (float bpm)]
            SetAbletonLinkStartStop = 0x21, // sets Ableton Link start/stop, args [isPlaying (uint8_t, 0 = stop, 1 = start)]
            RebootToOTAX = 0x22, // reboots the device to OTAX, args [X (uint8_t)]
            GetSampleRomBankDescriptor = 0x23, // returns json cstring with sample rom bank descriptor, args [bank index (uint8_t)]
            GetSampleRomBankPreviews = 0x24, // returns binary data with sample rom bank previews, args [bank index (uint8_t)]
        };

        static TaskHandle_t hTask;
        static spi_slave_transaction_t transaction;
        static uint8_t *send_buffer, *receive_buffer;
        static void api_task(void *pvParameters);
        static bool transmitCString(const RequestType reqType, const char *str);
        static bool receiveString(const RequestType reqType, std::string &str);
        static bool transmitBinaryFile(const RequestType reqType, const std::string &filename);
    public:
        SpiAPI() = delete;
        static void StartSpiAPI();
    };
}