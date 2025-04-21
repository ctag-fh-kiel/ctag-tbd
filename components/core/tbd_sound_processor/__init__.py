from esphome.components.tbd_module import new_tbd_component
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.core import CORE
from pathlib import Path

AUTO_LOAD = ['tbd_common']

CONFIG_SCHEMA = {}

async def to_code(config):
    component = new_tbd_component(__file__)

    utils_include_dir = component.include_dir / 'tbd' / 'sound_utils'
    component.add_include_dir(utils_include_dir)

    # component.add_include_dir(component.source_dir)
     
    if CORE.is_esp32: 
        from esphome.components.esp32 import add_idf_component
        cg.add_define('USE_ESP_DSP')
        add_idf_component(
            name="esp-dsp",
            repo="https://github.com/espressif/esp-dsp.git",
            ref="v1.6.1",
            # refresh='1day'
        )
        
        dsp_include = Path(CORE.build_path) / 'components' / 'esp-dsp' / 'modules' / 'iir' / 'include'
        cg.add_build_flag(f'-I{dsp_include}')