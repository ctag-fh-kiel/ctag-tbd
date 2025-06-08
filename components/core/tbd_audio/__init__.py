from esphome.components.tbd_module.python_dependencies import python_dependencies
python_dependencies('pydantic', 'cxxheaderparser', 'jinja2', ('humps', 'pyhumps'))

from esphome.components.tbd_module import new_tbd_component

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.core import CORE
from pathlib import Path

AUTO_LOAD = ['tbd_system', 'tbd_sound_utils']

CONF_AUTOSTART = 'autostart'

CONFIG_SCHEMA = cv.Schema({
    cv.Optional(CONF_AUTOSTART, default=False): bool,
})

async def to_code(config):
    new_tbd_component(__file__)

    audio_loop = cg.global_ns.namespace('tbd').namespace('audio_loop')
    if config[CONF_AUTOSTART]:
        cg.add(audio_loop.begin())