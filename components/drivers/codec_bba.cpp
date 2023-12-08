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

#define MAX(x, y) ((x)>(y)) ? (x) : (y)
#define MIN(x, y) ((x)<(y)) ? (x) : (y)

using namespace CTAG::DRIVERS;

static i2s_chan_handle_t tx_handle = NULL;
static i2s_chan_handle_t rx_handle = NULL;

es8388 Codec::codec;

void Codec::InitCodec() {
    ESP_LOGI("ES8388", "Starting i2s setup...");
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
            .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(44100),
            .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
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
    std_cfg.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_384;
    //std_cfg.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_512;
    //std_cfg.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_256; // -> bad does not work!


    /* Initialize the channels */
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_handle, &std_cfg));

    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));



    if(codec.identify()){
        ESP_LOGI("ES8388", "Found ES8388...");
    }else{
        ESP_LOGE("ES8388", "Could not find ES8388...");
    }
    codec.init();
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