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
#include <tbd/drivers/common/adc.hpp>

#include "freertos/FreeRTOS.h"

// otherwise linker error
extern "C" {

// #include "soc/rtc_cntl_reg.h"
// #include "soc/rtc_io_reg.h"
// #include "soc/rtc.h"
// #include "driver/rtc_io.h"
// #include "driver/rtc_cntl.h"
// #include "driver/adc.h"
#include "driver/dac_oneshot.h"
// #include "soc/soc.h"
// #include "soc/rtc.h"
// #include "soc/rtc_cntl_reg.h"
// #include "soc/sens_reg.h"
// #include "esp32/ulp.h"
// #include "ulp_drivers.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/portmacro.h"
// #include "freertos/queue.h"
#include <tbd/logging.hpp>

}


namespace tbd::drivers {

extern const uint8_t ulp_drivers_bin_start[] asm("_binary_ulp_drivers_bin_start");
extern const uint8_t ulp_drivers_bin_end[]   asm("_binary_ulp_drivers_bin_end");

static QueueHandle_t adcDataQueue = NULL;

uint16_t ADC::data[N_CVS];

static dac_oneshot_handle_t chan0_handle;
static dac_oneshot_handle_t chan1_handle;

static void IRAM_ATTR ulp_isr(void *arg) {
    static uint16_t data[N_CVS];
    //static uint32_t s = 0;
    //gpio_set_level(23, (s++)&0x01);
    /* hardware debugging
    gpio_set_level(SPI_TFT_MISO_PIN, 1);
    */

    // disable ULP timer, should get 10 us to settle
    CLEAR_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN);

#ifdef CONFIG_TBD_PLATFORM_STR
    data[0] = (uint16_t) *((&ulp_adc_data) + 0);
    data[1] = (uint16_t) *((&ulp_adc_data) + 1);
    data[2] = (uint16_t) *((&ulp_adc_data) + 2);
    data[3] = (uint16_t) *((&ulp_adc_data) + 3);
    data[4] = (uint16_t) *((&ulp_adc_data) + 4);
    data[5] = (uint16_t) *((&ulp_adc_data) + 6);
    data[6] = (uint16_t) *((&ulp_adc_data) + 5);
    data[7] = (uint16_t) *((&ulp_adc_data) + 7);
#else
    for(int i=0; i < N_CVS; i++){
        data[i] = (uint16_t) *((&ulp_adc_data) + i);
    }
#endif

    xQueueSendToFrontFromISR(adcDataQueue, data, NULL);

    /* hardware debugging 
    gpio_set_level(23, 0);
    */
    //gpio_set_level(23, 0);
}

void IRAM_ATTR ADC::Update() {
    xQueueReceive(adcDataQueue, data, portMAX_DELAY);
    // restart ULP
    SET_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN);
}

uint16_t ADC::GetChannelVal(int ch) {
    return data[ch];
}

void ADC::InitADCSystem() {
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

#if defined(CONFIG_TBD_PLATFORM_V2) || defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_AEM)
    init_analog_sub_system();
#endif

    ESP_ERROR_CHECK(ulp_run((&ulp_entry - RTC_SLOW_MEM) / sizeof(uint32_t)));

#if defined(CONFIG_TBD_PLATFORM_V2) || defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_AEM)
    SetCVINUnipolar(0);
    SetCVINUnipolar(1);
#endif
}

void ADC::SetCVINUnipolar(int ch) {
#if defined(CONFIG_TBD_PLATFORM_V2) || defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_AEM)
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
#if defined(CONFIG_TBD_PLATFORM_V2) || defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_AEM)
    ESP_LOGI("ADC", "SetCVINBipolar(%d)", ch);
    if (ch == 0) {
        //dac_output_voltage(DAC_CHAN_0, 57); // offset
        ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan0_handle, 57));
        adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_6); // GPIO32
    } else if (ch == 1) {
        //dac_output_voltage(DAC_CHAN_1, 57);
        ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan1_handle, 57));
        adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_6); // GPIO33
    }
#endif
}

void ADC::init_ulp_program() {
    /* load ULP program */
    ESP_ERROR_CHECK(ulp_load_binary(0, ulp_drivers_bin_start,
                                    (ulp_drivers_bin_end - ulp_drivers_bin_start) / sizeof(uint32_t)));

#ifdef CONFIG_TBD_PLATFORM_STR
    const gpio_num_t GPIO_CS0 = GPIO_NUM_32;
    const gpio_num_t GPIO_MOSI = GPIO_NUM_13;
    const gpio_num_t GPIO_SCLK = GPIO_NUM_12;
    const gpio_num_t GPIO_MISO = GPIO_NUM_0;

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

void ADC::GetChannelVals(uint16_t *d) {
    memcpy(d, data, sizeof(uint16_t) * N_CVS);
}

void ADC::init_analog_sub_system() {
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

}
