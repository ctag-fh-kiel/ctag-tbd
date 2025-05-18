""" Main helper component for all TBD esphome components.

    
"""

from enum import Enum, unique
from pathlib import Path
import logging
from typing import Callable
from esphome.core import CORE
from esphome.coroutine import coroutine

from .registry import *
from .files import *
from .component_info import *
from .collect_errors import collect_errors
from .prepare_build import *


AUTO_LOAD = ['tbd_system']


_LOGGER = logging.getLogger(__name__)

def is_esp32():
    return CORE.is_esp32

def is_desktop():
    return CORE.is_host

# build job utils

@unique
class GenerationStages(Enum):
    """ Priority of tbd build setup stages.

        Stages with higher priority run first with default value `0.0`.

        TBD does a lot of processing and code generation. This processing needs to be done in a defined order, since
        the output of one processing stage may be required by a subsequent stage. For this purpose esphome allows
        build jobs to be assigned priorities. The default priority assigned to by esphome for calling `to_code` of
        each component is `0.0`.

        The priority represent the following stages:

        `DEFAULT`: Components registering and config stage.
            Priority of `to_code` call if not specified. Normal components `to_code` module level function
            evaluates the component config if present and registers the component, including sources, include paths
            and defines.

        `COMPONENT`: Component registry build setup stage.
            Component registry gets evaluated and all code is copied or symlinked to build dir. Additionally, the
            PlatformIO config file is populated with defines, include paths and libraries.

        'REFLECTION': Reflection parsing and metadata generation stage.
            Functionality like the plugin registry, parse C++ code and generate additional wrapper or metadata files.

        'API': Api registry stage.
            With all other code generation and the build set up, the API registry will parse selected files for API
            endpoint declarations and read DTO definitions for endpoint and DTO code generation.
    """

    DEFAULT = 0.0
    COMPONENTS = -10.0
    REFLECTION = -20.0
    ERRORS = -30.0
    API = -40.0




GenerationJob = Callable[[], None]


def add_generation_job(job: GenerationJob):
    CORE.add_job(job)


def build_job_with_priority(priority: GenerationStages):
    def decorator(func):
        coro = coroutine(func)
        coro.priority = priority.value
        return coro

    return decorator


@generated_tbd_global(domain='middlewares')
def add_tbd_build_middleware():
    BUILD_MIDDLEWARE_FILENAME = 'tbd_module_post.py'
    source_file = Path(__file__).parent / BUILD_MIDDLEWARE_FILENAME
    if not source_file.is_file():
        raise ValueError(f'build middleware file {source_file} not found')
    copy_file_if_outdated(source_file, BUILD_MIDDLEWARE_FILENAME)

    extra_script_group = 'extra_scripts'
    post_script = f'pre:{BUILD_MIDDLEWARE_FILENAME}'
    
    if extra_script_group in CORE.platformio_options and post_script in CORE.platformio_options[extra_script_group]:
        return

    CORE.add_platformio_option(extra_script_group, [post_script])


def register_tbd_component(component: ComponentInfo):
    name = component.full_name
    if has_tbd_global(name, domain=COMPONENTS_DOMAIN):
        raise ValueError(f'component {component.full_name} already registered')
    set_tbd_global(name, component, domain=COMPONENTS_DOMAIN)
    

def new_tbd_component(init_file: str, *, auto_include: bool = True, auto_sources: bool = True, add_module_header: bool = True):
    """ Convenience function to create a TBD component.

        arguments:

        :param str init_file: the `__file__` special variable of the component's `__init__.py`

        :param bool auto_include: add default include paths
        :param bool auto_sources: add default source paths

        :return ComponentInfo: the newly created component
    """

    module = ComponentInfo.for_init_file(init_file)

    _LOGGER.info(f'adding TBD module {module.name}: {module.path}')

    if auto_include:
        include_dirs = module.get_default_include_dirs()
        for include_dir in include_dirs:
            module.add_include_dir(include_dir)
            _LOGGER.info(f'additional include dir: {include_dir}')
    
    if auto_sources:
        include_dirs = module.get_default_source_dirs()
        for source_dir in include_dirs:
            module.add_source_dir(source_dir)
            _LOGGER.info(f'additional source dir: {source_dir}')

    module.add_module_header()

    register_tbd_component(module)
    return module


@build_job_with_priority(GenerationStages.ERRORS)
def errors_job():
    collect_errors()


@build_job_with_priority(GenerationStages.COMPONENTS)
def prepare_build_job():
    prepare_build()


def to_code(config):
    new_tbd_component(__file__)

    add_generation_job(prepare_build_job)
    add_generation_job(errors_job)