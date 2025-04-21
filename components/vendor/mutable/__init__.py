import esphome.codegen as cg

from esphome.components.tbd_module import get_tbd_source_root

lib_dir = get_tbd_source_root() / 'vendor' / 'mutable'
cg.add_platformio_option('lib_deps', [f'mutable=symlink://{lib_dir}'])

