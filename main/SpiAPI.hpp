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

#include "sdkconfig.h"
#if CONFIG_TBD_USE_RP2350

#include <cstdint>
#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/spi_slave.h"

namespace CTAG::SPIAPI{
    class SpiAPI {
        // the api reflects requests coming from the host via SPI, rp2350 is host, p4 is slave
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
            SetActiveSampleKit = 0x18, // sets active sample rom bank/kit, args [bank index (uint8_t)]
            GetFirmwareInfo = 0x19, // returns json {"HWV": hardware version, "FWV": firmware version, "OTA": active ota partition}
            SetAbletonLinkTempo = 0x20, // sets Ableton Link tempo, args [tempo (float bpm)]
            SetAbletonLinkStartStop = 0x21, // sets Ableton Link start/stop, args [isPlaying (uint8_t, 0 = stop, 1 = start)]
            RebootToOTAX = 0x22, // reboots the device to OTAX, args [X (uint8_t)]
            SendFile = 0x23, // sends a file to the device, args [filepath (cstring), filedata (byte array)]
            GetSampleFileCount = 0x50,
            GetSampleFileInfo = 0x51,
            GetSampleFileWaveformPreview = 0x52,

            EnableFileTransferMode = 0x60,
            DisableFileTransferMode = 0x61,

            GetSynthDefinitionsJSON = 0x70,

            GetMacroMachineDefinitionsJSON = 0x80,
            UploadMacroMachineDefinitionJSON = 0x81,
            SetTrackMacroMachine = 0x82,
            GetSoundPresetListJSON = 0x90,
            GetSoundPresetJSON = 0x91,
            UploadSoundPresetJSON = 0x92,

            GetMacroSoundPresetList = 0xA0,
            GetMacroSoundPreset = 0xA1,
            GetMacroDefinition = 0xA2,
            ActivateTrackMachine = 0xA3,
            LoadTrackSoundPreset = 0xA4,
            GetTrackDefaultPresets = 0xA5, // returns JSON with default preset IDs per track from trackdefaults/default.json
            // SetTrackSampleBank = 0xA6,
            GetKitIndexJSON = 0xA7,
            GetSampleBankIndexJSON = 0xA8,
            // GetSynthUpdates = 0xA7,
            PutSamplePresetJSON = 0xA9,
            LoadTrackMacroDefinition = 0xAA,

            AnnounceApp = 0xAB, // RP2350 announces its active app, args [flags (uint8_t, bit0=plugin_lock, bit1=redirect_samples), app_name (cstring)]
            ReportPicoVersion = 0xAC, // RP2350 reports its firmware version, args [version_string (cstring)]
            GetPicoUpdateStatus = 0xAE, // RP2350 queries if Pico firmware was updated this boot, returns "updated" or "none"

            // Phase 1: Project storage on P4 SD card
            SaveProjectToP4 = 0xB0, // receive project binary from Pico, save to P4 SD, args [slotName (cstring)]
            LoadProjectFromP4 = 0xB1, // load project binary from P4 SD, send to Pico, args [slotName (cstring)]
            ListProjects = 0xB2, // list projects from user + factory dirs, returns JSON array
            DeleteProject = 0xB3, // delete project from user dir, args [slotName (cstring)]
            SavePicoConfig = 0xB4, // receive config binary from Pico, save to P4 SD
            LoadPicoConfig = 0xB5, // load config binary from P4 SD, send to Pico

            // Phase 2: Track default template management
            ListTrackDefaults = 0xB6, // scan overlay trackdefaults, return JSON list of template names
            GetTrackDefault = 0xB7, // return JSON content of a named template, args [name (cstring)]
            SaveTrackDefault = 0xB8, // save a user track default template, args [name (cstring)], then JSON data
            DeleteTrackDefault = 0xB9, // delete a user track default template, args [name (cstring)]

            SetActiveTrackDefault = 0xBB, // set active boot template, args [name (cstring)], writes user/config/active-trackdefault.txt

            // Phase 3: Project preset snapshot / parameter restore
            SetTrackParamValues = 0xBA, // batch-set track parameter values, args [trackIndex (uint8), count (uint8), values (int16[] in string_param)]
        };

        static std::string rp2350AppId;   // app name announced by RP2350 (empty = unknown/legacy)
        static std::string rp2350PicoVersion; // firmware version reported by RP2350
        static bool rp2350PluginLock;     // true = RP2350 app requested HTTP plugin switching be blocked
        static bool rp2350RedirectSamples; // true = WebUI should default to Samples view
        static SemaphoreHandle_t rp2350StateMutex; // protects rp2350AppId, rp2350PicoVersion from torn reads

        static TaskHandle_t hTask;
        static spi_slave_transaction_t transaction;
        static uint8_t *send_buffer, *receive_buffer;
        static void api_task(void *pvParameters);
        static bool transmitCString(const RequestType reqType, const char *str);
        static bool transmitBinary(const RequestType reqType, const uint8_t *data, uint32_t len);
        static bool receiveString(const RequestType reqType, std::string &str);
        static bool handle_send_file(const std::string& filepath,
                                      uint8_t* send_buffer,
                                      uint8_t* receive_buffer,
                                      spi_slave_transaction_t& transaction);
        static bool handle_send_file();

    public:
        SpiAPI() = delete;
        static void StartSpiAPI();
        static std::string GetRP2350AppId() {
            xSemaphoreTake(rp2350StateMutex, portMAX_DELAY);
            std::string copy = rp2350AppId;
            xSemaphoreGive(rp2350StateMutex);
            return copy;
        }
        static std::string GetPicoVersion() {
            xSemaphoreTake(rp2350StateMutex, portMAX_DELAY);
            std::string copy = rp2350PicoVersion;
            xSemaphoreGive(rp2350StateMutex);
            return copy;
        }
        static bool IsPluginLocked() { return rp2350PluginLock; }
        static bool ShouldRedirectSamples() { return rp2350RedirectSamples; }
    };
}

#endif // CONFIG_TBD_USE_RP2350