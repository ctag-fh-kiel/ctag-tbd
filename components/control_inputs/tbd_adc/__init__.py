from esphome.components.tbd_control_inputs import new_tbd_control_input
import esphome.config_validation as cv
from esphome.components.esp32 import add_idf_component, add_idf_sdkconfig_option
import esphome.codegen as cg

AUTO_LOAD = ['tbd_control_inputs']

NUM_CHANNELS = 4
NUM_TRIGGERS = 2

CONFIG_SCHEMA = cv.Schema({})

async def to_code(config):
    new_tbd_control_input(__file__, NUM_CHANNELS, NUM_TRIGGERS, config)