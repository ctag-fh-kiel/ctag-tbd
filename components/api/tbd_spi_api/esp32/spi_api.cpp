#include <tbd/spi_inputs/spi_inputs.hpp>

#include <driver/spi_common.h>
#include <driver/spi_slave.h>
#include <soc/gpio_num.h>

#include <tbd/system/port/heaps.hpp>
#include <tbd/api/api_adapters.hpp>

#include <tbd/ram.hpp>

namespace tbd::spi_api {

namespace impl {
struct SPI_SERVER;

// variables

bool expecting_payload_transaction = false;

api::impl::ApiReader<SPI_SERVER, false> packet_reader;
api::impl::ApiPacketHandler<SPI_SERVER, false> packet_handler;

spi_slave_transaction_t header_transaction_ {
    .flags = 0,
    .length = sizeof(api::Packet::HEADER_SIZE),
    .tx_buffer = packet_handler.output_buffer(),
    .rx_buffer = packet_reader.input_buffer(),
};

spi_slave_transaction_t data_transaction_ {
    .flags = 0,
    .length = 0,
    .tx_buffer = packet_handler.output_buffer() + api::Packet::HEADER_SIZE,
    .rx_buffer = packet_reader.input_buffer() + api::Packet::HEADER_SIZE,
};

// functions

Error begin();
Error end();
void do_work();

Error init_bus();
void do_handshake_transaction();
void do_data_transaction();

// implementations

Error begin() {
    return init_bus();
}

Error end() {
    if (spi_slave_free(TBD_SPI_INPUTS_SPI_HOST) != ESP_OK) {
        return TBD_ERR(SPI_INPUTS_FAILED_TO_DEINITIALIZE_SPI);
    }
    return TBD_OK;
}

void do_work() {
    if (!expecting_payload_transaction) {
        do_handshake_transaction();
    } else {
        do_data_transaction();
    }
}

Error init_bus() {
    spi_bus_config_t buscfg = {
        .mosi_io_num = TBD_SPI_INPUTS_SPI_PIN_MOSI,
        .miso_io_num = TBD_SPI_INPUTS_SPI_PIN_MISO,
        .sclk_io_num = TBD_SPI_INPUTS_SPI_PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        // .data_io_default_level = false,
        .max_transfer_sz = 2048,
        .flags = 0,
        .isr_cpu_id = ESP_INTR_CPU_AFFINITY_0,
        .intr_flags = 0
    };

    spi_slave_interface_config_t slvcfg = {
        .spics_io_num = GPIO_NUM_20,
        .flags = 0,
        .queue_size = 1,
        .mode = 3,
        .post_setup_cb = nullptr,
        .post_trans_cb = nullptr,
    };

    // send_buffer = (uint8_t*)spi_bus_dma_memory_alloc(RCV_HOST, 2048, 0);
    // send_buffer[0] = 0xCA;
    // send_buffer[1] = 0xFE;
    // receive_buffer = (uint8_t*)spi_bus_dma_memory_alloc(RCV_HOST, 2048, 0);
    // transaction.length = 2048 * 8;
    // transaction.tx_buffer = send_buffer;
    // transaction.rx_buffer = receive_buffer;

    if (spi_slave_initialize(TBD_SPI_INPUTS_SPI_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO) != ESP_OK) {
        return TBD_ERR(SPI_INPUTS_FAILED_TO_INITIALIZE_SPI);
    }

    // xTaskCreatePinnedToCore(api_task, "SpiAPI", 4096 * 2, nullptr, 10, &hTask, 0);
    return TBD_OK;
}

void do_handshake_transaction() {
    if (spi_slave_transmit(TBD_SPI_INPUTS_SPI_HOST, &header_transaction_, portMAX_DELAY) != ESP_OK) {
        //
    }
    if (packet_reader.type_peek() != api::Header::TYPE_NOOP && data_transaction_.length != 0) {
        expecting_payload_transaction = true;
    }
    expecting_payload_transaction = false;
}

void do_data_transaction() {
    if (spi_slave_transmit(TBD_SPI_INPUTS_SPI_HOST, &data_transaction_, portMAX_DELAY) != ESP_OK) {
        api::impl::ApiPacketHandler<SPI_SERVER, true> packet_handler;
        const auto response_length = packet_handler.handle(packet_reader.input_buffer(), data_transaction_.trans_len);
        if (response_length == 0) {
            return;
        }
    }
    expecting_payload_transaction = false;
    packet_handler.handle(packet_reader.input_buffer(), data_transaction_.trans_len);
    expecting_payload_transaction = false;
}

}

    // TaskHandle_t SpiAPI::hTask;
    // spi_slave_transaction_t SpiAPI::transaction;
    // uint8_t *SpiAPI::send_buffer, *SpiAPI::receive_buffer;

Error begin() {
    return impl::begin();
}

Error end() {
    return impl::end();
}

void do_work() {
    impl::do_work();
}

}
