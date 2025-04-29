import subprocess
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_REFRESH
import esphome.git as git
from esphome.components.tbd_module.cmake_dependencies import cmake_dependency

import esphome.components.tbd_audio_device as ad

AUTO_LOAD = ['tbd_audio_device']

DEPENDENCIES = ['host']

CONFIG_SCHEMA = cv.Schema({
    cv.Optional(CONF_REFRESH, default="1d"): cv.All(
        cv.string, cv.source_refresh
    ),
    cv.Optional(ad.CONF_CHUNK_SIZE, default=ad.DEFAULT_CHUNK_SIZE): int,
})

async def to_code(config):
    ad.ComponentInfo.enable_exceptions()
    # TODO: utilize a vanilla WAV library instead of something modified
    #

    # cg.add_library('rtaudio', '6.0.1', '')
    # cg.add_library('tinywav', None, 'https://github.com/mhroth/tinywav.git')
    
    rt_audio_lib = cmake_dependency(
        name='rtaudio',
        url='https://github.com/thestk/rtaudio.git',
        ref='5.2.0',
        cmake_parameters={'RTAUDIO_BUILD_STATIC_LIBS': 1}   
    )
    cg.add_build_flag('-lasound')

    params = ad.AudioDeviceParams(
        sample_rate=ad.DEFAULT_SAMPLE_RATE,
        num_channels=2,
        chunk_size=config[ad.CONF_CHUNK_SIZE],
        sample_io=ad.SampleIO.CALLBACK,
    )
    device = ad.new_tbd_audio_device(__file__, params)
