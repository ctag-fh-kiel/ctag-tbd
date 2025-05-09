from esphome.components.tbd_module.python_dependencies import python_dependencies
python_dependencies('proto-schema-parser')


from dataclasses import dataclass
from pathlib import Path
import esphome.config_validation as cv
import esphome.components.tbd_module as tbd
import esphome.codegen as cg
import logging

from .registry import *
from .generator import *


AUTO_LOAD = ['tbd_module', 'tbd_api']

_LOGGER = logging.getLogger(__file__)


CONFIG_SCHEMA = cv.Schema({})


APIS_GLOBAL = 'api'


@tbd.generated_tbd_global(APIS_GLOBAL)
def get_api_registry() -> ApiRegistry:
    return ApiRegistry()


# def new_component_api(path: str, component: tbd.ComponentInfo):
#     registry = get_api_registry()


async def to_code(config):
    component = tbd.new_tbd_component(__file__)
    cg.add_library('nanopb/Nanopb', '^0.4.91')

    # message_dir = module.source_dir
    # for file in message_dir.glob('*.proto'):
    #     out_path = message_output_dir / file.name
    #     tbd.copy_file_if_outdated(file, out_path)
    #     cg.add_platformio_option('custom_nanopb_protos', [f'+<{out_path}>'])

    # for file in message_dir.glob('*.options'):
    #     out_file = message_output_dir / file.name
    #     tbd.copy_file_if_outdated(file, out_file)

    get_api_registry().add_message_types(component.path / 'src' / 'api_base.proto')
    get_api_registry().add_message_types(component.path / 'src' / 'api_test.proto')
    get_api_registry().add_endpoints(component.path / 'src' / 'base_endpoints.cpp')
    tbd.add_generation_job(finalize_api_registry)


@tbd.build_job_with_priority(tbd.GenerationStages.API)
def finalize_api_registry():
    MESSAGE_BUILD_DIR = Path() / 'src' / 'messages'

    api_registry = get_api_registry()

    proto_build_files = []
    for proto_file in api_registry.proto_files:
            tbd.copy_file_if_outdated(proto_file, MESSAGE_BUILD_DIR / proto_file.name)
            proto_build_file = MESSAGE_BUILD_DIR / proto_file.name
            cg.add_platformio_option('custom_nanopb_protos', [f'+<{proto_build_file}>'])
            proto_build_files.append(proto_build_file)

    generator = ApiGenerator(api_registry)

    WRAPPERS_FILE = tbd.get_build_path() / MESSAGE_BUILD_DIR / 'wrappers.proto'
    generator.write_endpoints(tbd.get_generated_sources_path())
    generator.write_protos(WRAPPERS_FILE)
    cg.add_platformio_option('custom_nanopb_protos', [f'+<{WRAPPERS_FILE}>'])
