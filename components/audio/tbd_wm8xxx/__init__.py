import esphome.config_validation as cv

import esphome.components.tbd_audio_device as ad

import tbd_core.utils as tbu

AUTO_LOAD = ['tbd_audio_device']

CONFIG_SCHEMA = cv.Schema({
    cv.Required(tbu.CONF_TYPE): cv.one_of('wm8731', 'wm8974', 'wm8978'),
    cv.Required(tbu.CONF_PINS): {
        cv.Required(tbu.CONF_SPI): tbu.SPIPinConfig,
        cv.Required(tbu.CONF_I2S): tbu.I2SPinConfig,
    },
    cv.Optional(ad.CONF_CHUNK_SIZE, default=ad.DEFAULT_CHUNK_SIZE): int,
})

async def to_code(config):
    pins = config[tbu.CONF_PINS]
    params = ad.AudioDeviceParams(
        sample_rate=ad.DEFAULT_SAMPLE_RATE,
        num_channels=2,
        chunk_size=config[ad.CONF_CHUNK_SIZE],
        sample_io=ad.SampleIO.WORKER,
    )
    
    device = ad.new_tbd_audio_device(__file__, params)
    device.add_spi(pins[tbu.CONF_SPI])
    device.add_i2s(pins[tbu.CONF_I2S])

    device_type_flag = f'TBD_AUDIO_{config[tbu.CONF_TYPE].upper()}'
    device.module.add_define(device_type_flag)
    