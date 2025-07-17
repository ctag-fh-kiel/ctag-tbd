import esphome.components.tbd_module as tbd
import esphome.codegen as cg

REQUIRES = ['tbd_module']
CONFIG_SCHEMA = {}

async def to_code(config):
    tbd.new_tbd_component(__file__)  
    cg.add(cg.RawExpression('TBD_LOGI("main", "starting TBD");'))
