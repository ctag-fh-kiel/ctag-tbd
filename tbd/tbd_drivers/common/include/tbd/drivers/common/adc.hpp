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

#include <cinttypes>
#include <tbd/ram.hpp>


namespace tbd::drivers {

struct ADC {
    ADC() = delete;

    static void SetCVINUnipolar(int ch);
    static void SetCVINBipolar(int ch);
    static uint16_t GetChannelVal(int ch);
    static void GetChannelVals(uint16_t *);

    static void init();

    /** fetch current CV values
     *
     *  @warinig: The returned buffer is not owned by caller and multiple calls change
     *            its contents, do not keep references to buffer.
     *
     *  @warning: Pay close attention since raw output data needs to be interpreted
     *            correctly.
     *
     *  @note: The ADC module only returns control voltages.
     *
     *  @return N_CVS uint16_t CV values
     */
    TBD_IRAM static uint8_t* update();
    static void flush() {}
//
// protected:
//     static void init_ulp_program();
};

}
