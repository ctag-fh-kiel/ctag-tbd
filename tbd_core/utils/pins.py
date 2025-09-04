from typing import Any
import esphome.config_validation as cv
from esphome import pins


CONF_PINS = 'pins'
CONF_TYPE = 'type'

CONF_SPI = 'spi'
CONF_SPI_HOST = 'host'
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
    cv.Required(CONF_SPI_HOST): cv.int_,
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


def get_pin_label(key: str, config: dict[str, Any]) -> str:
    return f'GPIO_NUM_{config[key]['number']}'


def get_spi_pinout_defines(prefix: str, config: dict[str, Any]) -> list[tuple[str, str]]:
    defines = [
        (f'{prefix}_HOST', f'SPI{config[CONF_SPI_HOST]}_HOST'),
        (f'{prefix}_PIN_SCLK', get_pin_label(CONF_SCLK_PIN, config)),
        (f'{prefix}_PIN_MOSI', get_pin_label(CONF_MOSI_PIN, config)),
        (f'{prefix}_PIN_MISO', get_pin_label(CONF_MISO_PIN, config)),
    ]
    if CONF_SCLK_PIN in config:
        defines.append((f'{prefix}_PIN_CS', get_pin_label(CONF_CS_PIN, config))),
    return defines


def get_i2c_pinout_defines(prefix: str, config: dict[str, Any]) -> list[tuple[str, str]]:
    return [
        (f'{prefix}_PIN_SCL', get_pin_label(CONF_SCL_PIN, config)),
        (f'{prefix}_PIN_SDA', get_pin_label(CONF_SDA_PIN, config)),
    ]


def get_i2s_pinout_defines(prefix: str, config: dict[str, Any]) -> list[tuple[str, str]]:
    defines = [
        (f'{prefix}_PIN_BCLK', get_pin_label(CONF_BCLK_PIN, config)),
        (f'{prefix}_PIN_WS', get_pin_label(CONF_WS_PIN, config)),
        (f'{prefix}_PIN_DOUT', get_pin_label(CONF_DOUT_PIN, config)),
        (f'{prefix}_PIN_DIN', get_pin_label(CONF_DIN_PIN, config)),
    ]
    if CONF_MCLK_PIN in config:
        defines.append((f'{prefix}_PIN_MCLK', get_pin_label(CONF_MCLK_PIN, config)))
    return defines


__all__ = [
    'SPIPinConfig',
    'I2CPinConfig',
    'I2SPinConfig',
    'CONF_PINS',
    'CONF_TYPE',
    'CONF_SPI',
    'CONF_SCLK_PIN',
    'CONF_MOSI_PIN',
    'CONF_MISO_PIN',
    'CONF_CS_PIN',
    'CONF_I2C',
    'CONF_SCL_PIN',
    'CONF_SDA_PIN',
    'CONF_I2S',
    'CONF_MCLK_PIN',
    'CONF_BCLK_PIN',
    'CONF_WS_PIN',
    'CONF_DOUT_PIN',
    'CONF_DIN_PIN',
    'get_spi_pinout_defines',
    'get_i2c_pinout_defines',
    'get_i2s_pinout_defines',
]