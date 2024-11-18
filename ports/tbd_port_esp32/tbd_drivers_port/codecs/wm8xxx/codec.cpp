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
#include <tbd/drivers/codec.hpp>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/i2s.h"
#include "esp_log.h"

#include "soc/io_mux_reg.h"
#include <string.h>
#include <cmath>

#include "wm8xxx.hpp"

void Codec::InitCodec() {
    initSPI();
#if defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_STR)
    setupSPIWM8731();
    setupI2SWM8731();
    vTaskDelay(5000 / portTICK_PERIOD_MS); // wait until system is settled a bit
    // CodecManager has noise issues with highpass on
    // https://hackaday.io/project/7936-cortex-guitar-board/log/44553-this-adc-makes-weird-noises
    HighPassEnable();
    vTaskDelay(1000 / portTICK_PERIOD_MS); // wait until system is settled a bit
    HighPassDisable();
#elif defined(CONFIG_TBD_PLATFORM_V2) || defined(CONFIG_TBD_PLATFORM_MK2)
    setupSPIWM8978();
    setupI2SWM8978();
#elif CONFIG_TBD_PLATFORM_AEM
    WM8974_Init();
    setupI2SWM8978();
#endif

// release SPI on devices with no volume control
#ifdef TBD_VOLUME_CONTROL
    freeSPI();
#endif
    isReady = true;
}


void Codec::HighPassEnable() {
#if defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_STR)
    unsigned char cmd;
    trans.flags = 0;
    trans.length = 8;
    trans.rxlength = 0;
    trans.tx_buffer = &cmd;
    trans.rx_buffer = NULL;
    trans.addr = 0x05 << 1;
    cmd = 0x00; // disable HPF and store last DC value
    ESP_ERROR_CHECK(spi_device_transmit(codec_h, &trans));
#endif
}


void Codec::HighPassDisable() {
#if defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_STR)
    unsigned char cmd;
    trans.flags = 0;
    trans.length = 8;
    trans.rxlength = 0;
    trans.tx_buffer = &cmd;
    trans.rx_buffer = NULL;
    trans.addr = 0x05 << 1;
    cmd = 0x11; // disable HPF and store last DC value
    ESP_ERROR_CHECK(spi_device_transmit(codec_h, &trans));
#endif
}


void Codec::RecalibDCOffset() {
#if defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_STR)
    if(!isReady) return;
    HighPassEnable();
    vTaskDelay(50 / portTICK_PERIOD_MS); // wait until system is settled a bit
    HighPassDisable();
#endif
}


void Codec::SetOutputLevels(const uint32_t left, const uint32_t right) {
#if defined(CONFIG_TBD_PLATFORM_V2) || defined(CONFIG_TBD_PLATFORM_MK2)
    if(!isReady){
        ESP_LOGD("CODEC", "Codec not initialized");
    }
    ESP_LOGD("CODEC", "Setting levels to %li, %li", left, right);
    WM8978_HPvol_Set(static_cast<u8>(left), static_cast<u8>(right));
#endif
}


void IRAM_ATTR Codec::WriteBuffer(float *buf, uint32_t sz) {
#if defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_STR)
    int32_t tmp[sz * 2];
    int32_t tmp2;
    size_t nb;
    const float mult = 8388608.f;
    // 32 bit word config
    for (int i = 0; i < sz; i++) {
        tmp2 = (int32_t) (mult * buf[i * 2]);
        tmp2 = MAX(tmp2, -8388608);
        tmp2 = MIN(tmp2, 8388607);
        tmp[i * 2] = tmp2 << 8;
        tmp2 = (int32_t) (mult * buf[i * 2 + 1]);
        tmp2 = MAX(tmp2, -8388608);
        tmp2 = MIN(tmp2, 8388607);
        tmp[i * 2 + 1] = tmp2 << 8;
    }
    i2s_write(I2S_NUM_0, tmp, sz * 4 * 2, &nb, portMAX_DELAY);
#elif defined(CONFIG_TBD_PLATFORM_V2) || defined(CONFIG_TBD_PLATFORM_MK2)
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
    i2s_write(I2S_NUM_0, tmp, sz * 2 * 2, &nb, portMAX_DELAY);
#elif CONFIG_TBD_PLATFORM_AEM
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
        //tmp[i * 2 + 1] = 0; // not used in mono codec
    }
    i2s_write(I2S_NUM_0, tmp, sz * 2 * 2, &nb, portMAX_DELAY);
#endif
}


void IRAM_ATTR Codec::ReadBuffer(float *buf, uint32_t sz) {
#if defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_STR)
    int32_t tmp[sz * 2];
    int32_t *ptrTmp = tmp;
    size_t nb;
    const float div = 0.0000000004656612873077392578125f;
    // 32 bit word config
    i2s_read(I2S_NUM_0, tmp, sz * 4 * 2, &nb, portMAX_DELAY);
    while(sz > 0){
        *buf++ = div * (float) *ptrTmp++;
        *buf++ = div * (float) *ptrTmp++;
        sz--;
    }
#elif defined(CONFIG_TBD_PLATFORM_V2) || defined(CONFIG_TBD_PLATFORM_MK2)
    int16_t tmp[sz * 2];
    int16_t *ptrTmp = tmp;
    size_t nb;
    const float div = 3.0518509476E-5f;
    // 16 bit word config stereo
    i2s_read(I2S_NUM_0, tmp, sz * 2 * 2, &nb, portMAX_DELAY);
    while (sz > 0) {
        *buf++ = div * (float) *ptrTmp++;
        *buf++ = div * (float) *ptrTmp++;
        sz--;
    }
#elif CONFIG_TBD_PLATFORM_AEM
    int16_t tmp[sz * 2];
    int16_t *ptrTmp = tmp;
    size_t nb;
    const float div = 3.0518509476E-5f;
    i2s_read(I2S_NUM_0, tmp, sz * 2 * 2, &nb, portMAX_DELAY);
    while (sz > 0) {
        float s = div * (float) *ptrTmp++;
        ptrTmp++;
        *buf++ = s;
        *buf++ = s;
        sz--;
    }
#endif
}

}