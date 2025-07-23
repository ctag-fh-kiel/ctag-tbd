import esphome.config_validation as cv

import esphome.components.tbd_audio_device as ad

import tbd_core.utils as tbu


AUTO_LOAD = ['tbd_audio_device']

CONFIG_SCHEMA = {
    cv.Required(tbu.CONF_PINS): {
        cv.Required(tbu.CONF_I2C): tbu.I2CPinConfig,
        cv.Required(tbu.CONF_I2S): ad.I2SPinConfig,
    },
    cv.Optional(ad.CONF_CHUNK_SIZE, default=ad.DEFAULT_CHUNK_SIZE): int,
}

async def to_code(config):
    pins = config[tbu.CONF_PINS]
    params = ad.AudioDeviceParams(
        sample_rate=ad.DEFAULT_SAMPLE_RATE,
        num_channels=2,
        chunk_size=config[ad.CONF_CHUNK_SIZE],
        sample_io=ad.SampleIO.WORKER,
    )
    
    device = ad.new_tbd_audio_device(__file__, params)
    device.add_i2c(pins[tbu.CONF_I2C])
    device.add_i2s(pins[tbu.CONF_I2S])
