from esphome.core import CORE
import esphome.config_validation as cv
import esphome.codegen as cg
from pathlib import Path


from esphome.components.tbd_module import new_tbd_component

AUTO_LOAD = ['tbd_module']

CONF_USE_FILESYSTEM_WRAPPER = 'use_filesystem_wrapper'
CONF_SAMPLES_ADDRESS        = 'sample_rom_start'
CONF_SAMPLES_SIZE           = 'sample_rom_size'

DEFAULT_SAMPLE_ROM_START_ADDRESS=0xB00000
DEFAULT_SAMPLE_ROM_SIZE=0x1500000

if CORE.is_host:
    CONFIG_SCHEMA = cv.Schema({
        cv.Optional(CONF_USE_FILESYSTEM_WRAPPER, default=False): bool, 
    })
else:
    CONFIG_SCHEMA = cv.Schema(cv.Schema({
    cv.Optional(CONF_SAMPLES_ADDRESS, default=DEFAULT_SAMPLE_ROM_START_ADDRESS): cv.int_range(min=0),
    cv.Optional(CONF_SAMPLES_SIZE, default=DEFAULT_SAMPLE_ROM_SIZE): cv.int_range(min=0),
}))


async def to_code(config):
    new_tbd_component(__file__)

    if CORE.is_host:
        if config[CONF_USE_FILESYSTEM_WRAPPER]:
            CORE.add_build_flag('-DTBD_FILE_SYSTEM_USE_WRAPPER=1')
        else:
            CORE.add_build_flag('-DTBD_FILE_SYSTEM_USE_WRAPPER=0')
    else:
        from esphome.components.esp32 import add_idf_component

        CORE.add_build_flag('-DTBD_FILE_SYSTEM_USE_WRAPPER=1')
        cg.add_build_flag(f'-DTBD_STORAGE_SAMPLES_ADDRESS={config[CONF_SAMPLES_ADDRESS]}')
        cg.add_build_flag(f'-DTBD_DEFAULT_SAMPLE_ROM_SIZE={config[CONF_SAMPLES_SIZE]}')

        cg.add_define('USE_LITTLEFS')
        add_idf_component(
            name="littlefs",
            repo="https://github.com/joltwallet/esp_littlefs.git",
            ref="v1.19.1",
            submodules=['src/littlefs']
            # refresh='1day'
        )

        littlefs_include = Path(CORE.build_path) / 'components' / 'littlefs' / 'include'
        cg.add_build_flag(f'-I{littlefs_include}')
