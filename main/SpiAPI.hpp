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

namespace CTAG::SPIAPI {
    class SpiAPI {
        // the api reflects requests coming from the host via SPI, rp2350 is host, p4 is slave
        enum class RequestType : uint8_t {
            //
            // API's that shoudn't be used and should be removed (old ctag apis)
            //

            // returns json cstring with all plugins
            // TODO: Remove, not used by groovebox
            // GetPlugins = 0x01,

            // returns cstring with active plugin, args uint8_t for channel
            // TODO: Remove, not used by groovebox
            // GetActivePlugin = 0x02,

            // returns json cstring with all params of active plugin, args uint8_t for channel
            // TODO: Remove, not used by groovebox
            // GetActivePluginParams = 0x03,

            // sets active plugin, args [channel (uint8_t), pluginID (cstring)]
            // TODO: Remove, not used by groovebox
            // SetActivePlugin = 0x04,

            // sets a plugin parameter, args [channel (uint8_t), paramID (cstring), value (int32_t)]
            // TODO: Remove, not used by groovebox
            // SetPluginParam = 0x05,

            // sets a plugin parameter for CV, args [channel (uint8_t), paramID (cstring), value (int8_t)]
            // TODO: Remove, not used by groovebox
            // SetPluginParamCV = 0x06,

            // sets a plugin parameter for TRIG, args [channel (uint8_t), paramID (cstring), value (int8_t)]
            // TODO: Remove, not used by groovebox
            // SetPluginParamTRIG = 0x07,

            // returns json cstring with all presets for a channel, args [channel (uint8_t)]
            // TODO: Remove, not used by groovebox
            // GetPresets = 0x08,

            // returns json cstring with all preset data for a plugin, args [pluginID (cstring)]
            // TODO: Remove, not used by groovebox
            // GetPresetData = 0x09,

            // sets a preset data for a plugin, args [pluginID (cstring), presetData (json cstring)]
            // TODO: Remove, not used by groovebox
            // SetPresetData = 0x0A,

            // loads a preset for a channel, args [channel (uint8_t), presetID (int8_t)]
            // TODO: Remove, not used by groovebox
            // LoadPreset = 0x0B,

            // saves a preset for a channel, args [channel (uint8_t), presetID (int8_t), presetName (cstring)]
            // TODO: Remove, not used by groovebox
            // SavePreset = 0x0C,

            // returns json cstring with all favorites
            // TODO: Remove, not used by groovebox
            // GetAllFavorites = 0x0D,

            // saves a favorite, args [favorite number (int8_t), json cstring with favorite data]
            // TODO: Remove, not used by groovebox
            // SaveFavorite = 0x0E,

            // loads a favorite, args [favorite number (int8_t)]
            // TODO: Remove, not used by groovebox
            // LoadFavorite = 0x0F,

            // returns json cstring with current configuration
            // TODO: Remove, not used by groovebox
            GetConfiguration = 0x10,

            // sets the configuration, args [json cstring with configuration data]
            // TODO: Remove, not used by groovebox
            SetConfiguration = 0x11,

            // returns json cstring with IO capabilities
            // TODO: Remove, not used by groovebox (similar to firmware info)
            // GetIOCapabilities = 0x12,

            // updates running plugin data, args [channel (uint8_t), presetData (json cstring)]
            // TODO: Remove, not used by groovebox
            // SetPluginParamsJSON = 0x14,

            //
            // Legacy apis that could be removed
            //

            // batch-set track parameter values, args [trackIndex (uint8), count (uint8), values (int16[] in string_param)]
            // TODO: Not used anymore, remove.
            SetTrackParamValues = 0xBA,

            // TODO: Remove - Just set track volume ot zero instead.
            SetTrackMute = 0xBD,

            // returns json cstring with sample rom descriptor
            // TODO: Replace with GetKitIndexPage and GetSampleBankIndex, remove this.
            GetSampleRomDescriptor = 0x16,

            //
            // Legacy apis
            //

            // returns json {"HWV": hardware version, "FWV": firmware version, "OTA": active ota partition}
            // TODO: return something other thabn json!
            // TODO: this should be shared among all firmwares, maybe move to earlier index
            GetFirmwareInfo = 0x19,

            // Enable file transfer mode, currently does nothing, should probably enable file transfer mode in p4
            EnableFileTransferMode = 0x60,

            // Disable file transfer mode, currently does nothing, but should also reload list of macro definitions, macro presets, kits and sample names
            DisableFileTransferMode = 0x61,

            // TODO: Is this needed if loadtrackmacrodef and loadtrakcksoundpreset exist?
            SetTrackMacroMachine = 0x82,

            // Get list of available presets
            // TODO: return something other than json!
            // TODO: Replace with GetMacroSoundPresetsPage, remove this.
            GetSoundPresetListJSON = 0x90,

            // Get one preset by name
            // TODO: return something other thabn json!
            // Use GetMacroSoundPresetsPage isntead
            GetSoundPresetJSON = 0x91,

            // TODO: Use something other than json!
            // Use UploadMacroSoundPreset instead
            UploadSoundPresetJSON = 0x92,

            // TODO: Is this needed? the track defaults should point to all presets, and the presets activates the machines. remove.
            ActivateTrackMachine = 0xA3,

            // Activate macro preset (and machine) on track
            LoadTrackSoundPreset = 0xA4,

            // returns JSON with default preset IDs per track from trackdefaults/default.json
            // TODO: return something other thabn json
            GetTrackDefaultPresets = 0xA5,

            // Apply macro definition to track
            // TODO: Is this needed if LoadTrackSoundPreset exists?
            LoadTrackMacroDefinition = 0xAA,

            // RP2350 announces its active app, args [flags (uint8_t, bit0=plugin_lock, bit1=redirect_samples), app_name (cstring)]
            AnnounceApp = 0xAB,

            // RP2350 reports its firmware version, args [version_string (cstring)]
            ReportPicoVersion = 0xAC,

            // RP2350 queries if Pico firmware was updated this boot, returns "updated" or "none"
            GetPicoUpdateStatus = 0xAE,

            // set active boot template, args [name (cstring)], writes user/config/active-trackdefault.txt
            SetActiveTrackDefault = 0xBB,

            //
            // Legacy apis to keep or upgrade
            //

            // reboots the device, still needed.
            Reboot = 0x13,

            // reboots the device to OTA1, remove, use RebootToOTAX instead.
            RebootToOTA1 = 0x15,

            // reboots the device to OTAX, args [X (uint8_t)]
            RebootToOTAX = 0x22,

            // sets Ableton Link tempo, args [tempo (float bpm)]
            // TODO: Combine into a single ableton link command with more options.
            SetAbletonLinkTempo = 0x20,
            // sets Ableton Link start/stop, args [isPlaying (uint8_t, 0 = stop, 1 = start)]
            // TODO: Combine
            SetAbletonLinkStartStop = 0x21,

            // sets active wavetable bank, args [bank index (uint8_t)]
            // TODO: Combine with SetActiveSampleKit and use ApplyKitRequest
            SetActiveWaveTableBank = 0x17,
            // sets active sample rom bank/kit, args [bank index (uint8_t)]
            SetActiveSampleKit = 0x18,

            // TODO: Use other api than json
            // Replaced by GetMacroSoundPresetPage
            GetMacroSoundPresetList = 0xA0,
            // TODO: Use other api than json
            // TODO: Not needed, should already be cached from GetMacroSoundPresetPage
            GetMacroSoundPreset = 0xA1,

            // TODO: Use other api than json
            // TODO: Should not be needed, sound presets hide these.
            GetMacroDefinition = 0xA2,

            // TODO: Use other api than json
            // TODO: Should not be needed
            GetEngineDefinitionList = 0x70,

            // TODO: Use other api than json
            // Replaced by GetMachineDefinitionsPage
            // GetEngineDefinitionJSON = 0x71,

            // TODO: Use other api than json
            // Replaced by GetMacroDefinitionsPage
            GetMacroMachineDefinitionsJSON = 0x80,

            // TODO: Use other api than json
            // TODO: Remove.
            UploadMacroMachineDefinitionJSON = 0x81,

            // TODO: Use other api than json
            // Replaced by GetKitIndexPage
            GetKitIndexJSON = 0xA7,

            // TODO: Use other api than json
            // Replaced by GetSampleBankIndex
            GetSampleBankIndexJSON = 0xA8,

            // TODO: Use other api than json
            // Replaced by UploadMacroSoundPreset
            PutSamplePresetJSON = 0xA9,


            //
            // Legacy apis that could be replaced by simpler file apis
            //

            // receive project binary from Pico, save to P4 SD, args [slotName (cstring)]
            // TODO: Maybe replace with file write call?
            SaveProjectToP4 = 0xB0,

            // load project binary from P4 SD, send to Pico, args [slotName (cstring)]
            // TODO: Maybe replace with file read call?
            LoadProjectFromP4 = 0xB1,

            // list projects from user + factory dirs, returns JSON array
            // TODO: Maybe replace with file list call?
            ListProjects = 0xB2,

            // delete project from user dir, args [slotName (cstring)]
            // TODO: Maybe replace with file delete call?
            DeleteProject = 0xB3,

            // receive config binary from Pico, save to P4 SD
            // TODO: Replace with file write call?
            SavePicoConfig = 0xB4,

            // load config binary from P4 SD, send to Pico
            // TODO: Replace with file query+read call?
            LoadPicoConfig = 0xB5,

            // sends a file to the device, args [filepath (cstring), filedata (byte array)]
            // TODO: Replace with simpler file write call
            SendFile = 0x23,

            // TODO: Replace with simple file list call?
            GetSampleFileCount = 0x50,

            // TODO: Combine these two
            GetSampleFileInfo = 0x51,
            GetSampleFileWaveformPreview = 0x52,

            // scan overlay trackdefaults, return JSON list of template names
            // TODO: Maybe replace with a file read call?
            ListTrackDefaults = 0xB6,

            // return JSON content of a named template, args [name (cstring)]
            // TODO: Maybe replace with a file read call?
            GetTrackDefault = 0xB7,

            // save a user track default template, args [name (cstring)], then JSON data
            // TODO: Maybe replace with a file write call?
            SaveTrackDefault = 0xB8,

            // delete a user track default template, args [name (cstring)]
            // TODO: Maybe replace with a file delete call?
            DeleteTrackDefault = 0xB9,

            //
            // Legacy api's to keep as is
            //

            // receive screenshot BMP from Pico, save to user/screenshots/
            SaveScreenshot = 0xBC,
            // send screenshot but do not save it.
            SendScreenshot = 0xBD,

            //
            // New unnecessary apis
            //

            //
            // New apis replacing older ones
            //

            //
            // New apis
            //

            // GetTrackDefinitionsPage = 0xD0,
            GetEngineDefinitionsPage = 0xD1,
            GetMacroDefinitionsPage = 0xD2,
            GetMacroSoundPresetsPage = 0xD3,
            // GetMacroPreset = 0xD3,
            GetKitIndexPage = 0xD4,
            GetSampleBankIndex = 0xD5,
            GetSampleInfoAndPreview = 0xD6,
            UploadMacroSoundPreset = 0xD7,

            ReloadMacroDefinitions = 0xD8,
            ReloadMacroSoundPresets = 0xD9,
            ReloadKitIndes = 0xDA,

            FileQuery = 0xDB,
            FileReadlock = 0xDC,
            FileWriteBlock = 0xDD,
            FileDelete = 0xDE,
            FileListPage = 0xDF,


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