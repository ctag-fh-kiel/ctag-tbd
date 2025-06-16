from esphome.components.tbd_module.python_dependencies import python_dependencies

python_dependencies(('proto_schema_parser', 'proto-schema-parser'), 'pybind11')

from pathlib import Path
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome import automation
import logging

import tbd_core.buildgen as tbd

from tbd_core.api import ApiRegistry, find_events
from .generator import ApiWriter


_LOGGER = logging.getLogger(__file__)

CONF_MAX_PAYLOAD_SIZE = 'max_payload_size'

AUTO_LOAD = ['tbd_module', 'tbd_api']
CONFIG_SCHEMA = cv.Schema({
    cv.Optional(CONF_MAX_PAYLOAD_SIZE, 128): cv.positive_int,
})

APIS_GLOBAL = 'api'

API_NAMESPACE = cg.global_ns.namespace('tbd').namespace('api')


@tbd.generated_tbd_global(APIS_GLOBAL)
def get_api_registry() -> ApiRegistry:
    return ApiRegistry()

def register_actions_for_module(*source_files: Path | str, base_path: Path | str | None = None) -> None:
    """ Call this in module loading code! """

    events = find_events(*source_files, base_path=base_path)

    for event in events:
        action_name = f'tbd_api.{event.name}'
        cls_name = f'{event.event_name}_action'
        action = API_NAMESPACE.class_(cls_name, automation.Action)

        @automation.register_action(action_name, action, {})
        async def new_action(config, action_id, template_arg, args):
            var = cg.new_Pvariable(action_id, template_arg)
            return var

def _register_actions():
    base_path = Path(__file__).parent / 'src'
    register_actions_for_module('base_endpoints.cpp', 'api_test.cpp', base_path=base_path)
_register_actions()

async def to_code(config):
    component = tbd.new_tbd_component(__file__)
    cg.add_library('nanopb/Nanopb', '^0.4.91')

    component.add_define('TBD_API_MAX_PAYLOAD_SIZE', config[CONF_MAX_PAYLOAD_SIZE])

    # add core endpoints implementations and types
    api_registry = get_api_registry()
    api_registry.add_message_types(component.path / 'src' / 'api_base.proto')
    api_registry.add_message_types(component.path / 'src' / 'api_test.proto')
    api_registry.add_source(component.path / 'src' / 'base_endpoints.cpp')
    api_registry.add_source(component.path / 'src' / 'api_test.cpp')

    tbd.add_generation_job(finalize_api_registry)


@tbd.build_job_with_priority(tbd.GenerationStages.API)
def finalize_api_registry():
    message_files_dir = Path() / 'src' / 'messages'
    python_client_path = tbd.get_build_path() / 'clients' / 'python'
    typescript_client_path = tbd.get_build_path() / 'clients' / 'typescript'
    arduino_client_path = tbd.get_build_path() / 'clients' / 'arduino'

    api = get_api_registry().get_api()
    api_gen = ApiWriter(api)

    messages_dir = tbd.get_build_path() / message_files_dir
    api_gen.write_messages(tbd.get_generated_sources_path())
    api_gen.write_endpoints(tbd.get_generated_sources_path())
    api_gen.write_events(tbd.get_generated_sources_path())
    api_gen.write_protos(messages_dir)


    api_gen.write_python_client(python_client_path, messages_dir)
    api_gen.write_typescript_client(typescript_client_path, messages_dir)
    api_gen.write_arduino_client(arduino_client_path, messages_dir)

    cg.add_platformio_option('custom_nanopb_protos', [f'+<{ messages_dir / api_gen.dtos_proto }>'])
