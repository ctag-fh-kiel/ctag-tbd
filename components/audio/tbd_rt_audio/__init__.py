import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_REFRESH

import esphome.components.tbd_audio_device as ad

from tbd_core.buildgen.build_deps import cmake_dependency


AUTO_LOAD = ['tbd_audio_device']

DEPENDENCIES = ['host']

CONFIG_SCHEMA = cv.Schema({
    cv.Optional(CONF_REFRESH, default="1d"): cv.All(
        cv.string, cv.source_refresh
    ),
    cv.Optional(ad.CONF_CHUNK_SIZE, default=ad.DEFAULT_CHUNK_SIZE): int,
})

async def to_code(config):
    # TODO: utilize a vanilla WAV library instead of something modified
    #

    # cg.add_library('rtaudio', '6.0.1', '')
    # cg.add_library('tinywav', None, 'https://github.com/mhroth/tinywav.git')

    params = ad.AudioDeviceParams(
        sample_rate=ad.DEFAULT_SAMPLE_RATE,
        num_channels=2,
        chunk_size=config[ad.CONF_CHUNK_SIZE],
        sample_io=ad.SampleIO.CALLBACK,
    )
    device = ad.new_tbd_audio_device(__file__, params, needs_exceptions=True)
    device.module.add_dependency(cmake_dependency(
        name='rtaudio',
        url='https://github.com/thestk/rtaudio.git',
        ref='5.2.0',
        cmake_parameters={'RTAUDIO_BUILD_STATIC_LIBS': 1}
    ))
    device.module.add_system_library('asound')

