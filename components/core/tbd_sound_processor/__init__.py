from esphome.components.tbd_module import new_tbd_component
import esphome.config_validation as cv
from esphome.components.tbd_api import get_api_registry

from tbd_core.buildgen import AutoReflection

# TBDings has requires the most memory at 113944 bytes
# use 112k=114688 bytes as default
DEFAULT_FIXED_ALLOCATION_SIZE = 114688

CONF_MAX_MEMORY = 'max_memory'

CONFIG_SCHEMA = cv.Schema({
    cv.Optional(CONF_MAX_MEMORY, default=DEFAULT_FIXED_ALLOCATION_SIZE): cv.positive_int,
})

async def to_code(config):
    component = new_tbd_component(__file__, auto_reflect=AutoReflection.ALL)
    component.add_define('TBD_SOUND_PROCESSOR_MAX_MEMORY', config[CONF_MAX_MEMORY])
