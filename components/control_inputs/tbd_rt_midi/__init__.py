from esphome.components.tbd_control_inputs import new_tbd_control_input
import esphome.config_validation as cv
import esphome.codegen as cg

AUTO_LOAD = ['tbd_control_inputs']
DEPENDENCIES = ['host']

NUM_CHANNELS = 90
NUM_TRIGGERS = 40

CONFIG_SCHEMA = cv.Schema({})

async def to_code(config):
    new_tbd_control_input(__file__, NUM_CHANNELS, NUM_TRIGGERS, config)
