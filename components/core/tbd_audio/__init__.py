from esphome.components.tbd_module.python_dependencies import python_dependencies
python_dependencies('pydantic', 'cxxheaderparser', 'jinja2')

from esphome.components.tbd_module import new_tbd_component

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.core import CORE
from pathlib import Path

AUTO_LOAD = ['tbd_sound_utils', 'tbd_dsp']

CONFIG_SCHEMA = {}

async def to_code(config):
    new_tbd_component(__file__)

    audio_loop = cg.global_ns.namespace('tbd').namespace('audio_loop')
    cg.add(audio_loop.begin())