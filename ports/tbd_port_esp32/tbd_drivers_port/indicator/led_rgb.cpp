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
#include <tbd/drivers/common/indicator.hpp>

#include "freertos/FreeRTOS.h"
#include "driver/ledc.h"


/*
static void led_test_task(void *pvParameter)
{
    unsigned int v = 0;
    while(1) {
        setIndicator(0, 0, v++);
        if(v>255)v=0;
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
*/

namespace {

ledc_channel_config_t ledc_channel[3];

}

namespace tbd::drivers {

void Indicator::Init() {

    // esp32sX does not support high speed mode
    #if SOC_LEDC_SUPPORT_HS_MODE
        auto speed_mode = LEDC_HIGH_SPEED_MODE;
    #else
        auto speed_mode = LEDC_LOW_SPEED_MODE;
    #endif

    ledc_timer_config_t ledc_timer;
    ledc_timer.duty_resolution = LEDC_TIMER_8_BIT; // resolution of PWM duty
    ledc_timer.freq_hz = 5000;                      // frequency of PWM signal
    ledc_timer.speed_mode = speed_mode;           // timer mode
    ledc_timer.timer_num = LEDC_TIMER_0;            // timer index
    ledc_timer.clk_cfg = LEDC_AUTO_CLK;              // Auto select the source clock

    ledc_timer_config(&ledc_timer);

    ledc_channel[0].channel = LEDC_CHANNEL_0;
    ledc_channel[0].duty = 0;
    ledc_channel[0].gpio_num = TBD_RGB_PIN_RED;
    ledc_channel[0].speed_mode = speed_mode;
    ledc_channel[0].hpoint = 0;
    ledc_channel[0].timer_sel = LEDC_TIMER_0;

    ledc_channel[1].channel = LEDC_CHANNEL_1;
    ledc_channel[1].duty = 0;
    ledc_channel[1].gpio_num = TBD_RGB_PIN_GREEN;
    ledc_channel[1].speed_mode = speed_mode;
    ledc_channel[1].hpoint = 0;
    ledc_channel[1].timer_sel = LEDC_TIMER_0;

    ledc_channel[2].channel = LEDC_CHANNEL_2;
    ledc_channel[2].duty = 0;
    ledc_channel[2].gpio_num = TBD_RGB_PIN_BLUE;
    ledc_channel[2].speed_mode = speed_mode;
    ledc_channel[2].hpoint = 0;
    ledc_channel[2].timer_sel = LEDC_TIMER_0;

    for (int ch = 0; ch < 3; ch++) {
        ledc_channel_config(&ledc_channel[ch]);
    }

    SetLedRGB(0, 0, 0);
    // testing
    //xTaskCreate(&led_test_task, "led_test_task", 4096, NULL, 5, NULL);
}

void Indicator::GetLedRGB(int &r, int &g, int &b) {
    r = ledc_get_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
    g = ledc_get_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel);
    b = ledc_get_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel);
}


void Indicator::SetLedRGB(int r, int g, int b) {
    ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, r >> 1);
    ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
    ledc_set_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel, g >> 2);
    ledc_update_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel);
    ledc_set_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel, b >> 3);
    ledc_update_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel);
}


void Indicator::SetLedR(int r) {
    ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, r >> 1);
    ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
}


void Indicator::SetLedG(int g) {
    ledc_set_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel, g >> 2);
    ledc_update_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel);
}


void Indicator::SetLedB(int b) {
    ledc_set_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel, b >> 3);
    ledc_update_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel);
}

}
