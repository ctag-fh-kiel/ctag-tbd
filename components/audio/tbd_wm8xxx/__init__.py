import esphome.config_validation as cv

import esphome.components.tbd_audio_device as ad

AUTO_LOAD = ['tbd_audio_device']

CONFIG_SCHEMA = cv.Schema({
    cv.Required(ad.CONF_TYPE): cv.one_of('wm8731', 'wm8974', 'wm8978'),
    cv.Required(ad.CONF_PINS): {
        cv.Required(ad.CONF_SPI): ad.SPIPinConfig,
        cv.Required(ad.CONF_I2S): ad.I2SPinConfig,
    }
})

async def to_code(config):
    pins = config[ad.CONF_PINS]
    device = ad.new_tbd_audio_device(__file__, config)
    device.add_spi(pins[ad.CONF_SPI])
    device.add_i2s(pins[ad.CONF_I2S])
