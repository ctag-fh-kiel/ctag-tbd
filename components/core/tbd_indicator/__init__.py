from esphome.core import CORE
import esphome.config_validation as cv
from esphome import pins

import esphome.components.tbd_module as tbd


AUTO_LOAD = ['tbd_module']

CONF_PINS     = 'pins'
CONF_PIN      = 'pin'
CONF_RED_PIN  = 'red'
CONF_GREEN_PN = 'green'
CONF_BLUE_PIN = 'blue'


CONFIG_SCHEMA = cv.Schema({
    cv.Optional(CONF_PINS): cv.All(cv.only_on_esp32, {
        cv.Required(CONF_RED_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_GREEN_PN): pins.gpio_output_pin_schema,
        cv.Required(CONF_BLUE_PIN): pins.gpio_output_pin_schema,
    }),
    cv.Optional(CONF_PIN): cv.All(cv.only_on_esp32, pins.gpio_output_pin_schema)
})

def add_rgb_pins(config, module: tbd.ComponentInfo):
    def get_pin_number(key):
        return f'GPIO_NUM_{config[key]['number']}'

    module.add_define('TBD_INDICATOR_USE_RGB')
    module.add_define(f'TBD_RGB_PIN_RED', get_pin_number(CONF_RED_PIN))
    module.add_define(f'TBD_RGB_PIN_GREEN', get_pin_number(CONF_GREEN_PN))
    module.add_define(f'TBD_RGB_PIN_BLUE', get_pin_number(CONF_BLUE_PIN))

async def to_code(config):
    module = tbd.new_tbd_component(__file__)

    if not tbd.is_desktop():
        pin = config.get(CONF_PIN)
        pins = config.get(CONF_PINS)

        if pin is None and pins is None:
            raise ValueError('either rgb pins option or neopixel pin option required')
        
        if pin is not None and pins is not None:
            raise ValueError('can not have both rgb pinout and neopixel pin')

        if pins is not None:
            add_rgb_pins(config[CONF_PINS], module)
        else:
            module.add_define('TBD_INDICATOR_USE_NEOPIXEL')
            module.add_define('TBD_NEOPIXEL_PIN_DOUT', f'GPIO_NUM_{pin['number']}')

            from esphome.components.esp32 import add_idf_component
            add_idf_component(
                name="led_strip",
                repo="https://github.com/espressif/idf-extra-components.git",
                path="led_strip"
                # refresh='1day'
            )

