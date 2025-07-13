from esphome.components.tbd_module.python_dependencies import python_dependencies

from esphome.components.tbd_serialization import get_dto_registry
from esphome.components.tbd_serialization import get_dtos

from tbd_core.buildgen import GenerationStages, get_reflectables
from tbd_core.buildgen.component_info import AutoReflection
from tbd_core.serialization import SerializableGenerator

python_dependencies(('proto_schema_parser', 'proto-schema-parser'), 'pybind11')

from pathlib import Path
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome import automation
import logging

import tbd_core.buildgen as tbd

from tbd_core.api import ApiRegistry, find_events, Api
from .generator import ApiWriter


_LOGGER = logging.getLogger(__file__)


API_REGISTRY_GLOBAL = 'api_registry'
APIS_GLOBAL = 'apis'

API_NAMESPACE = cg.global_ns.namespace('tbd').namespace('api')
CONF_MAX_PAYLOAD_SIZE = 'max_payload_size'


AUTO_LOAD = ['tbd_module', 'tbd_api']
CONFIG_SCHEMA = cv.Schema({
    cv.Optional(CONF_MAX_PAYLOAD_SIZE, 1024): cv.positive_int,
})


@tbd.generated_tbd_global(API_REGISTRY_GLOBAL, after_stage=GenerationStages.REFLECTION)
def get_api_registry() -> ApiRegistry:
    return ApiRegistry(get_dto_registry())


@tbd.generated_tbd_global(APIS_GLOBAL, after_stage=GenerationStages.API)
def get_api() -> Api:
    return ApiRegistry(get_dto_registry()).get_api()


async def to_code(config):
    component = tbd.new_tbd_component(__file__, auto_reflect=AutoReflection.ALL)
    component.add_external_dependency('nanopb/Nanopb', '^0.4.91')
    component.add_define('TBD_API_MAX_PAYLOAD_SIZE', config[CONF_MAX_PAYLOAD_SIZE])
    tbd.add_generation_job(finalize_api_registry)
    tbd.add_generation_job(generate_clients)


@tbd.build_job_with_priority(tbd.GenerationStages.API)
def finalize_api_registry():
    api_gen = ApiWriter(get_api(), get_dtos())

    gen_sources_dir = tbd.get_build_path() / tbd.get_generated_sources_path()
    api_gen.write_messages(gen_sources_dir)
    api_gen.write_endpoints(gen_sources_dir)
    api_gen.write_events(gen_sources_dir)


@tbd.build_job_with_priority(tbd.GenerationStages.CLIENTS)
def generate_clients():
    python_client_path = tbd.get_build_path() / 'clients' / 'python'
    typescript_client_path = tbd.get_build_path() / 'clients' / 'typescript'
    arduino_client_path = tbd.get_build_path() / 'clients' / 'arduino'

    dto_gen = SerializableGenerator(get_dtos()['api'], get_reflectables())
    api_gen = ApiWriter(get_api(), dto_gen)

    messages_dir = tbd.get_build_path() / tbd.get_messages_path() / 'api'

    api_gen.write_python_client(python_client_path, messages_dir)
    api_gen.write_typescript_client(typescript_client_path, messages_dir)
    api_gen.write_arduino_client(arduino_client_path, messages_dir)
