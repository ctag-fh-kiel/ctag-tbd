/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020, 2024 by Robert Manzke. All rights reserved.
(c) 2023 MIDI-Message-Parser aka 'bba_update()' by Mathias Brüssel.

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

namespace CTAG {
    namespace DRIVERS {
        class rp2350_spi_stream final{
        public:
            rp2350_spi_stream() = delete;
            static void Init();
            static uint32_t Read(uint8_t* data, uint32_t max_len);
            static uint32_t CopyCurrentBuffer(uint8_t *dst, uint32_t const max_len);
        private:
            static spi_slave_transaction_t transaction[2];
            static uint32_t currentTransaction;
        };
    }
}