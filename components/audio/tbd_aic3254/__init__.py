import esphome.config_validation as cv

import esphome.components.tbd_audio_device as ad

AUTO_LOAD = ['tbd_audio_device']

CONFIG_SCHEMA = {
    cv.Required(ad.CONF_PINS): {
        cv.Required(ad.CONF_I2C): ad.I2CPinConfig,
        cv.Required(ad.CONF_I2S): ad.I2SPinConfig,
    },
    cv.Optional(ad.CONF_CHUNK_SIZE, default=ad.DEFAULT_CHUNK_SIZE): int,
}

async def to_code(config):
    pins = config[ad.CONF_PINS]
    params = ad.AudioDeviceParams(
        sample_rate=ad.DEFAULT_SAMPLE_RATE,
        num_channels=2,
        chunk_size=config[ad.CONF_CHUNK_SIZE],
        sample_io=ad.SampleIO.WORKER,
    )
    
    device = ad.new_tbd_audio_device(__file__, params)
    device.add_i2c(pins[ad.CONF_I2C])
    device.add_i2s(pins[ad.CONF_I2S])
