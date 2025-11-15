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

#include "rp2350_spi_stream.hpp"

#include <algorithm>

#include "esp_log.h"
#include "esp_intr_alloc.h"
#include <cstring>
#include "soc/gpio_num.h"
#include "tusb.hpp"

#include "driver/gpio.h"


#define RCV_HOST    SPI2_HOST // SPI2 connects to rp2350 spi1
#define SPI_DATA_SZ 1024 // midi data buffer with header
#define GPIO_HANDSHAKE GPIO_NUM_51 // handshake line, P4_PICO_03 which is GPIO22 on rp2350
#define GPIO_MOSI GPIO_NUM_31
#define GPIO_MISO GPIO_NUM_29
#define GPIO_SCLK GPIO_NUM_30
#define GPIO_CS GPIO_NUM_28

DRAM_ATTR spi_slave_transaction_t CTAG::DRIVERS::rp2350_spi_stream::transaction[3];
DRAM_ATTR uint32_t CTAG::DRIVERS::rp2350_spi_stream::currentTransaction;
DMA_ATTR static uint8_t *rcvBuf0;
DMA_ATTR static uint8_t *rcvBuf1;
DMA_ATTR static uint8_t *rcvBuf2;
DMA_ATTR static uint8_t *sendBuf0;
DMA_ATTR static uint8_t *sendBuf1;
DMA_ATTR static uint8_t *sendBuf2;

/*
QueueHandle_t debug_queue = nullptr;
static void debug_thread(void* arg) {
    int val = 0;
    while (true){
        xQueueReceive(debug_queue, &val, portMAX_DELAY);
        ESP_LOGI("rp2350_spi_stream", "debug_thread: %d", val);
    }
}
*/

// Called after a transaction is queued and ready for pickup by master. We use this to set the handshake line high.
IRAM_ATTR static void spi_post_setup_cb(spi_slave_transaction_t *trans){
    gpio_set_level(GPIO_HANDSHAKE, 1);
}

// Called after transaction is sent/received. We use this to set the handshake line low.
IRAM_ATTR static void spi_post_trans_cb(spi_slave_transaction_t *trans){
    gpio_set_level(GPIO_HANDSHAKE, 0);
}

uint8_t* CTAG::DRIVERS::rp2350_spi_stream::Init(){

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
        .data_io_default_level = false,
        .max_transfer_sz = SPI_DATA_SZ,
        .flags = 0,
        .isr_cpu_id = ESP_INTR_CPU_AFFINITY_1,
        .intr_flags = 0
    };

    //Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg = {
        .spics_io_num = GPIO_CS,
        .flags = 0,
        .queue_size = 3,
        .mode = 3,
        .post_setup_cb = spi_post_setup_cb,
        .post_trans_cb = spi_post_trans_cb
    };

    //Configuration for the handshake line
    gpio_config_t io_conf = {
        .pin_bit_mask = BIT64(GPIO_HANDSHAKE),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
        .hys_ctrl_mode = GPIO_HYS_SOFT_DISABLE
    };

    //Configure handshake line as output
    gpio_config(&io_conf);

    //Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
    gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY);

    rcvBuf0 = (uint8_t*) spi_bus_dma_memory_alloc(RCV_HOST, SPI_DATA_SZ, 0);
    rcvBuf1 = (uint8_t*) spi_bus_dma_memory_alloc(RCV_HOST, SPI_DATA_SZ, 0);
    rcvBuf2 = (uint8_t*) spi_bus_dma_memory_alloc(RCV_HOST, SPI_DATA_SZ, 0);
    sendBuf0 = (uint8_t*) spi_bus_dma_memory_alloc(RCV_HOST, SPI_DATA_SZ, 0);
    sendBuf1 = (uint8_t*) spi_bus_dma_memory_alloc(RCV_HOST, SPI_DATA_SZ, 0);
    sendBuf2 = (uint8_t*) spi_bus_dma_memory_alloc(RCV_HOST, SPI_DATA_SZ, 0);

    std::fill_n(rcvBuf0, SPI_DATA_SZ, 0);
    std::fill_n(rcvBuf1, SPI_DATA_SZ, 0);
    std::fill_n(rcvBuf2, SPI_DATA_SZ, 0);
    std::fill_n(sendBuf0, SPI_DATA_SZ, 0);
    std::fill_n(sendBuf1, SPI_DATA_SZ, 0);
    std::fill_n(sendBuf2, SPI_DATA_SZ, 0);

    ESP_LOGI("rp2350 spi", "Init()");
    auto ret = spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
    assert(ret == ESP_OK);

    sendBuf0[0] = 0xCA; sendBuf0[1] = 0xFE;
    sendBuf1[0] = 0xCA; sendBuf1[1] = 0xFE;
    sendBuf2[0] = 0xCA; sendBuf2[1] = 0xFE;

    transaction[0].length = SPI_DATA_SZ * 8;
    transaction[0].rx_buffer = rcvBuf0;
    transaction[0].tx_buffer = sendBuf0;

    transaction[1].length = SPI_DATA_SZ * 8;
    transaction[1].rx_buffer = rcvBuf1;
    transaction[1].tx_buffer = sendBuf1;

    transaction[2].length = SPI_DATA_SZ * 8;
    transaction[2].rx_buffer = rcvBuf2;
    transaction[2].tx_buffer = sendBuf2;

    currentTransaction = 0;

    /*
    debug_queue = xQueueCreate(20, sizeof(int));
    xTaskCreatePinnedToCore(debug_thread, "debug_thread", 2048, NULL, 5, NULL, 0);
    */

    return &rcvBuf0[2]; // skip watermark bytes
}

IRAM_ATTR uint32_t CTAG::DRIVERS::rp2350_spi_stream::GetCurrentBuffer(uint8_t **dst, uint32_t const max_len, uint32_t ledStatus) {
    if (max_len > SPI_DATA_SZ - 2) {
        //ESP_LOGE("rp2350_spi_stream", "max_len %d is too large, max is %d", max_len, DATA_SZ - 2);
        return 0; // Invalid length
    }

    // pack led status for current frame
    uint8_t* tx_buf = (uint8_t*) transaction[currentTransaction].tx_buffer;
    uint32_t *led = (uint32_t*) &tx_buf[2];
    *led = ledStatus;

    // pack midi data from USB device midi
    uint32_t *midi_len = (uint32_t*) &tx_buf[6]; // amount of midi bytes to package
    uint8_t *midi_buf = (uint8_t*) &tx_buf[10];
    const uint32_t buf_size = SPI_DATA_SZ - 2 - 4 - 4; // 2 bytes for fingerprint, 4 bytes for led status, 4 bytes for midi length
    *midi_len = tusb::Read(midi_buf, buf_size);

    // queue transaction of transceive
    esp_err_t ret;
    ret = spi_slave_queue_trans(RCV_HOST, &transaction[currentTransaction], 0);
    if (ESP_OK != ret) {
        //ESP_LOGE("rp2350_spi_stream", "Failed to queue transaction: %s", esp_err_to_name(ret));
        return 0; // Failed to queue transaction
    }
    currentTransaction = (currentTransaction + 1) % 3; // switch to next transaction buffer

    // get result of last transaction
    spi_slave_transaction_t* ret_trans;
    ret = spi_slave_get_trans_result(RCV_HOST, &ret_trans, 0);
    if (ESP_OK != ret) {
        //ESP_LOGE("rp2350_spi_stream", "Failed receive transaction: %s", esp_err_to_name(ret));
        return 0;
    }

    // grab received buffer
    uint8_t* ret_buf = (uint8_t*)ret_trans->rx_buffer;

    // check watermark for valid transaction, if not *dst remains unchanged on previous buffer
    if (ret_buf[0] != 0xCA || ret_buf[1] != 0xFE) {
        //ESP_LOGE("rp2350_spi_stream", "Invalid transaction received, expected CA FE, got %02X %02X", ret_buf[0], ret_buf[1]);
        return 0; // Invalid transaction
    }

    // data was valid, return new buffer pointer
    *dst = &ret_buf[2];

    /*
    static int val = 0;
    if (ret_buf[2+N_CVS*4] != val) {
        val = ret_buf[2+N_CVS*4];
        xQueueSend(debug_queue, &val, 0);
    }
    */

    return max_len;
}