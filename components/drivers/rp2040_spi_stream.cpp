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

#include "rp2040_spi_stream.hpp"

#include "esp_log.h"
#include "esp_intr_alloc.h"
#include <cstring>
#include "soc/gpio_num.h"


#define RCV_HOST    SPI2_HOST
#define DATA_SZ 2048 // midi data buffer with header

DRAM_ATTR spi_slave_transaction_t CTAG::DRIVERS::rp2040_spi_stream::transaction[2];
DRAM_ATTR uint32_t CTAG::DRIVERS::rp2040_spi_stream::currentTransaction;
DMA_ATTR static uint8_t *buf0;
DMA_ATTR static uint8_t *buf1;
DMA_ATTR static uint8_t *sendBuf;

void CTAG::DRIVERS::rp2040_spi_stream::Init(){
    //Configuration for the SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_NUM_31,
        .miso_io_num = GPIO_NUM_29,
        .sclk_io_num = GPIO_NUM_30,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .max_transfer_sz = DATA_SZ,
        .flags = 0,
        .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
        .intr_flags = 0
    };

    //Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg = {
        .spics_io_num = GPIO_NUM_28,
        .flags = 0,
        .queue_size = 2,
        .mode = 3,
        .post_setup_cb = 0,
        .post_trans_cb = 0
    };

    buf0 = (uint8_t*) spi_bus_dma_memory_alloc(SPI2_HOST, DATA_SZ, 0);
    buf1 = (uint8_t*) spi_bus_dma_memory_alloc(SPI2_HOST, DATA_SZ, 0);
    sendBuf = (uint8_t*) spi_bus_dma_memory_alloc(SPI2_HOST, DATA_SZ, 0);

    ESP_LOGI("rp2040 spi", "Init()");
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

IRAM_ATTR uint32_t CTAG::DRIVERS::rp2040_spi_stream::Read(uint8_t* data, uint32_t max_len){
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
