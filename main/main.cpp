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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <tbd/drivers/codec.hpp>

#include <tbd/sound_manager.hpp>
#include <tbd/sound_processor/allocator.hpp>

#if TBD_CALIBRATION
    #include <tbd/calibration.hpp>
#endif

#if TBD_DISPLAY
    #include <tbd/display.hpp>
#endif

#if TBD_INDICATOR
    #include <tbd/drivers/indicator.hpp>
#endif

#if TBD_FILE_SYSTEM
    #include <tbd/drivers/file_system.hpp>
#endif

#ifdef TBD_API_WIFI
    #include <tbd/network.hpp>
    #include <tbd/network/config.hpp>
    #include <tbd/api/rest_api.hpp>
#elif TBD_API_SERIAL
    #include "SerialAPI.hpp"
#endif


extern "C" {
void app_main();

void app_main() {
    // reserve large block of memory before anything else happens
    CTAG::SP::ctagSPAllocator::AllocateInternalBuffer(CONFIG_SP_FIXED_MEM_ALLOC_SZ); // TBDings has highest needs of 113944 bytes, take 112k=114688 bytes as default

    // wait until power is somewhat more settled
    vTaskDelay(2000 / portTICK_PERIOD_MS);

#if TBD_FILE_SYSTEM
    // init fs
    tbd::drivers::FileSystem::InitFS();
#endif


#if TBD_INDICATOR
    tbd::drivers::Indicator::Init();
    tbd::drivers::Indicator::SetLedRGB(0, 0, 255);
#endif

#if TBD_DISPLAY
    tbd::Display::Init();
    tbd::Display::ShowFWVersion();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
#endif

#if defined(TBD_API_SERIAL)
    vTaskDelay(2000 / portTICK_PERIOD_MS);
#endif

    tbd::audio::SoundProcessorManager::begin();

    #ifdef TBD_API_WIFI
        tbd::network::NetworkConfig network_config;
        tbd::Network::SetSSID(network_config.ssid());
        tbd::Network::SetPWD(network_config.pwd());
        tbd::Network::SetIsAccessPoint(network_config.is_access_point());
        tbd::Network::SetIP(network_config.ip());
        tbd::Network::SetMDNSName(network_config.mdns_name());
        tbd::Network::Up();
        tbd::api::RestApi::begin();
    #elif TBD_API_SERIAL
#error "whoops"
        CTAG::SAPI::SerialAPI::StartSerialAPI();
    #endif
}

}
