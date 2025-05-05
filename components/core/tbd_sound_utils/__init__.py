from esphome.components.tbd_module import new_tbd_component


AUTO_LOAD = ['tbd_module', 'tbd_dsp', 'tbd_storage', 'mutable']

async def to_code(config):
    component = new_tbd_component(__file__)

    utils_include_dir = component.include_dir / 'tbd' / 'sound_utils'
    component.add_include_dir(utils_include_dir)

