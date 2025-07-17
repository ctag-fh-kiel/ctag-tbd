from tbd_core.buildgen import new_tbd_component, get_tbd_vendor_root


async def to_code():
    component = new_tbd_component(__file__)
    lib_dir = get_tbd_vendor_root() / 'moog' / 'MoogLadders'
    component.add_external_dependency(name='moog', repository=f'file://{lib_dir}')