from esphome.core import CORE
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome import pins
from pathlib import Path

from esphome.components.tbd_module import new_tbd_component

AUTO_LOAD = ['tbd_module']

CONF_PINS     = 'pins'
CONF_PIN      = 'pin'
CONF_RED_PIN  = 'red'
CONF_GREEN_PN = 'green'
CONF_BLUE_PIN = 'blue'


if CORE.is_host:
    CONFIG_SCHEMA = cv.Schema({})
else:
    CONFIG_SCHEMA = cv.Schema({
        cv.Optional(CONF_PINS): {
            cv.Required(CONF_RED_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_GREEN_PN): pins.gpio_output_pin_schema,
            cv.Required(CONF_BLUE_PIN): pins.gpio_output_pin_schema,
        },
        cv.Optional(CONF_PIN): pins.gpio_output_pin_schema
    })

def add_rgb_pins(config):
    def get_pin_number(key):
        return f'GPIO_NUM_{config[key]['number']}'

    cg.add_build_flag('-DTBD_INDICATOR_USE_RGB=1')
    cg.add_build_flag(f'-DTBD_RGB_PIN_RED={get_pin_number(CONF_RED_PIN)}')
    cg.add_build_flag(f'-DTBD_RGB_PIN_GREEN={get_pin_number(CONF_GREEN_PN)}')
    cg.add_build_flag(f'-DTBD_RGB_PIN_BLUE={get_pin_number(CONF_BLUE_PIN)}')

async def to_code(config):
    new_tbd_component(__file__)
    if not CORE.is_host:
        pin = config.get(CONF_PIN)
        pins = config.get(CONF_PINS)

        if pin is None and pins is None:
            raise ValueError('either rgb pins option or neopixel pin option required')
        
        if pin is not None and pins is not None:
            raise ValueError('can not have both rgb pinout and neopixel pin')

        if pins is not None:
            add_rgb_pins(config[CONF_PINS])
        else:
            cg.add_build_flag('-DTBD_INDICATOR_USE_NEOPIXEL=1')
            cg.add_build_flag(f'-DTBD_NEOPIXEL_PIN_DOUT=GPIO_NUM_{pin['number']}')

            from esphome.components.esp32 import add_idf_component
            add_idf_component(
                name="led_strip",
                repo="https://github.com/espressif/idf-extra-components.git",
                path="led_strip"
                # refresh='1day'
            )

            led_strip_include = Path(CORE.build_path) / 'components' / 'led_strip' / 'include'
            cg.add_build_flag(f'-I{led_strip_include}')

