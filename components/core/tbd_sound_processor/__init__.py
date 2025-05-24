from esphome.components.tbd_module import new_tbd_component
import esphome.config_validation as cv
import esphome.codegen as cg

# TBDings has requires the most memory at 113944 bytes
# use 112k=114688 bytes as default
DEFAULT_FIXED_ALLOCATION_SIZE = 114688

CONF_MAX_MEMORY = 'max_memory'

CONFIG_SCHEMA = cv.Schema({
    cv.Optional(CONF_MAX_MEMORY, default=DEFAULT_FIXED_ALLOCATION_SIZE): cv.positive_int,
})

async def to_code(config):
    new_tbd_component(__file__)

    # max_memory of 0 reverts to normal heap allocation
    if buffer_size := config.get(CONF_MAX_MEMORY):
        allocator = cg.global_ns.namespace('tbd').namespace('audio').namespace('SoundProcessorAllocator')
        allocator.allocate_buffers(buffer_size)



