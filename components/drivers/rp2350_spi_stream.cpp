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

#include "sdkconfig.h"
#if CONFIG_TBD_USE_RP2350

#include "rp2350_spi_stream.hpp"

#include <algorithm>
#include <atomic>

#include "esp_log.h"
#include "esp_intr_alloc.h"
#include <cstring>
#include "soc/gpio_num.h"
#include "tusb.hpp"

#include "driver/gpio.h"

#include "link.hpp"

#define RCV_HOST    SPI2_HOST // SPI2 connects to rp2350 spi1
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
bool CTAG::DRIVERS::rp2350_spi_stream::receiving = false;
uint32_t CTAG::DRIVERS::rp2350_spi_stream::transferErrorCount = 0;
uint32_t CTAG::DRIVERS::rp2350_spi_stream::transferSuccessCount = 0;
uint32_t CTAG::DRIVERS::rp2350_spi_stream::parseErrorCount = 0;
uint32_t CTAG::DRIVERS::rp2350_spi_stream::queueErrorCount = 0;

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
    transferErrorCount = 0;
    transferSuccessCount = 0;
    queueErrorCount = 0;
    parseErrorCount = 0;

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
        .max_transfer_sz = STREAM_BUFFER_SIZE_,
        .flags = 0,
        .isr_cpu_id = ESP_INTR_CPU_AFFINITY_1,
        .intr_flags = ESP_INTR_FLAG_LEVEL3|ESP_INTR_FLAG_IRAM
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
    gpio_set_level(GPIO_HANDSHAKE, 0);

    //Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
    gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY);

    ESP_LOGI("rp2350 spi", "Init()");
    esp_err_t ret = spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    // IDF 6 requires the SPI host to be initialized before allocating
    // host-specific DMA buffers with spi_bus_dma_memory_alloc().
    rcvBuf0 = (uint8_t*) spi_bus_dma_memory_alloc(RCV_HOST, STREAM_BUFFER_SIZE_, 0);
    rcvBuf1 = (uint8_t*) spi_bus_dma_memory_alloc(RCV_HOST, STREAM_BUFFER_SIZE_, 0);
    rcvBuf2 = (uint8_t*) spi_bus_dma_memory_alloc(RCV_HOST, STREAM_BUFFER_SIZE_, 0);
    sendBuf0 = (uint8_t*) spi_bus_dma_memory_alloc(RCV_HOST, STREAM_BUFFER_SIZE_, 0);
    sendBuf1 = (uint8_t*) spi_bus_dma_memory_alloc(RCV_HOST, STREAM_BUFFER_SIZE_, 0);
    sendBuf2 = (uint8_t*) spi_bus_dma_memory_alloc(RCV_HOST, STREAM_BUFFER_SIZE_, 0);
    ESP_ERROR_CHECK((rcvBuf0 && rcvBuf1 && rcvBuf2 && sendBuf0 && sendBuf1 && sendBuf2) ? ESP_OK : ESP_ERR_NO_MEM);

    std::fill_n(rcvBuf0, STREAM_BUFFER_SIZE_, 0);
    std::fill_n(rcvBuf1, STREAM_BUFFER_SIZE_, 0);
    std::fill_n(rcvBuf2, STREAM_BUFFER_SIZE_, 0);
    std::fill_n(sendBuf0, STREAM_BUFFER_SIZE_, 0);
    std::fill_n(sendBuf1, STREAM_BUFFER_SIZE_, 0);
    std::fill_n(sendBuf2, STREAM_BUFFER_SIZE_, 0);

    sendBuf0[0] = 0xCA; sendBuf0[1] = 0xFE;
    sendBuf1[0] = 0xCA; sendBuf1[1] = 0xFE;
    sendBuf2[0] = 0xCA; sendBuf2[1] = 0xFE;

    transaction[0].length = STREAM_BUFFER_SIZE_ * 8;
    transaction[0].rx_buffer = rcvBuf0;
    transaction[0].tx_buffer = sendBuf0;

    transaction[1].length = STREAM_BUFFER_SIZE_ * 8;
    transaction[1].rx_buffer = rcvBuf1;
    transaction[1].tx_buffer = sendBuf1;

    transaction[2].length = STREAM_BUFFER_SIZE_ * 8;
    transaction[2].rx_buffer = rcvBuf2;
    transaction[2].tx_buffer = sendBuf2;

    currentTransaction = 0;

    return &rcvBuf0[2]; // skip watermark bytes
}

IRAM_ATTR uint32_t CTAG::DRIVERS::rp2350_spi_stream::GetCurrentBuffer(void *sendbuffer, void **receivebuffer) {
    // uint8_t* tx_buf = (uint8_t*) transaction[currentTransaction].tx_buffer;

    // tx_buf[0] = 0xCA;
    // tx_buf[1] = 0xFE;
    // memcpy(tx_buf + 2, sendbuffer, STREAM_BUFFER_SIZE_ - 2);

    // // queue transaction of transceive
    // esp_err_t ret;
    // ret = spi_slave_queue_trans(RCV_HOST, &transaction[currentTransaction], 0);
    // if (ESP_OK != ret) {
    //     // ESP_LOGD("rp2350_spi_stream", "Failed to queue transaction: %s", esp_err_to_name(ret));
    //     queueErrorCount++;
    //     return 0; // Failed to queue transaction
    // }
    // currentTransaction = (currentTransaction + 1) % 3; // switch to next transaction buffer

    // // get result of last transaction
    // spi_slave_transaction_t* ret_trans;
    // ret = spi_slave_get_trans_result(RCV_HOST, &ret_trans, 0);
    // if (ESP_OK != ret) {
    //     transferErrorCount++;
    //     // ESP_LOGE("rp2350_spi_stream", "Failed receive transaction: %s", esp_err_to_name(ret));
    //     return 0;
    // }

    // // grab received buffer
    // uint8_t* ret_buf = (uint8_t*)ret_trans->rx_buffer;
    // if (ret_trans->length != STREAM_BUFFER_SIZE_ * 8 ) { // size is in bits
    //     transferErrorCount++;
    //     ESP_LOGE("rp2350_spi_stream", "Failed receive length (%d)", ret_trans->length);
    //     return 0;
    // };

    // // check watermark for valid transaction, if not *dst remains unchanged on previous buffer
    // if (ret_buf[0] != 0xCA || ret_buf[1] != 0xFE) {
    //     // ESP_LOGE("rp2350_spi_stream", "Invalid transaction received (%d bits), expected CA FE, got [%02X %02X] %02X %02X %02X %02X",
    //     //     ret_trans->length, ret_buf[0], ret_buf[1], ret_buf[2], ret_buf[3], ret_buf[4], ret_buf[5]);
    //     parseErrorCount++;
    //     return 0; // Invalid transaction
    // }

    // // data was valid, return new buffer pointer
    // *receivebuffer = ret_buf + 2;
    // // memcpy(receivebuffer, ret_buf + 2, STREAM_BUFFER_SIZE_ - 2);
    // // memset(ret_buf, 0, STREAM_BUFFER_SIZE_); // clear received buffer after processing
    // transferSuccessCount++;

    // /*
    // static int val = 0;
    // if (ret_buf[2+N_CVS*4] != val) {
    //     val = ret_buf[2+N_CVS*4];
    //     xQueueSend(debug_queue, &val, 0);
    // }
    // */

    // return STREAM_BUFFER_SIZE_ - 2;
    return 0;
}

IRAM_ATTR bool CTAG::DRIVERS::rp2350_spi_stream::QueueBuffer(void *sendbuffer) {
    uint8_t* tx_buf = (uint8_t*) transaction[currentTransaction].tx_buffer;

    tx_buf[0] = 0xCA;
    tx_buf[1] = 0xFE;
    memcpy(tx_buf + 2, sendbuffer, STREAM_BUFFER_SIZE_ - 2);

    // queue transaction of transceive
    esp_err_t ret;
    ret = spi_slave_queue_trans(RCV_HOST, &transaction[currentTransaction], 0);
    if (ESP_OK != ret) {
        // ESP_LOGD("rp2350_spi_stream", "Failed to queue transaction: %s", esp_err_to_name(ret));
        queueErrorCount++;
        return false; // Failed to queue transaction
    }

    // currentTransaction = (currentTransaction + 1) % 3; // switch to next transaction buffer

    // // get result of last transaction
    // spi_slave_transaction_t* ret_trans;
    // ret = spi_slave_get_trans_result(RCV_HOST, &ret_trans, 0);
    // if (ESP_OK != ret) {
    //     transferErrorCount++;
    //     // ESP_LOGE("rp2350_spi_stream", "Failed receive transaction: %s", esp_err_to_name(ret));
    //     return 0;
    // }

    // // grab received buffer
    // uint8_t* ret_buf = (uint8_t*)ret_trans->rx_buffer;
    // if (ret_trans->length != STREAM_BUFFER_SIZE_ * 8 ) { // size is in bits
    //     transferErrorCount++;
    //     ESP_LOGE("rp2350_spi_stream", "Failed receive length (%d)", ret_trans->length);
    //     return 0;
    // };

    // // check watermark for valid transaction, if not *dst remains unchanged on previous buffer
    // if (ret_buf[0] != 0xCA || ret_buf[1] != 0xFE) {
    //     // ESP_LOGE("rp2350_spi_stream", "Invalid transaction received (%d bits), expected CA FE, got [%02X %02X] %02X %02X %02X %02X",
    //     //     ret_trans->length, ret_buf[0], ret_buf[1], ret_buf[2], ret_buf[3], ret_buf[4], ret_buf[5]);
    //     parseErrorCount++;
    //     return 0; // Invalid transaction
    // }

    // // data was valid, return new buffer pointer
    // *receivebuffer = ret_buf + 2;
    // // memcpy(receivebuffer, ret_buf + 2, STREAM_BUFFER_SIZE_ - 2);
    // // memset(ret_buf, 0, STREAM_BUFFER_SIZE_); // clear received buffer after processing
    // transferSuccessCount++;

    // /*
    // static int val = 0;
    // if (ret_buf[2+N_CVS*4] != val) {
    //     val = ret_buf[2+N_CVS*4];
    //     xQueueSend(debug_queue, &val, 0);
    // }
    // */

    // return STREAM_BUFFER_SIZE_ - 2;
    return true;
}


IRAM_ATTR void CTAG::DRIVERS::rp2350_spi_stream::GetSendBuffer(void **sendbuffer) {
    uint8_t* tx_buf = (uint8_t*) transaction[currentTransaction].tx_buffer;
    *sendbuffer = tx_buf + 2;
}

IRAM_ATTR bool CTAG::DRIVERS::rp2350_spi_stream::GetReceivedBuffer(void **receivebuffer) {
    *receivebuffer = nullptr;
    uint8_t* tx_buf = (uint8_t*) transaction[currentTransaction].tx_buffer;

    // tx_buf[0] = 0xCA;
    // tx_buf[1] = 0xFE;
    // memcpy(tx_buf + 2, sendbuffer, STREAM_BUFFER_SIZE_ - 2);

    esp_err_t ret;
    // if (!receiving) {
    //     receiving = true;
    //     // ESP_LOGI("rp2350_spi_stream", "GetReceivedBuffer: queueing transaction %d", currentTransaction);
    //     // queue transaction of transceive
    //     ret = spi_slave_queue_trans(RCV_HOST, &transaction[currentTransaction], 0);
    //     if (ESP_OK != ret) {
    //         // ESP_LOGD("rp2350_spi_stream", "Failed to queue transaction: %s", esp_err_to_name(ret));
    //         queueErrorCount++;
    //         return 0; // Failed to queue transaction
    //     }
    // }

    // get result of last transaction
    spi_slave_transaction_t* ret_trans;
    ret = spi_slave_get_trans_result(RCV_HOST, &ret_trans, 0);
    // ESP_LOGI("rp2350_spi_stream", "GetReceivedBuffer: got transaction result %d", ret);
    if (ESP_OK != ret) {
        // transferErrorCount++;
        // ESP_LOGE("rp2350_spi_stream", "Failed receive transaction: %s", esp_err_to_name(ret));
        return 0;
    }

    // grab received buffer
    uint8_t* ret_buf = (uint8_t*)ret_trans->rx_buffer;
    // ESP_LOGI("rp2350_spi_stream", "GetReceivedBuffer: got transaction %d, %d bytes",
    //   currentTransaction, ret_trans->length / 8);
    if (ret_trans->length != STREAM_BUFFER_SIZE_ * 8 ) { // size is in bits
        transferErrorCount++;
        ESP_LOGE("rp2350_spi_stream", "Failed receive length (%d)", ret_trans->length);
        return 0;
    };

    // got transmission, start receiving again
    receiving = false;
    currentTransaction = (currentTransaction + 1) % 3; // switch to next transaction buffer

    // check watermark for valid transaction, if not *dst remains unchanged on previous buffer
    if (ret_buf[0] != 0xCA || ret_buf[1] != 0xFE) {
        ESP_LOGE("rp2350_spi_stream", "Invalid transaction received (%d bits), expected CA FE, got [%02X %02X] %02X %02X %02X %02X",
            ret_trans->length, ret_buf[0], ret_buf[1], ret_buf[2], ret_buf[3], ret_buf[4], ret_buf[5]);
        parseErrorCount++;
        return 0; // Invalid transaction
    }

    *receivebuffer = ret_buf + 2;
    return true;
}

#endif // CONFIG_TBD_USE_RP2350
