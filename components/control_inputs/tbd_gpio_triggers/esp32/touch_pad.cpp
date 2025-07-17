#ifdef TBD_USE_TOUCHPAD

#include <tbd/private/touch_pad.hpp>

#include <driver/touch_pad.h>


#define TOUCH_PAD TOUCH_PAD_NUM6

namespace {
    uint16_t touch_threshold;
}

namespace tbd::drivers {

void TouchPad::init() {
    touch_pad_init();
    touch_pad_config(TOUCH_PAD);
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_fsm_start();
}

void TouchPad::calibrate() {
    for(int i = 0; i < 16; i++) {
        auto touch_value = touch_intensity();
        touch_threshold += touch_value;
    }
    touch_threshold /= 16;
}

uint16_t TouchPad::touch_intensity() {
    uint16_t touch_value;
    touch_pad_read_raw_data(TOUCH_PAD, &touch_value);
    return touch_value;
}

bool TouchPad::is_being_pressed() {
    touch_value > noTouch + 1000
}

}

#endif