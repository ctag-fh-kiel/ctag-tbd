#if TBD_INDICATOR_USE_NEOPIXEL

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
#include <tbd/indicator.hpp>

#include "esp_log.h"

#include "led_strip_types.h"
#include "led_strip.h"

namespace tbd::drivers {

static led_strip_handle_t led_strip;
static int _r, _g, _b;

void Indicator::init() {
    ESP_LOGI("RGBLed", "RGB LED strip initialization");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
            .strip_gpio_num = TBD_NEOPIXEL_PIN_DOUT,
            .max_leds = 1, // at least one LED on board
            // .led_pixel_format = 0,
            .led_model = LED_MODEL_WS2812,
            .flags = {
                    .invert_out = 0,
            },
    };

    led_strip_rmt_config_t rmt_config = {
            .clk_src = RMT_CLK_SRC_DEFAULT,
            .resolution_hz = 10 * 1000 * 1000, // 10MHz
            .mem_block_symbols = 0,
            .flags = {
                .with_dma = false,
            }
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_clear(led_strip);
    _r = _g = _b = 0;
}

void Indicator::GetLedRGB(int &r, int &g, int &b) {
    r = _r;
    g = _g;
    b = _b;
}

void Indicator::SetLedRGB(int r, int g, int b) {
    r >>= 2;
    g >>= 3;
    b >>= 2;
    _r = r;
    _g = g;
    _b = b;
    led_strip_set_pixel(led_strip, 0, r, g, b);
    led_strip_refresh(led_strip);
}

void Indicator::SetLedR(int r) {
    SetLedRGB(r, _g, _b);
}

void Indicator::SetLedG(int g) {
    SetLedRGB(_r, g, _b);
}

void Indicator::SetLedB(int b) {
    SetLedRGB(_r, _g, b);
}

}

#endif