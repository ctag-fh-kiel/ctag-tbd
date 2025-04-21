import esphome.config_validation as cv

import esphome.components.tbd_audio_device as ad

AUTO_LOAD = ['tbd_audio_device']

CONFIG_SCHEMA = {
    cv.Required(ad.CONF_PINS): {
        cv.Required(ad.CONF_I2C): ad.I2CPinConfig,
        cv.Required(ad.CONF_I2S): ad.I2SPinConfig,
    }
}

async def to_code(config):
    pins = config[ad.CONF_PINS]
    device = ad.new_tbd_audio_device(__file__, ad.SampleIO.WORKER, config)
    device.add_i2c(pins[ad.CONF_I2C])
    device.add_i2s(pins[ad.CONF_I2S])
