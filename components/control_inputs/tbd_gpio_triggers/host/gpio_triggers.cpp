#include <tbd/drivers/common/gpio.hpp>


namespace tbd::drivers {

void GPIO::InitGPIO() {

}


uint8_t GPIO::GetTrig0() {
    return 0;
}


uint8_t GPIO::GetTrig1() {
    return 0;
}


TBD_IRAM uint8_t GPIO::GetPushButton() {
    return 0;
}

}
