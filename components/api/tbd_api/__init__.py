from esphome.components.tbd_module.python_dependencies import python_dependencies
python_dependencies(('proto_schema_parser', 'proto-schema-parser'), 'pybind11')

import subprocess
from pathlib import Path
import esphome.config_validation as cv
import esphome.components.tbd_module as tbd
import esphome.codegen as cg
import logging

from .registry import *
from .generator import ApiWriter


_LOGGER = logging.getLogger(__file__)

CONF_MAX_PAYLOAD_SIZE = 'max_payload_size'

AUTO_LOAD = ['tbd_module', 'tbd_api']
CONFIG_SCHEMA = cv.Schema({
    cv.Optional(CONF_MAX_PAYLOAD_SIZE, 128): cv.positive_int,
})

APIS_GLOBAL = 'api'


@tbd.generated_tbd_global(APIS_GLOBAL)
def get_api_registry() -> ApiRegistry:
    return ApiRegistry()


async def to_code(config):
    component = tbd.new_tbd_component(__file__)
    cg.add_library('nanopb/Nanopb', '^0.4.91')

    component.add_define('TBD_API_MAX_PAYLOAD_SIZE', config[CONF_MAX_PAYLOAD_SIZE])

    # add core endpoints implementations and types
    get_api_registry().add_message_types(component.path / 'src' / 'api_base.proto')
    get_api_registry().add_message_types(component.path / 'src' / 'api_test.proto')
    get_api_registry().add_endpoints(component.path / 'src' / 'base_endpoints.cpp')

    tbd.add_generation_job(finalize_api_registry)



@tbd.build_job_with_priority(tbd.GenerationStages.API)
def finalize_api_registry():
    message_files_dir = Path() / 'src' / 'messages'
    python_client_path = tbd.get_build_path() / 'clients' / 'python'
    typescript_client_path = tbd.get_build_path() / 'clients' / 'typescript'

    api = get_api_registry().get_api()
    api_gen = ApiWriter(api)

    api_gen.write_endpoints(tbd.get_generated_sources_path())
    api_gen.write_protos(tbd.get_build_path() / message_files_dir)

    messages_dir = tbd.get_build_path() / message_files_dir
    api_gen.write_python_client(python_client_path, messages_dir)
    api_gen.write_typescript_client(typescript_client_path, messages_dir)

    cg.add_platformio_option('custom_nanopb_protos', [f'+<{ messages_dir / api_gen.dtos_proto }>'])
