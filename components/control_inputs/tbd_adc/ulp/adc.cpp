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
#include <tbd/control_inputs/impl/adc.hpp>

#include "freertos/FreeRTOS.h"

#include <cstring>
#include <driver/dac_oneshot.h>

// otherwise linker error
extern "C" {

#include "soc/rtc_cntl_reg.h"

#include "soc/rtc_io_reg.h"
#include "driver/rtc_io.h"
#include "driver/rtc_cntl.h"
#include "driver/adc.h"
#include "soc/soc.h"
#include "soc/rtc.h"

// #include "ulp_fsm_common.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "freertos/queue.h"

// #include "ulp.h"
// #include "ulp.h"
// #include "ulp_tbd_drivers_port.h"
#include "esp32/ulp.h"
#include "ulp_main.h"
#include <tbd/logging.hpp>

// #include "esp32/ulp_main.h"

}

#if TBD_CV_ADC
#include "esp_adc/adc_oneshot.h"
#endif

#if !(TBD_CV_ADC || TBD_CV_MCP3208)
    #error "ULP onboard ADC is not available"
#endif

// FIXME: having the directory name derived component name here is rather brittle
extern const uint8_t ulp_drivers_bin_start[] asm("_binary_ulp_tbd_drivers_port_bin_start");
extern const uint8_t ulp_drivers_bin_end[]   asm("_binary_ulp_tbd_drivers_port_bin_end");


namespace {

QueueHandle_t adcDataQueue = NULL;

#if TBD_CV_ADC
dac_oneshot_handle_t chan0_handle;
dac_oneshot_handle_t chan1_handle;

void init_analog_sub_system() {
    /* config DAC for offset voltage */
    /* default for 0->5V CV in */
    /*
    dac_output_enable(DAC_CHAN_0);
    dac_output_voltage(DAC_CHAN_0, 58);
    dac_output_enable(DAC_CHAN_1);
    dac_output_voltage(DAC_CHAN_1, 58);
     */
    /* NEW API: DAC oneshot init */
    dac_oneshot_config_t chan0_cfg = {
            .chan_id = DAC_CHAN_0,
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&chan0_cfg, &chan0_handle));
    ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan0_handle, 58));
    dac_oneshot_config_t chan1_cfg = {
            .chan_id = DAC_CHAN_1,
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&chan1_cfg, &chan1_handle));
    ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan1_handle, 58));


    /* config ADC channels */

    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_0); // GPIO32
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_0); // GPIO33
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_0); // GPIO34
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_0); // GPIO35

    /* enable ULP on adc 1 */

    adc1_ulp_enable();
}
#endif

void init_ulp_program() {
    /* load ULP program */
    ESP_ERROR_CHECK(ulp_load_binary(0, ulp_drivers_bin_start,
                                    (ulp_drivers_bin_end - ulp_drivers_bin_start) / sizeof(uint32_t)));

#if TBD_CV_MCP3208
    const gpio_num_t GPIO_CS0 = TBD_MCP3208_PIN_CS;
    const gpio_num_t GPIO_MOSI = TBD_MCP3208_PIN_MOSI;
    const gpio_num_t GPIO_SCLK = TBD_MCP3208_PIN_SCLK;
    const gpio_num_t GPIO_MISO = TBD_MCP3208_PIN_MISO;

    rtc_gpio_init(GPIO_CS0);
    rtc_gpio_set_direction(GPIO_CS0, RTC_GPIO_MODE_OUTPUT_ONLY);
    rtc_gpio_set_level(GPIO_CS0, 0);

    rtc_gpio_init(GPIO_MOSI);
    rtc_gpio_set_direction(GPIO_MOSI, RTC_GPIO_MODE_OUTPUT_ONLY);
    rtc_gpio_set_level(GPIO_MOSI, 0);

    rtc_gpio_init(GPIO_SCLK);
    rtc_gpio_set_direction(GPIO_SCLK, RTC_GPIO_MODE_OUTPUT_ONLY);
    rtc_gpio_set_level(GPIO_SCLK, 0);

    rtc_gpio_init(GPIO_MISO);
    rtc_gpio_set_direction(GPIO_MISO, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pullup_en(GPIO_MISO);
#endif
}

uint16_t cv_data[N_CVS];

}

namespace tbd::drivers {

void IRAM_ATTR ulp_isr(void *arg) {
    static uint16_t data[N_CVS];

    // disable ULP timer, should get 10 us to settle
    CLEAR_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN);

    #if TBD_CV_MCP3208
        // header generated from the mcp3208.S asm can not determine ulp_adc_data is an array
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Warray-bounds"
            data[0] = (uint16_t) *((&ulp_adc_data) + 0);
            data[1] = (uint16_t) *((&ulp_adc_data) + 1);
            data[2] = (uint16_t) *((&ulp_adc_data) + 2);
            data[3] = (uint16_t) *((&ulp_adc_data) + 3);
            data[4] = (uint16_t) *((&ulp_adc_data) + 4);
            data[5] = (uint16_t) *((&ulp_adc_data) + 6);
            data[6] = (uint16_t) *((&ulp_adc_data) + 5);
            data[7] = (uint16_t) *((&ulp_adc_data) + 7);
        #pragma GCC diagnostic pop
    #else
        for(int i=0; i < N_CVS; i++){
            data[i] = (uint16_t) *((&ulp_adc_data) + i);
        }
    #endif

    xQueueSendToFrontFromISR(adcDataQueue, data, NULL);
}

uint8_t* ADC::update() {
    xQueueReceive(adcDataQueue, cv_data, portMAX_DELAY);
    // restart ULP
    SET_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN);
    return reinterpret_cast<uint8_t*>(cv_data);
}

uint16_t ADC::GetChannelVal(int ch) {
    return cv_data[ch];
}

void ADC::init() {
    esp_err_t err;

    // create data queue
    adcDataQueue = xQueueCreate(1, sizeof(uint16_t) * N_CVS);

    // set up ULP program
    err = rtc_isr_register((intr_handler_t) &ulp_isr, (void *) 0, (uint32_t) RTC_CNTL_SAR_INT_ST_M, 0);
    ESP_ERROR_CHECK(err);
    REG_SET_BIT(RTC_CNTL_INT_ENA_REG, RTC_CNTL_ULP_CP_INT_ENA_M);
    // set RTC FAST CLOCK to XTAL/4 = 10MHz at 40Mhz XTAL speed
    rtc_clk_fast_freq_set(RTC_FAST_FREQ_XTALD4);
    init_ulp_program();

#if TBD_CV_ADC
    init_analog_sub_system();
#endif

    ESP_ERROR_CHECK(ulp_run((&ulp_entry - RTC_SLOW_MEM) / sizeof(uint32_t)));

#if TBD_CV_ADC
    SetCVINUnipolar(0);
    SetCVINUnipolar(1);
#endif
}

void ADC::SetCVINUnipolar(int ch) {
#if TBD_CV_ADC
    ESP_LOGI("ADC", "SetCVINUnipolar(%d)", ch);
    if (ch == 0) {
        //dac_output_voltage(DAC_CHAN_0, 55); // offset
        ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan0_handle, 55));
        adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_0); // GPIO32
    } else if (ch == 1) {
        //dac_output_voltage(DAC_CHAN_1, 55);
        ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan1_handle, 55));
        adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_0); // GPIO33
    }
#endif
}

void ADC::SetCVINBipolar(int ch) {
#if TBD_CV_ADC
    ESP_LOGI("ADC", "SetCVINBipolar(%d)", ch);
    if (ch == 0) {
        //dac_output_voltage(DAC_CHAN_0, 57); // offset
        ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan0_handle, 57));
        adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_6); // GPIO32
    } else if (ch == 1) {
        //dac_output_voltage(DAC_CHAN_1, 57);
        //dac_output_voltage(DAC_CHAN_1, 57);
        ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan1_handle, 57));
        adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_6); // GPIO33
    }
#endif
}

void ADC::GetChannelVals(uint16_t *d) {
    memcpy(d, cv_data, sizeof(uint16_t) * N_CVS);
}

}
