#include <atomic>
#include <tbd/spi_inputs/spi_inputs.hpp>
#include <tbd/spi_inputs/module.hpp>

#include <tbd/spi_api/spi_api.hpp>
#include <tbd/control_inputs.hpp>
#include <tbd/ram.hpp>

#include <driver/spi_slave.h>
#include <soc/gpio_num.h>

#include "tusb.hpp"


#define RCV_HOST    SPI2_HOST // SPI2 connects to rp2350 spi1
#define SPI_DATA_SZ 1024 // midi data buffer with header

namespace {

std::atomic<bool> warning_flag;
std::atomic<float> input_level;
std::atomic<float> output_level;

// TBD_DMA spi_slave_transaction_t CTAG::DRIVERS::rp2350_spi_stream::transaction[3];
// TBD_DMA uint32_t CTAG::DRIVERS::rp2350_spi_stream::currentTransaction;
TBD_DMA uint8_t rcvBuf0[SPI_DATA_SZ];
TBD_DMA uint8_t rcvBuf1[SPI_DATA_SZ];
TBD_DMA uint8_t rcvBuf2[SPI_DATA_SZ];
TBD_DMA uint8_t sendBuf0[SPI_DATA_SZ];
TBD_DMA uint8_t sendBuf1[SPI_DATA_SZ];
TBD_DMA uint8_t sendBuf2[SPI_DATA_SZ];

spi_slave_transaction_t transactions[3];
size_t current_transaction = 0;

}

namespace tbd::spi_inputs {
Error begin(){

    //Configuration for the SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = TBD_SPI_INPUTS_PIN_MOSI,
        .miso_io_num = TBD_SPI_INPUTS_PIN_MISO,
        .sclk_io_num = TBD_SPI_INPUTS_PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        // .data_io_default_level = false,
        .max_transfer_sz = SPI_DATA_SZ,
        .flags = 0,
        // .isr_cpu_id = ESP_INTR_CPU_AFFINITY_1,
        .intr_flags = 0
    };

    //Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg = {
        .spics_io_num = GPIO_NUM_28,
        .flags = 0,
        .queue_size = 3,
        .mode = 3,
        .post_setup_cb = nullptr,
        .post_trans_cb = nullptr,
    };

    std::fill_n(rcvBuf0, SPI_DATA_SZ, 0);
    std::fill_n(rcvBuf1, SPI_DATA_SZ, 0);
    std::fill_n(rcvBuf2, SPI_DATA_SZ, 0);
    std::fill_n(sendBuf0, SPI_DATA_SZ, 0);
    std::fill_n(sendBuf1, SPI_DATA_SZ, 0);
    std::fill_n(sendBuf2, SPI_DATA_SZ, 0);

    TBD_LOGI(spi_inputs::tag, "starting SPI inputs");
    auto ret = spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
    assert(ret == ESP_OK);

    sendBuf0[0] = 0xCA; sendBuf0[1] = 0xFE;
    sendBuf1[0] = 0xCA; sendBuf1[1] = 0xFE;
    sendBuf2[0] = 0xCA; sendBuf2[1] = 0xFE;

    transactions[0].length = SPI_DATA_SZ * 8;
    transactions[0].rx_buffer = rcvBuf0;
    transactions[0].tx_buffer = sendBuf0;

    transactions[1].length = SPI_DATA_SZ * 8;
    transactions[1].rx_buffer = rcvBuf1;
    transactions[1].tx_buffer = sendBuf1;

    transactions[2].length = SPI_DATA_SZ * 8;
    transactions[2].rx_buffer = rcvBuf2;
    transactions[2].tx_buffer = sendBuf2;

    // return &rcvBuf0[2]; // skip watermark bytes
    return TBD_OK;
}

TBD_IRAM uint32_t get_current_buffer(uint8_t **dst, uint32_t const max_len, const uint32_t ledStatus) {
    if (max_len > SPI_DATA_SZ - 2) {
        //ESP_LOGE("rp2350_spi_stream", "max_len %d is too large, max is %d", max_len, DATA_SZ - 2);
        return 0; // Invalid length
    }

    // pack led status for current frame
    const auto tx_buf = static_cast<const uint8_t*>(transactions[current_transaction].tx_buffer);
    const auto led = reinterpret_cast<const uint32_t*>(tx_buf + 2);
    *led = ledStatus;

    // pack midi data from USB device midi
    const auto midi_len = reinterpret_cast<const uint32_t*>(tx_buf + 6); // amount of midi bytes to package
    const auto midi_buf = reinterpret_cast<const int8_t*>(tx_buf + 10);
    const uint32_t buf_size = SPI_DATA_SZ - 2 - 4 - 4; // 2 bytes for fingerprint, 4 bytes for led status, 4 bytes for midi length
    *midi_len = tbd::spi_inputs::tusb::read(midi_buf, buf_size);

    // queue transaction of transceive
    if (spi_slave_queue_trans(RCV_HOST, &transactions[current_transaction], 0) != ESP_OK) {
        return 0; // Failed to queue transaction
    }
    current_transaction = (current_transaction + 1) % 3; // switch to next transaction buffer

    // get result of last transaction
    spi_slave_transaction_t* ret_trans;
    if (spi_slave_get_trans_result(RCV_HOST, &ret_trans, 0) != ESP_OK) {
        return 0;
    }

    // grab received buffer
    const auto ret_buf = static_cast<uint8_t*>(ret_trans->rx_buffer);
    // check watermark for valid transaction, if not *dst remains unchanged on previous buffer
    if (ret_buf[0] != 0xCA || ret_buf[1] != 0xFE) {
        return 0; // Invalid transaction
    }

    // data was valid, return new buffer pointer
    *dst = &ret_buf[2];

    return max_len;
}

}

namespace tbd {

void ControlInputs::init() {
    TBD_LOGI(spi_inputs::tag, "initializing input manager");

    spi_api::begin();
}

void TBD_IRAM ControlInputs::update(uint8_t **trigs, float **cvs) {
    // auto data = spi_inputs::get_current_buffer();

    // raw output contains both CVs and triggers in the correct format
    // *cvs = reinterpret_cast<float*>(data);
    // *trigs = &data[N_CVS * 4];
}

void ControlInputs::update_metrics(const sound_processor::ProcessingMetrics& metrics) {
    warning_flag = metrics.warning;
    input_level = metrics.input_level;
    output_level = metrics.output_level;
}

void ControlInputs::SetCVChannelBiPolar(const bool &v0, const bool &v1, const bool &v2, const bool &v3) {

}


void ControlInputs::flush() {
    spi_api::end();
}

}
