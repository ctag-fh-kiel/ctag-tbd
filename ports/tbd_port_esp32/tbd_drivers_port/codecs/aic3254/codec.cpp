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
#include <tbd/drivers/common/codec.hpp>

#include "aic3254.hpp"

#include "freertos/FreeRTOS.h"
#include <driver/i2s_std.h>
#include "esp_log.h"
#include "esp_attr.h"

#if !TBD_AUDIO_AIC3254
    #error "no aic3254 sound chip configured"
#endif

#define MAX(x, y) ((x)>(y)) ? (x) : (y)
#define MIN(x, y) ((x)<(y)) ? (x) : (y)

namespace {
    aic3254 codec;
}

namespace tbd::drivers {

static i2s_chan_handle_t tx_handle = NULL;
static i2s_chan_handle_t rx_handle = NULL;



void Codec::init() {
    ESP_LOGI("BBA Codec", "Starting i2s setup...");
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(i2s_port_t(0), I2S_ROLE_MASTER);
    chan_cfg.auto_clear = false;
    // TODO is 4 dma descriptors enough? -> any effect on latency, started with 4 but sometime there was noise
    // TODO can be estimated from this https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2s.html
    chan_cfg.dma_desc_num = 4;
    chan_cfg.dma_frame_num = 32;

    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle));

    // Note that GPIO33 ~ GPIO37 and GPIO47 ~ GPIO48 can be powered either by VDD_SPI or VDD3P3_CPU.
    // compare pg. 469 https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf
    // https://github.com/micropython/micropython/issues/7928

    i2s_std_config_t std_cfg = {
            .clk_cfg = {
                    .sample_rate_hz = 44100,
                    .clk_src = I2S_CLK_SRC_DEFAULT,
                    .ext_clk_freq_hz = 0,
                    .mclk_multiple = I2S_MCLK_MULTIPLE_256,
            },
            .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
            .gpio_cfg = {
                    .mclk = GPIO_NUM_39,
                    .bclk = GPIO_NUM_45,
                    .ws   = GPIO_NUM_1,
                    .dout = GPIO_NUM_2,
                    .din  = GPIO_NUM_21,
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

    if(codec.identify()){
        ESP_LOGI("BBA Codec", "Found codec...");
    }else{
        ESP_LOGE("BBA Codec", "Error: Could not find codec!");
    }
    codec.init();
}

void Codec::deinit() {
}

void Codec::HighPassEnable() {
}

void Codec::HighPassDisable() {
}

void Codec::RecalibDCOffset() {
}

void Codec::SetOutputLevels(const uint32_t left, const uint32_t right) {
    codec.setOutputVolume(static_cast<uint8_t>(left), static_cast<uint8_t>(right));
}

void IRAM_ATTR Codec::ReadBuffer(float *buf, uint32_t sz) {
    int32_t tmp[sz * 2];
    int32_t *ptrTmp = tmp;
    size_t nb;
    const float div = 1.0f / 2147483648.0f;
    // 32 bit word config stereo
    i2s_channel_read(rx_handle, tmp, sz*2*4, &nb, portMAX_DELAY);
    while (sz > 0) {
        *buf++ = div * (float) *ptrTmp++;
        *buf++ = div * (float) *ptrTmp++;
        sz--;
    }
}

void IRAM_ATTR Codec::WriteBuffer(float *buf, uint32_t sz) {
    int32_t tmp[sz * 2];
    int32_t tmp2;
    size_t nb;
    const float mult = 2147483647.f;
    // 32 bit word config
    for (int i = 0; i < sz; i++) {
        tmp2 = (int32_t) (mult * buf[i * 2]);
        tmp2 = MAX(tmp2, -2147483647);
        tmp2 = MIN(tmp2, 2147483647);
        tmp[i * 2] = tmp2;
        tmp2 = (int32_t) (mult * buf[i * 2 + 1]);
        tmp2 = MAX(tmp2, -2147483648);
        tmp2 = MIN(tmp2, 2147483647);
        tmp[i * 2 + 1] = tmp2;
    }
    i2s_channel_write(tx_handle, tmp, sz*2*4, &nb, portMAX_DELAY);
}

}
