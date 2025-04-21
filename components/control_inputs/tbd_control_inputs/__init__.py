from esphome.components.tbd_module import new_tbd_component, ComponentInfo
from esphome import pins
import esphome.config_validation as cv
from esphome.core import CORE
from dataclasses import dataclass

AUTO_LOAD = ['tbd_module', 'tbd_system']

CONF_PINS = 'pins'
CONF_TYPE = 'type'

CONF_SPI = 'spi'
CONF_SCLK_PIN = 'sclk'
CONF_MOSI_PIN = 'mosi'
CONF_MISO_PIN = 'miso'
CONF_CS_PIN = 'cs'

CONF_I2C = 'i2c'
CONF_SCL_PIN = 'scl'
CONF_SDA_PIN = 'sda'


CONF_I2S = 'i2s'
CONF_MCLK_PIN = 'mclk'
CONF_BCLK_PIN = 'bclk'
CONF_WS_PIN = 'ws'
CONF_DOUT_PIN = 'dout'
CONF_DIN_PIN = 'din'

# SPI device config pinout
SPIPinConfig = {
    # clock pin (SCLK)
    cv.Required(CONF_SCLK_PIN): pins.gpio_output_pin_schema,
    # data out pin (MOSI)
    cv.Required(CONF_MOSI_PIN): pins.gpio_output_pin_schema,
    # data in pin (MISO)
    cv.Required(CONF_MISO_PIN): pins.gpio_input_pin_schema,
    # chip select pin (CS/SS)
    cv.Required(CONF_CS_PIN): pins.gpio_output_pin_schema,
} 

# I2C device config pinout
I2CPinConfig = {
    # clock pin (SCL)
    cv.Required(CONF_SCL_PIN): pins.gpio_output_pin_schema,
    # bidirectional data pin (SDA)
    cv.Required(CONF_SDA_PIN): pins.gpio_output_pin_schema,

} 

# I2C audio data pinout
I2SPinConfig = {
    # master clock (MCLK)
    cv.Optional(CONF_MCLK_PIN): pins.gpio_output_pin_schema,
    # bit clock (BCLK)
    cv.Required(CONF_BCLK_PIN): pins.gpio_output_pin_schema,
    # word select /left-right clock (WS/LRCK)  
    cv.Required(CONF_WS_PIN): pins.gpio_output_pin_schema,
    # data out / DAC data pin (DOUT/DACDAT)
    cv.Required(CONF_DOUT_PIN): pins.gpio_output_pin_schema,
    # data in  / ADC data pin (DIN/ADCDAT)
    cv.Required(CONF_DIN_PIN): pins.gpio_input_pin_schema,
}

@dataclass
class ControlInputs:
    module: ComponentInfo
    num_channels: int 
    num_triggers: int
    # subtype: str | None = None

    def add_flag(self):
        CORE.add_build_flag(f'-DTBD_CV_{self.module.name.upper()}=1')

    def add_num_channels_flag(self):
        CORE.add_build_flag(f'-DN_CVS={self.num_channels}')

    def add_num_triggers_flag(self):
        CORE.add_build_flag(f'-DN_TRIGS={self.num_triggers}')

def new_tbd_control_input(init_file: str, num_channels: int, num_triggers: int, config, **kwargs):  
    module = new_tbd_component(init_file, **kwargs)
     
    # subtype = config[CONF_TYPE] if CONF_TYPE in config else None
    device = ControlInputs(module, num_channels, num_triggers)
    device.add_flag()
    device.add_num_channels_flag()
    device.add_num_triggers_flag()
    return device

new_tbd_component(__file__)