from esphome.core import CORE
import esphome.config_validation as cv
import esphome.codegen as cg

from esphome.components.tbd_module import new_tbd_component

DEPENDENCIES = ['tbd_indicator', 'tbd_control_inputs']

AUTO_LOAD = [
    'tbd_module', 
    'tbd_system', 
    'tbd_config',
    'tbd_presets', 
    'tbd_storage', 
    'tbd_sound_utils',
    'tbd_sound_processor',
]

CONFIG_SCHEMA = {}

async def to_code(config):
    new_tbd_component(__file__)

    sound_manager = cg.global_ns.namespace('tbd').namespace('audio').namespace('SoundProcessorManager')
    cg.add(sound_manager.begin())

