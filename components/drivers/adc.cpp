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


#include "adc.hpp"
#include <string.h>

// otherwise linker error
extern "C" {
    #include "soc/rtc_cntl_reg.h"
    #include "soc/rtc_io_reg.h"
    #include "soc/rtc.h"
    #include "driver/rtc_io.h"
    #include "driver/rtc_cntl.h"
    #include "driver/adc.h"
    #include "driver/dac.h"
    #include "soc/soc.h"
    #include "soc/rtc.h"
    #include "soc/rtc_cntl_reg.h"
    #include "soc/sens_reg.h"
    #include "esp32/ulp.h"
    #include "ulp_drivers.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "freertos/portmacro.h"
    #include "freertos/queue.h"
    #include "esp_log.h"
}


using namespace CTAG;
using namespace DRIVERS;

extern const uint8_t ulp_drivers_bin_start[] asm("_binary_ulp_drivers_bin_start");
extern const uint8_t ulp_drivers_bin_end[]   asm("_binary_ulp_drivers_bin_end");


static QueueHandle_t adcDataQueue = NULL;

static void IRAM_ATTR ulp_isr(void* arg)
{
    static uint16_t data[4];
    //static uint32_t s = 0;
    //gpio_set_level(23, (s++)&0x01);
    /* hardware debugging 
    gpio_set_level(SPI_TFT_MISO_PIN, 1);
    */

    // disable ULP timer, should get 10 us to settle
    CLEAR_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN);

    data[0] = (uint16_t)*((&ulp_adc_data) + 0);
    data[1] = (uint16_t)*((&ulp_adc_data) + 1);
    data[2] = (uint16_t)*((&ulp_adc_data) + 2);
    data[3] = (uint16_t)*((&ulp_adc_data) + 3);
    //setLedRGB(data[0], data[1], data[3]);
    
    xQueueSendToFrontFromISR(adcDataQueue, data, NULL );

    /* hardware debugging 
    gpio_set_level(23, 0);
    */
    //gpio_set_level(23, 0);
}

void IRAM_ATTR ADC::Update(){
    xQueueReceive(adcDataQueue, data, 0);
    // restart ULP
    SET_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN);
}

uint16_t ADC::GetChannelVal(int ch){
    return data[ch];
}

void ADC::InitADCSystem(){
    esp_err_t err;

    // create data queue
    adcDataQueue = xQueueCreate(1, sizeof(uint16_t)*4);

    // set up ULP program
    err = rtc_isr_register((intr_handler_t)&ulp_isr, (void*)0, (uint32_t)RTC_CNTL_SAR_INT_ST_M);
    ESP_ERROR_CHECK(err);
    REG_SET_BIT(RTC_CNTL_INT_ENA_REG, RTC_CNTL_ULP_CP_INT_ENA_M);
    // set RTC FAST CLOCK to XTAL/4 = 10MHz at 40Mhz XTAL speed
    rtc_clk_fast_freq_set(RTC_FAST_FREQ_XTALD4);
    init_ulp_program();
    ESP_ERROR_CHECK( ulp_run((&ulp_entry - RTC_SLOW_MEM) / sizeof(uint32_t)));
    SetCVINUnipolar(0);
    SetCVINUnipolar(1);
}

void ADC::SetCVINUnipolar(int ch){
    if(ch == 0){
        dac_output_voltage(DAC_CHANNEL_1, 55); // offset
        adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_0); // GPIO32
    }else if(ch == 1){
        dac_output_voltage(DAC_CHANNEL_2, 55);
        adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_0); // GPIO33
    }
}

void ADC::SetCVINBipolar(int ch){
    if(ch == 0){
        dac_output_voltage(DAC_CHANNEL_1, 57); // offset
        adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_6); // GPIO32
    }else if(ch == 1){
        dac_output_voltage(DAC_CHANNEL_2, 57);
        adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_6); // GPIO33
    }
}

uint16_t ADC::data[4];

void ADC::init_ulp_program()
{
    /* load ULP program */
    ESP_ERROR_CHECK( ulp_load_binary(0, ulp_drivers_bin_start,(ulp_drivers_bin_end - ulp_drivers_bin_start) / sizeof(uint32_t)));
    /* config DAC for offset voltage */
    /* default for 0->5V CV in */
    dac_output_enable(DAC_CHANNEL_1);
    dac_output_voltage(DAC_CHANNEL_1, 58);
    dac_output_enable(DAC_CHANNEL_2);
    dac_output_voltage(DAC_CHANNEL_2, 58);
    /* config ADC channels */
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_0); // GPIO32
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_0); // GPIO33
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_0); // GPIO34
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_0); // GPIO35
    /* enable ULP on adc 1 */
    adc1_ulp_enable();
}

void ADC::GetChannelVals(uint16_t *d){
    memcpy(d, data, sizeof(uint16_t) * 4);
}
