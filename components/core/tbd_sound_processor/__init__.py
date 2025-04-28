from esphome.components.tbd_module import new_tbd_component
from esphome.components.tbd_module.python_dependencies import python_dependencies
python_dependencies('pydantic', 'cxxheaderparser')

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.core import CORE
from pathlib import Path

AUTO_LOAD = ['tbd_common', 'tbd_dsp']

CONFIG_SCHEMA = {}

async def to_code(config):
    component = new_tbd_component(__file__)

    utils_include_dir = component.include_dir / 'tbd' / 'sound_utils'
    component.add_include_dir(utils_include_dir)
