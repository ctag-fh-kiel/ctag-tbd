from tbd_module import new_tbd_component

import esphome.codegen as cg
import esphome.config_validation as cv


AUTO_LOAD = [
    'tbd_module',
    'tbd_sound_processor',
    'tbd_control_inputs',
    'tbd_audio_device',
    'tbd_sound_utils',
]

CONF_AUTOSTART = 'autostart'

CONFIG_SCHEMA = cv.Schema({
    cv.Optional(CONF_AUTOSTART, default=False): bool,
})

async def to_code(config):
    new_tbd_component(__file__)

    audio_loop = cg.global_ns.namespace('tbd').namespace('audio_loop')
    if config[CONF_AUTOSTART]:
        cg.add(audio_loop.begin())
