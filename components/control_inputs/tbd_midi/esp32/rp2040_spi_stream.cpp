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
#if TBD_MIDI_INPUT_RP2040

#include "rp2040_spi_stream.hpp"

#include "driver/spi_slave.h"
#include "esp_log.h"
#include <esp_intr_alloc.h>
#include <cstring>


#define GPIO_MOSI 17
#define GPIO_MISO 16
#define GPIO_SCLK 15
#define GPIO_CS 18
#define RCV_HOST    SPI2_HOST
#define DATA_SZ 64 // midi data buffer with header


namespace {

TBD_DRAM spi_slave_transaction_t transaction[2];
TBD_DRAM uint32_t currentTransaction;
TBD_DMA static uint8_t buf0[DATA_SZ];
TBD_DMA static uint8_t buf1[DATA_SZ];
TBD_DMA static uint8_t sendBuf[DATA_SZ];

}

namespace tbd::drivers {

void rp2040_spi_stream::init(){
    //Configuration for the SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .max_transfer_sz = DATA_SZ,
        .flags = 0,
        .isr_cpu_id = INTR_CPU_ID_0,
        .intr_flags = 0
    };

    //Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg = {
        .spics_io_num = GPIO_CS,
        .flags = 0,
        .queue_size = 2,
        .mode = 3,
        .post_setup_cb = 0,
        .post_trans_cb = 0
    };

    ESP_LOGI("rp2040 spi", "init()");
    auto ret = spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
    assert(ret == ESP_OK);

    transaction[0].length = DATA_SZ * 8;
    transaction[0].rx_buffer = buf0;
    transaction[0].tx_buffer = sendBuf;

    transaction[1].length = DATA_SZ * 8;
    transaction[1].rx_buffer = buf1;
    transaction[1].tx_buffer = sendBuf;

    currentTransaction = 0;
}

IRAM_ATTR size_t rp2040_spi_stream::read(uint8_t* data, size_t max_len){
    // this is all non-blocking
    spi_slave_queue_trans(RCV_HOST, &transaction[currentTransaction], 0);
    currentTransaction ^= 0x1;

    // get result of last transaction
    spi_slave_transaction_t* ret_trans;
    auto ret = spi_slave_get_trans_result(RCV_HOST, &ret_trans, 0);

    if (ESP_OK == ret){
        auto* ptr = (uint8_t*)ret_trans->rx_buffer;
        if (ptr[0] == 0xCA && ptr[1] == 0xFE && ptr[2] != 0){
            // TODO: check if max_len is enough
            if (ptr[2] > max_len){
                ESP_LOGE("rp2040 spi", "buffer too small");
                return 0;
            }
            memcpy(data, ptr + 3, ptr[2]);
            return ptr[2];
        }
    }

    return 0;
}

}

#endif