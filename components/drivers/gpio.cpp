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


#include "gpio.hpp"
#include "driver/gpio.h"
#include <string.h>

#define TRIG0_PIN 39
#define TRIG1_PIN 36
#define GPIO_INPUT_PIN_SEL  ((1ULL<<TRIG0_PIN)| (1ULL<<TRIG1_PIN ))

using namespace CTAG::DRIVERS;

void GPIO::InitGPIO(){
    gpio_config_t io_conf;
    memset(&io_conf, 0, sizeof(io_conf));
    // inputs
    //interrupt edge
    io_conf.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_NEGEDGE;
    //bit mask of the pins
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    //io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    t0delay = 0;
    t1delay = 0;
}

// trigs get delayed as ADC sampling is slow, delay approx. 3ms
uint8_t IRAM_ATTR GPIO::GetTrig0(){
    t0delay <<= 1;
    t0delay |= gpio_get_level((gpio_num_t)TRIG0_PIN);
    return (uint8_t) ((t0delay & 0x08) > 1);
}

uint8_t IRAM_ATTR GPIO::GetTrig1(){
    t1delay <<= 1;
    t1delay |= gpio_get_level((gpio_num_t)TRIG1_PIN);
    return (uint8_t) ((t1delay & 0x08) > 1);
}

DRAM_ATTR uint32_t CTAG::DRIVERS::GPIO::t0delay;
DRAM_ATTR uint32_t CTAG::DRIVERS::GPIO::t1delay;