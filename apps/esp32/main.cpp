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

#include <tbd/favorites.hpp>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <tbd/drivers/file_system.hpp>
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

#if TBD_NETWORK
#include <tbd/network.hpp>
#include <tbd/network/config.hpp>
#endif

#if TBD_API_REST
#include <tbd/api/rest_api.hpp>
#endif

#if TBD_API_SERIAL
#include <tbd/api/serial_api.hpp>
#endif

#if TBD_API_SHELL
#include <tbd/api/common/shell.hpp>
#endif


extern "C" {

void app_main() {
    // reserve large block of memory before anything else happens
    TBD_LOGI("main", "AllocateInternalBuffer");
    CTAG::SP::ctagSPAllocator::AllocateInternalBuffer(
        CONFIG_SP_FIXED_MEM_ALLOC_SZ); // TBDings has highest needs of 113944 bytes, take 112k=114688 bytes as default

    TBD_LOGI("main", "FileSystem::begin");
    // init fs
    tbd::drivers::FileSystem::begin();

#if TBD_INDICATOR
    TBD_LOGI("main", "Indicator::init");
    tbd::drivers::Indicator::init();
    tbd::drivers::Indicator::SetLedRGB(0, 0, 255);
#endif

#if TBD_DISPLAY
    TBD_LOGI("main", "Display::Init");
    tbd::Display::Init();
    tbd::Display::ShowFWVersion();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
#endif

#if defined(TBD_API_SERIAL)
    TBD_LOGI("main", "serial api, empty fcn");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
#endif

#if TBD_API_SHELL
    // blocking
    // tbd::api::Shell::begin();
    // return;
#endif

#if TBD_NETWORK
    TBD_LOGI("main", "Network::begin");
    tbd::network::NetworkConfig network_config;
    tbd::Network::SetSSID(network_config.ssid());
    tbd::Network::SetPWD(network_config.pwd());
    tbd::Network::SetIsAccessPoint(network_config.is_access_point());
    tbd::Network::SetIP(network_config.ip());
    tbd::Network::SetMDNSName(network_config.mdns_name());
    tbd::Network::Up();
#endif

#if TBD_API_REST
    TBD_LOGI("main", "rest api begin");
    tbd::api::RestApi::begin();
#endif

#if TBD_API_SERIAL
    TBD_LOGI("main", "serial api begin");
    tbd::api::SerialApi::begin();
#endif

    TBD_LOGI("main", "favorites init");
    tbd::Favorites::init();

    // start the audio processing
    TBD_LOGI("main", "SoundProcessorManager::begin");
    tbd::audio::SoundProcessorManager::begin();
}
}