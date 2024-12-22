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

#include "codec_bba.hpp"

#include <driver/i2s_std.h>
#include "esp_log.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"

#define I2S_NUM         (0)
#define I2S_MCK_IO      (GPIO_NUM_13)
#define I2S_BCK_IO      (GPIO_NUM_12)
#define I2S_WS_IO       (GPIO_NUM_10)
#define I2S_DO_IO       (GPIO_NUM_9)
#define I2S_DI_IO       (GPIO_NUM_11)

#define EXAMPLE_SAMPLE_RATE     (44100)
#define EXAMPLE_MCLK_MULTIPLE   (256) // If not using 24-bit data width, 256 should be enough
#define EXAMPLE_MCLK_FREQ_HZ    (EXAMPLE_SAMPLE_RATE * EXAMPLE_MCLK_MULTIPLE)
#define I2C_NUM         I2C_NUM_0
#define I2C_SCL_IO      (GPIO_NUM_8)
#define I2C_SDA_IO      (GPIO_NUM_7)
#define GPIO_OUTPUT_PA  (GPIO_NUM_53)

using namespace CTAG::DRIVERS;

static i2s_chan_handle_t tx_handle = NULL;
static i2s_chan_handle_t rx_handle = NULL;

// init es_handle
static es8311_handle_t es_handle = nullptr;

void Codec::InitCodec() {

    ESP_LOGI("BBA Codec", "Init ES8311 PA GPIO");
    gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << GPIO_OUTPUT_PA), // 选择GPIO48
            .mode = GPIO_MODE_OUTPUT,                  // 配置为输出模式
            .pull_up_en = GPIO_PULLUP_DISABLE,         // 禁用上拉
            .pull_down_en = GPIO_PULLDOWN_DISABLE,     // 禁用下拉
            .intr_type = GPIO_INTR_DISABLE             // 禁用中断
    };
    gpio_config(&io_conf);

    // 设置GPIO48为高电平
    gpio_set_level(GPIO_OUTPUT_PA, 1);

    ESP_LOGI("BBA Codec", "Starting i2s setup...");
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(i2s_port_t(0), I2S_ROLE_MASTER);
    chan_cfg.auto_clear = false;
    // TODO is 4 dma descriptors enough? -> any effect on latency, started with 4 but sometime there was noise
    // TODO can be estimated from this https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2s.html
    chan_cfg.dma_desc_num = 4;
    chan_cfg.dma_frame_num = 32;

    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle));

    i2s_std_config_t std_cfg = {
            .clk_cfg = {
                    .sample_rate_hz = 44100,
                    .clk_src = I2S_CLK_SRC_DEFAULT,
                    .ext_clk_freq_hz = 0,
                    .mclk_multiple = I2S_MCLK_MULTIPLE_256,
            },
            .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
            .gpio_cfg = {
                    .mclk = I2S_MCK_IO,
                    .bclk = I2S_BCK_IO,
                    .ws = I2S_WS_IO,
                    .dout = I2S_DO_IO,
                    .din = I2S_DI_IO,
                    .invert_flags = {
                            .mclk_inv = false,
                            .bclk_inv = false,
                            .ws_inv = false,
                    },
            },
    };
    std_cfg.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_256;

    /* Initialize the channels */
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_handle, &std_cfg));

    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));
    ESP_LOGI("BBA Codec", "Configuring codec with i2c!");

    const i2c_config_t es_i2c_cfg = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = I2C_SDA_IO,
            .scl_io_num = I2C_SCL_IO,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master = {
                    .clk_speed = 100000,
            },
            .clk_flags = 0,
    };
    i2c_param_config(I2C_NUM, &es_i2c_cfg);
    i2c_driver_install(I2C_NUM, I2C_MODE_MASTER,  0, 0, 0);

    /* Initialize es8311 codec */
    es_handle = es8311_create(I2C_NUM, ES8311_ADDRRES_0);

    const es8311_clock_config_t es_clk = {
            .mclk_inverted = false,
            .sclk_inverted = false,
            .mclk_from_mclk_pin = true,
            .mclk_frequency = EXAMPLE_MCLK_FREQ_HZ,
            .sample_frequency = EXAMPLE_SAMPLE_RATE
    };
    es8311_init(es_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16);
    es8311_sample_frequency_config(es_handle, EXAMPLE_SAMPLE_RATE * EXAMPLE_MCLK_MULTIPLE, EXAMPLE_SAMPLE_RATE);
    int v;
    es8311_voice_volume_set(es_handle, 70, &v);
    es8311_microphone_config(es_handle, false);
}

void Codec::HighPassEnable() {
}

void Codec::HighPassDisable() {
}

void Codec::RecalibDCOffset() {
}

void Codec::SetOutputLevels(const uint32_t left, const uint32_t right) {
    int v;
    es8311_voice_volume_set(es_handle, (int)left, &v);
}

void IRAM_ATTR Codec::ReadBuffer(float *buf, uint32_t sz) {
    int16_t tmp[sz * 2];
    int16_t *ptrTmp = tmp;
    size_t nb;
    const float div = 3.0518509476E-5f;
    // 16 bit word config stereo
    i2s_channel_read(rx_handle, tmp, sz*2*2, &nb, portMAX_DELAY);
    while (sz > 0) {
        *buf++ = div * (float) *ptrTmp++;
        *buf++ = div * (float) *ptrTmp++;
        sz--;
    }
}

void IRAM_ATTR Codec::WriteBuffer(float *buf, uint32_t sz) {
    int16_t tmp[sz * 2];
    int16_t tmp2;
    size_t nb;
    const float mult = 32767.f;
    // 16 bit word config
    for (int i = 0; i < sz; i++) {
        tmp2 = (int32_t) (mult * buf[i * 2]);
        tmp2 = MAX(tmp2, -32767);
        tmp2 = MIN(tmp2, 32767);
        tmp[i * 2] = tmp2;
        tmp2 = (int32_t) (mult * buf[i * 2 + 1]);
        tmp2 = MAX(tmp2, -32767);
        tmp2 = MIN(tmp2, 32767);
        tmp[i * 2 + 1] = tmp2;
    }
    i2s_channel_write(tx_handle, tmp, sz*2*2, &nb, portMAX_DELAY);
}