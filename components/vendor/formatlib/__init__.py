import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components.tbd_module import new_tbd_component

REQUIRES = ['tbd_module']

# cg.add_library('formatlib', None, 'https://github.com/fmtlib/fmt.git')

async def to_code(config: cv.Schema):
    new_tbd_component(__file__)
