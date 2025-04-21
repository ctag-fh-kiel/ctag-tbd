import esphome.codegen as cg

from esphome.components.tbd_module import get_tbd_source_root

lib_dir = get_tbd_source_root() / 'vendor' / 'moog'
cg.add_platformio_option('lib_deps', [f'moog=symlink://{lib_dir}'])