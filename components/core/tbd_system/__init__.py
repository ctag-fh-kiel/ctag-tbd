from esphome.components.tbd_module import new_tbd_component

AUTO_LOAD = ['tbd_module']
CONFIG_SCHEMA = {}

async def to_code(config):
    new_tbd_component(__file__)