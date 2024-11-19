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

#include <stdio.h>
#include <vector>

#include "esp_system.h"
#include "adc.hpp"

#include <tbd/drivers/codec.hpp>

#include <tbd/sound_manager.hpp>
#include <tbd/sound_processor/allocator.hpp>

#if TDB_ADC
    #include "gpio.hpp"
#endif

#if TBD_CALIBRATION
    #include "Calibration.hpp"
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

#ifdef CONFIG_WIFI_UI
    #include <tbd/network.hpp>
    #include <tbd/network/config.hpp>
    #include "RestServer.hpp"
#endif


extern "C" {
void app_main();
}

/* as much as possible static:
https://softwareengineering.stackexchange.com/questions/352624/static-vs-non-static-in-embedded-systems
https://www.embedded.com/modern-c-embedded-systems-part-2-evaluating-c/
https://www.embedded.com/modern-c-in-embedded-systems-part-1-myth-and-reality/
*/

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

#if defined(CONFIG_SERIAL_UI)
    vTaskDelay(2000 / portTICK_PERIOD_MS);
#endif

    tbd::audio::SoundProcessorManager::StartSoundProcessor();

    #ifdef CONFIG_WIFI_UI
        tbd::network::NetworkConfig network_config;
        CTAG::NET::Network::SetSSID(network_config.ssid());
        CTAG::NET::Network::SetPWD(network_config.pwd());
        CTAG::NET::Network::SetIsAccessPoint(network_config.is_access_point());
        CTAG::NET::Network::SetIP(network_config.ip());
        CTAG::NET::Network::SetMDNSName(network_config.mdns_name());
        CTAG::NET::Network::Up();
        CTAG::REST::RestServer::StartRestServer();
    #elif CONFIG_SERIAL_UI
        SAPI::SerialAPI::StartSerialAPI();
    #endif
}
