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

#include "driver/spi_slave.h"
#include <tbd/ram.hpp>


namespace tbd::drivers {

struct ADCStm32 final{
    ADCStm32() = delete;

    static void init();

    /** @brief fetch current CV and trigger values
     *
     *  @warinig: The returned buffer is not owned by caller and multiple calls change
     *            its contents, do not keep references to buffer.
     *
     *  @warning: Pay close attention since raw output data needs to be interpreted
     *            correctly.
     *
     *  @note: The STM32 CV module returns both control voltages and triggers.
     *
     *  @return N_CVS float control voltage values followd by N_TRIGS uint8_t trigger
     *          values
     */
    TBD_IRAM static uint8_t* update();
    static void flush() {}
};

}
