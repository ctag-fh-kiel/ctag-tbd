from pathlib import Path
import esphome.codegen as cg

from esphome.components.tbd_module import get_tbd_source_root

# lib_dir = get_tbd_source_root() / 'vendor' / 'moog'
lib_dir = Path('/home/mlee/code/tbd-esphome/vendor/moog/MoogLadders')
cg.add_platformio_option('lib_deps', [f'moog=file://{lib_dir}'])