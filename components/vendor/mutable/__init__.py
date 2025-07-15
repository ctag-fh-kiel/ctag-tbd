from tbd_core.buildgen import new_tbd_component, get_tbd_vendor_root


async def to_code(config):
    component = new_tbd_component(__file__)
    lib_dir = get_tbd_vendor_root() / 'mutable' / 'eurorack'
    component.add_external_library(name='mutable', repository=f'file://{lib_dir}')
