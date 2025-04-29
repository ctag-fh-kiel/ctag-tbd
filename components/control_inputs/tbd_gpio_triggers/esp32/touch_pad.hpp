#include <cinttypes>


namespace tbd::drivers {

struct TouchPad {
    TouchPad() = delete;
    
    static void init();
    static void calibrate();
    
    static uint16_t touch_intensity();
    static bool is_being_touched();
};

}
