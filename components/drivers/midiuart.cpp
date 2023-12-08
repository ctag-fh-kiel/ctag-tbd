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

#include "midiuart.hpp"
#include "driver/uart.h"

using namespace CTAG::DRIVERS;

static const int RX_BUF_SIZE = 128;

midiuart::midiuart() {
    const uart_port_t uart_num = UART_NUM_1;
    uart_config_t uart_config = {
            .baud_rate = 31250,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .rx_flow_ctrl_thresh = 0,
            .source_clk = UART_SCLK_DEFAULT,
    };
// Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, 8, 11, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

// Install UART driver using an event queue here
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, RX_BUF_SIZE*2, \
                                        0, 0, NULL, ESP_INTR_FLAG_LOWMED|ESP_INTR_FLAG_SHARED));
}

midiuart::~midiuart(){
    uart_driver_delete(UART_NUM_1);
}

int midiuart::GetBufferSize() const{
    return RX_BUF_SIZE;
}

void midiuart::write(uint8_t *data, std::size_t len){
    uart_write_bytes(UART_NUM_1, (const char *)data, len);
}

void midiuart::read(uint8_t *data, int *len) {
    *len = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 0);
}