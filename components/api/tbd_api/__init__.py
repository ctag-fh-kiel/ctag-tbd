from esphome.components.tbd_module.python_dependencies import python_dependencies
python_dependencies('proto-schema-parser')


from pathlib import Path
import esphome.config_validation as cv
import esphome.components.tbd_module as tbd
import esphome.codegen as cg
import subprocess
import logging


from .registry import *
from .generator import *

_LOGGER = logging.getLogger(__file__)


AUTO_LOAD = ['tbd_module', 'tbd_api']

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

    get_api_registry().add_message_types(component.path / 'src' / 'api_base.proto')
    get_api_registry().add_message_types(component.path / 'src' / 'api_test.proto')
    get_api_registry().add_endpoints(component.path / 'src' / 'base_endpoints.cpp')
    tbd.add_generation_job(finalize_api_registry)


@tbd.build_job_with_priority(tbd.GenerationStages.API)
def finalize_api_registry():
    MESSAGE_BUILD_DIR = Path() / 'src' / 'messages'
    PYTHON_API_PATH = tbd.get_build_path() / 'apis' / 'python'

    api_registry = get_api_registry()

    generator = ApiGenerator(api_registry)

    generator.write_endpoints(tbd.get_generated_sources_path())
    generator.write_protos(tbd.get_build_path() / MESSAGE_BUILD_DIR)
    generator.write_python_client(PYTHON_API_PATH)
    # --proto_path=$SRC_DIR --python_out=$DST_DIR $SRC_DIR/addressbook.proto
    API_TYPES_PATH = tbd.get_build_path() / MESSAGE_BUILD_DIR / generator.wrappers_proto

    print(['protoc', f'--proto_path={MESSAGE_BUILD_DIR}', f'--python_out={PYTHON_API_PATH}', generator.wrappers_proto])

    PYTHON_SOURCES_PATH = Path(__file__).parent / 'clients' / 'python'
    BASE_ENDPOINTS_PATH = Path(__file__).parent / 'base_endpoints.py'
    tbd.copy_tree_if_outdated(PYTHON_SOURCES_PATH, PYTHON_API_PATH)
    tbd.copy_file_if_outdated(BASE_ENDPOINTS_PATH, PYTHON_API_PATH / 'base_endpoints.py')

    subprocess.run(['protoc', f'--proto_path={tbd.get_build_path() / MESSAGE_BUILD_DIR}', f'--python_out={PYTHON_API_PATH}', generator.wrappers_proto])
    cg.add_platformio_option('custom_nanopb_protos', [f'+<{ API_TYPES_PATH }>'])
