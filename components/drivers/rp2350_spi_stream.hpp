/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020, 2024 by Robert Manzke. All rights reserved.

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
#include <stdint.h>
#include <atomic>

namespace CTAG {
    namespace DRIVERS {

        class rp2350_spi_stream final{
        public:
            rp2350_spi_stream() = delete;
            // Initialize the SPI stream, returns pointer to the buffer
            static uint8_t* Init();
            // returns effective length of buffer, first arg is pointer to buffer, second arg is led status, which is sent to subsystem
            static uint32_t GetCurrentBuffer(void *sendbuffer, void **receivebuffer);
            static uint32_t GetBufferSize() {return STREAM_BUFFER_SIZE_ - 2;}
            static uint32_t queueErrorCount;
            static uint32_t transferErrorCount;
            static uint32_t transferSuccessCount;
            static uint32_t parseErrorCount;
            static bool receiving;
            static bool QueueBuffer(void *sendbuffer);
            static void GetSendBuffer(void **sendbuffer);
            static bool GetReceivedBuffer(void **receivebuffer);
        private:
            static const uint32_t STREAM_BUFFER_SIZE_ {512}; // midi data buffer with header
            static spi_slave_transaction_t transaction[3];
            static uint32_t currentTransaction;
            static uint32_t buf_sz_dynamic; // remaining dynamic size of buffer after default data fields (watermark, ableton ...)
        };
    }
}