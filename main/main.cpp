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


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "adc.hpp"
#include "fs.hpp"
#include "led_rgb.hpp"
#include "gpio.hpp"

#include "Calibration.hpp"
#include "codec.hpp"
#include <vector>
#include "SPManager.hpp"
#include <tbd/sound_processor/allocator.hpp>

#if defined(CONFIG_TBD_PLATFORM_AEM) || defined(CONFIG_TBD_PLATFORM_MK2) || defined(CONFIG_TBD_PLATFORM_BBA)
    #include "Display.hpp"
#endif

using namespace CTAG;

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
    ctagSPAllocator::AllocateInternalBuffer(CONFIG_SP_FIXED_MEM_ALLOC_SZ); // TBDings has highest needs of 113944 bytes, take 112k=114688 bytes as default

    // wait until power is somewhat more settled
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // init fs
    DRIVERS::FileSystem::InitFS();

#ifndef CONFIG_TBD_PLATFORM_STR
    DRIVERS::LedRGB::InitLedRGB();
    DRIVERS::LedRGB::SetLedRGB(0, 0, 255);
#endif

#if defined(CONFIG_TBD_PLATFORM_AEM) || defined(CONFIG_TBD_PLATFORM_MK2) || defined(CONFIG_TBD_PLATFORM_BBA)
    DRIVERS::Display::Init();
    DRIVERS::Display::ShowFWVersion();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
#endif

#if defined(CONFIG_SERIAL_UI)
    vTaskDelay(2000 / portTICK_PERIOD_MS);
#endif

    AUDIO::SoundProcessorManager::StartSoundProcessor();
}
