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
#include <tbd/drivers/common/cv_input_stm32.hpp>

#include "esp_log.h"
#include "esp_intr_alloc.h"


#define GPIO_MOSI 13
#define GPIO_MISO 12
#define GPIO_SCLK 14
#define GPIO_CS 15
#define RCV_HOST    HSPI_HOST
#define DMA_CHAN    2
#define DATA_SZ 102 //N_CVS*4 + N_TRIGS + 2 // is ncvs * 4 + ntrigs + 2 reserved

namespace {
    TBD_DRAM spi_slave_transaction_t transaction[2];
    TBD_DRAM uint32_t currentTransaction;
    TBD_DMA uint8_t buf0[DATA_SZ];
    TBD_DMA uint8_t buf1[DATA_SZ];
}

namespace tbd::drivers {

void CVInputsStm32::Init() {
    //Configuration for the SPI bus
    spi_bus_config_t buscfg={
            .mosi_io_num=GPIO_MOSI,
            .miso_io_num=GPIO_MISO,
            .sclk_io_num=GPIO_SCLK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = DATA_SZ,
            .flags = 0,
            .intr_flags = 0
    };

    //Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg={
            .spics_io_num=GPIO_CS,
            .flags=0,
            .queue_size=2,
            .mode=0,
            .post_setup_cb=0,
            .post_trans_cb=0
    };

    esp_err_t ret;
    ret=spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, DMA_CHAN);
    assert(ret==ESP_OK);

    transaction[0].length = DATA_SZ*8;
    transaction[0].rx_buffer = buf0;
    transaction[0].tx_buffer = NULL;

    transaction[1].length = DATA_SZ*8;
    transaction[1].rx_buffer = buf1;
    transaction[1].tx_buffer = NULL;

    currentTransaction = 0;
}

TBD_IRAM void * CVInputsStm32::Update() {
    esp_err_t ret;
    spi_slave_queue_trans(RCV_HOST, &transaction[currentTransaction], portMAX_DELAY);
    currentTransaction ^= 0x1;

    // get result of last transaction
    spi_slave_transaction_t *ret_trans;
    ret = spi_slave_get_trans_result(RCV_HOST, &ret_trans, 0);

    if(ESP_OK == ret){
        return ret_trans->rx_buffer;
    }

    return transaction[currentTransaction].rx_buffer;
}

}
