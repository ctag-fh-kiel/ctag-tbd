from pathlib import Path
import esphome.codegen as cg


async def to_code(config):
    # cg.add_platformio_option('lib_deps', [f'mutable=https://github.com/ctag-fh-kiel/eurorack.git#tbd-esphome'])
    lib_dir = Path('/home/mlee/code/tbd-esphome/vendor/mutable/eurorack')
    cg.add_platformio_option('lib_deps', [f'mutable=file://{lib_dir}'])
