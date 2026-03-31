/***************
TBD-16 — Pico Firmware Update System

(c) 2024-2026 Johannes Elias Lohbihler for dadamachines

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

dadamachines has a commercial license to use this code in the TBD-16 product.
Other commercial use requires a separate license agreement.

Provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "sdkconfig.h"
#if CONFIG_TBD_USE_RP2350

#include "picoboot3_master.hpp"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstring>

static const char *TAG = "Picoboot3";

// SPI3_HOST pins for picoboot3 master mode.
// In NORMAL operation, the Pico is SPI master: Pico TX(GPIO35) → P4 GPIO23, Pico RX(GPIO32) ← P4 GPIO22.
// In PICOBOOT3 mode, the P4 becomes master, so MOSI/MISO swap relative to normal labeling:
//   P4 MOSI (master out) → GPIO22 → wire → Pico GPIO32 (SPI0_RX = slave in)
//   P4 MISO (master in)  ← GPIO23 ← wire ← Pico GPIO35 (SPI0_TX = slave out)
#define PB3_HOST    SPI3_HOST
#define PB3_MOSI    GPIO_NUM_22  // P4 master TX → Pico GPIO32 (slave RX)
#define PB3_MISO    GPIO_NUM_23  // P4 master RX ← Pico GPIO35 (slave TX)
#define PB3_SCLK    GPIO_NUM_21
#define PB3_CS      GPIO_NUM_20
#define PB3_CLOCK_HZ 10000000    // 10 MHz (conservative for picoboot3)

// Picoboot3 command codes
#define CMD_READY_BUSY  0x01
#define CMD_VERSION     0x02
#define CMD_READ        0x10
#define CMD_PROGRAM     0x20
#define CMD_ERASE       0x30
#define CMD_GO_TO_APP   0x40
#define CMD_FLASH_SIZE  0x50
#define CMD_ACTIVATE    0xA5

static spi_device_handle_t spi_dev = nullptr;

// DMA-capable buffers for SPI transactions
static uint8_t *tx_buf = nullptr;
static uint8_t *rx_buf = nullptr;
static const size_t BUF_SIZE = 4224;  // 7-byte header + 4096 data + margin

/// Send command bytes. Picoboot3 uses separate transactions for command and response.
static esp_err_t send_cmd(const uint8_t *cmd, size_t cmd_len) {
    memcpy(tx_buf, cmd, cmd_len);
    spi_transaction_t t = {};
    t.length = cmd_len * 8;
    t.tx_buffer = tx_buf;
    t.rx_buffer = nullptr;
    return spi_device_polling_transmit(spi_dev, &t);
}

/// Receive response bytes in a separate SPI transaction (CS toggles between cmd and response).
static esp_err_t recv_resp(uint8_t *resp, size_t resp_len) {
    memset(tx_buf, 0xFF, resp_len);  // clock out dummy bytes
    spi_transaction_t t = {};
    t.length = resp_len * 8;
    t.tx_buffer = tx_buf;
    t.rx_buffer = rx_buf;
    esp_err_t ret = spi_device_polling_transmit(spi_dev, &t);
    if (ret == ESP_OK) {
        memcpy(resp, rx_buf, resp_len);
    }
    return ret;
}

namespace CTAG::DRIVERS {

esp_err_t Picoboot3Master::Init() {
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = PB3_MOSI;
    buscfg.miso_io_num = PB3_MISO;
    buscfg.sclk_io_num = PB3_SCLK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = BUF_SIZE;

    esp_err_t ret = spi_bus_initialize(PB3_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_initialize failed: %s", esp_err_to_name(ret));
        return ret;
    }

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = PB3_CLOCK_HZ;
    devcfg.mode = 3;  // CPOL=1, CPHA=1 — matches picoboot3
    devcfg.spics_io_num = PB3_CS;
    devcfg.queue_size = 1;
    devcfg.cs_ena_pretrans = 1;
    devcfg.cs_ena_posttrans = 1;

    ret = spi_bus_add_device(PB3_HOST, &devcfg, &spi_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_add_device failed: %s", esp_err_to_name(ret));
        spi_bus_free(PB3_HOST);
        return ret;
    }

    // Allocate DMA-capable buffers
    tx_buf = (uint8_t *)heap_caps_calloc(1, BUF_SIZE, MALLOC_CAP_DMA);
    rx_buf = (uint8_t *)heap_caps_calloc(1, BUF_SIZE, MALLOC_CAP_DMA);
    if (!tx_buf || !rx_buf) {
        ESP_LOGE(TAG, "DMA buffer allocation failed");
        Deinit();
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "SPI master initialized on SPI3_HOST, %d Hz, Mode 3", PB3_CLOCK_HZ);
    return ESP_OK;
}

void Picoboot3Master::Deinit() {
    if (spi_dev) {
        spi_bus_remove_device(spi_dev);
        spi_dev = nullptr;
    }
    spi_bus_free(PB3_HOST);
    if (tx_buf) { free(tx_buf); tx_buf = nullptr; }
    if (rx_buf) { free(rx_buf); rx_buf = nullptr; }
    ESP_LOGI(TAG, "SPI master deinitialized");
}

esp_err_t Picoboot3Master::Activate(uint8_t *out_resp) {
    uint8_t cmd = CMD_ACTIVATE;
    esp_err_t ret = send_cmd(&cmd, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Activate: send_cmd failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Delay for picoboot3 DMA handler to process and load TX FIFO
    esp_rom_delay_us(500);

    uint8_t resp[4] = {0};
    ret = recv_resp(resp, 4);
    if (out_resp) memcpy(out_resp, resp, 4);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Activate: recv_resp failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Activate: response: %02x %02x %02x %02x",
             resp[0], resp[1], resp[2], resp[3]);

    // Expect "pbt3" = {0x70, 0x62, 0x74, 0x33}
    if (resp[0] != 0x70 || resp[1] != 0x62 || resp[2] != 0x74 || resp[3] != 0x33) {
        ESP_LOGE(TAG, "Activate: unexpected response");
        return ESP_ERR_INVALID_RESPONSE;
    }

    ESP_LOGI(TAG, "Activated — picoboot3 handshake OK");
    return ESP_OK;
}

esp_err_t Picoboot3Master::GetVersion(uint8_t &major, uint8_t &minor, uint8_t &patch) {
    uint8_t cmd = CMD_VERSION;
    esp_err_t ret = send_cmd(&cmd, 1);
    if (ret != ESP_OK) return ret;

    esp_rom_delay_us(100);

    uint8_t resp[3];
    ret = recv_resp(resp, 3);
    if (ret != ESP_OK) return ret;

    major = resp[0];
    minor = resp[1];
    patch = resp[2];
    ESP_LOGI(TAG, "Version: %d.%d.%d", major, minor, patch);
    return ESP_OK;
}

esp_err_t Picoboot3Master::GetFlashSize(uint32_t &size) {
    uint8_t cmd = CMD_FLASH_SIZE;
    esp_err_t ret = send_cmd(&cmd, 1);
    if (ret != ESP_OK) return ret;

    esp_rom_delay_us(100);

    uint8_t resp[4];
    ret = recv_resp(resp, 4);
    if (ret != ESP_OK) return ret;

    // Little-endian 32-bit
    size = resp[0] | (resp[1] << 8) | (resp[2] << 16) | (resp[3] << 24);
    ESP_LOGI(TAG, "Flash size: %lu bytes (%lu KB)", size, size / 1024);
    return ESP_OK;
}

esp_err_t Picoboot3Master::IsReady(bool &ready) {
    uint8_t cmd = CMD_READY_BUSY;
    esp_err_t ret = send_cmd(&cmd, 1);
    if (ret != ESP_OK) return ret;

    esp_rom_delay_us(50);

    uint8_t resp;
    ret = recv_resp(&resp, 1);
    if (ret != ESP_OK) return ret;

    ready = (resp == 0x01);
    return ESP_OK;
}

esp_err_t Picoboot3Master::WaitReady(uint32_t timeout_ms) {
    TickType_t start = xTaskGetTickCount();
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);

    while ((xTaskGetTickCount() - start) < timeout_ticks) {
        bool ready = false;
        esp_err_t ret = IsReady(ready);
        if (ret != ESP_OK) return ret;
        if (ready) return ESP_OK;
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ESP_LOGE(TAG, "WaitReady timeout (%lu ms)", timeout_ms);
    return ESP_ERR_TIMEOUT;
}

esp_err_t Picoboot3Master::EraseSector(uint16_t sector) {
    // Command: 0x30 + 2-byte sector (little-endian)
    uint8_t cmd[3];
    cmd[0] = CMD_ERASE;
    cmd[1] = sector & 0xFF;
    cmd[2] = (sector >> 8) & 0xFF;

    esp_err_t ret = send_cmd(cmd, 3);
    if (ret != ESP_OK) return ret;

    ESP_LOGD(TAG, "Erase sector %d (addr 0x%05lX)", sector, (uint32_t)sector * 4096);
    return ESP_OK;
}

esp_err_t Picoboot3Master::ProgramPage(uint32_t addr, const uint8_t *data, uint16_t len) {
    if (len == 0 || len > 4096 || (addr % 256) != 0 || (len % 256) != 0) {
        ESP_LOGE(TAG, "ProgramPage: invalid addr=0x%08lX len=%d", addr, len);
        return ESP_ERR_INVALID_ARG;
    }

    // Command: 0x20 + 4-byte addr (LE) + 2-byte len (LE) + data
    uint8_t header[7];
    header[0] = CMD_PROGRAM;
    header[1] = addr & 0xFF;
    header[2] = (addr >> 8) & 0xFF;
    header[3] = (addr >> 16) & 0xFF;
    header[4] = (addr >> 24) & 0xFF;
    header[5] = len & 0xFF;
    header[6] = (len >> 8) & 0xFF;

    // Build full command in tx_buf: header + data
    memcpy(tx_buf, header, 7);
    memcpy(tx_buf + 7, data, len);

    spi_transaction_t t = {};
    t.length = (7 + len) * 8;
    t.tx_buffer = tx_buf;
    t.rx_buffer = nullptr;
    esp_err_t ret = spi_device_polling_transmit(spi_dev, &t);
    if (ret != ESP_OK) return ret;

    ESP_LOGD(TAG, "Program %d bytes at 0x%08lX", len, addr);
    return ESP_OK;
}

esp_err_t Picoboot3Master::ReadFlash(uint32_t addr, uint8_t *data, uint16_t len) {
    if (len == 0 || len > 4096) {
        return ESP_ERR_INVALID_ARG;
    }

    // Command: 0x10 + 4-byte addr (LE) + 2-byte len (LE)
    uint8_t cmd[7];
    cmd[0] = CMD_READ;
    cmd[1] = addr & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = (addr >> 16) & 0xFF;
    cmd[4] = (addr >> 24) & 0xFF;
    cmd[5] = len & 0xFF;
    cmd[6] = (len >> 8) & 0xFF;

    esp_err_t ret = send_cmd(cmd, 7);
    if (ret != ESP_OK) return ret;

    esp_rom_delay_us(100);

    ret = recv_resp(data, len);
    return ret;
}

esp_err_t Picoboot3Master::GoToApp() {
    uint8_t cmd = CMD_GO_TO_APP;
    esp_err_t ret = send_cmd(&cmd, 1);
    ESP_LOGI(TAG, "GO_TO_APP sent — Pico will jump to application");
    return ret;
}

}  // namespace CTAG::DRIVERS

#endif // CONFIG_TBD_USE_RP2350
