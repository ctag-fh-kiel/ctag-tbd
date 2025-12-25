/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.
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

#include "Control.hpp"
#include "rp2350_spi_stream.hpp"

uint8_t *CTAG::CTRL::Control::buf_ptr = nullptr; // buffer pointer for current cv + trig data

IRAM_ATTR void CTAG::CTRL::Control::Update(void **data, uint32_t ledStatus) {
    CTAG::DRIVERS::rp2350_spi_stream::GetCurrentBuffer(data, ledStatus);
}

void CTAG::CTRL::Control::Init() {
    ESP_LOGI("Control", "Initializing control! %d CVs, %d Trigs", N_CVS, N_TRIGS);
    buf_ptr = DRIVERS::rp2350_spi_stream::Init();
}