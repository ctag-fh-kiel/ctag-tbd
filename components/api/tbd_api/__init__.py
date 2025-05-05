from pathlib import Path
import esphome.config_validation as cv
import esphome.components.tbd_module as tbd
import esphome.codegen as cg
import logging


AUTO_LOAD = ['tbd_module']

_LOGGER = logging.getLogger(__file__)


CONFIG_SCHEMA = cv.Schema({})


APIS_GLOBAL = 'api'


class ApiRegistry:
    def __init__():
        pass

    def add_endpoint(path: str, header: Path, call: str):
        pass


@tbd.generated_tbd_global(APIS_GLOBAL)
def get_api_registry() -> ApiRegistry:
    return ApiRegistry()


def new_component_api(path: str, component: tbd.ComponentInfo):
    registry = get_api_registry()


async def to_code(config):
    module = tbd.new_tbd_component(__file__)
    cg.add_library('nanopb/Nanopb', '^0.4.91')

    message_dir = module.source_dir
    message_output_dir = Path() / 'src' / 'messages'

    for file in message_dir.glob('*.proto'):
        out_path = message_output_dir / file.name
        tbd.copy_file_if_outdated(file, out_path)
        cg.add_platformio_option('custom_nanopb_protos', [f'+<{out_path}>'])

    for file in message_dir.glob('*.options'):
        out_file = message_output_dir / file.name
        tbd.copy_file_if_outdated(file, out_file)


@tbd.build_job_with_priority(tbd.GenerationStages.API)
def finalize_api_registry():
    _LOGGER.error('apis registering')

tbd.add_generation_job(finalize_api_registry)