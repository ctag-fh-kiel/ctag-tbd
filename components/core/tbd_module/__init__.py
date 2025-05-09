""" Main helper component for all TBD esphome components.

    
"""

from enum import Enum, unique
from pathlib import Path
import logging
from typing import Callable
from esphome.core import CORE
import shutil
from esphome.coroutine import coroutine

from .registry import *
from .files import *
from .component_info import *

AUTO_LOAD = ['tbd_system']


_LOGGER = logging.getLogger(__name__)

def is_esp32():
    return CORE.is_esp32

def is_desktop():
    return CORE.is_host

# build job utils

@unique
class GenerationStages(Enum):
    COMPONENTS = -10.0
    PLUGINS = -20.0
    API = -30.0

COMPONENTS_DOMAIN = 'components'


GenerationJob = Callable[[], None]


def add_generation_job(job: GenerationJob):
    CORE.add_job((job))


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


@build_job_with_priority(GenerationStages.COMPONENTS)
def to_code(config):
    new_tbd_component(__file__)    

    EXCEPTION_FLAG = '-fno-exceptions'
    CPP_STD_FLAG='-std=c++20'
    build_flags = [flag for flag in CORE.build_flags if not (flag.startswith('-std=') or flag == EXCEPTION_FLAG)]
    CORE.build_flags = set(build_flags)
    CORE.add_build_flag(CPP_STD_FLAG)

    _LOGGER.info('adding global TBD defines')
    for module in get_tbd_domain(COMPONENTS_DOMAIN).values():
        if not isinstance(module, ComponentInfo):
            raise ValueError(f'bad module type in TBD modules list {type(module)}')

        for key, value in module.defines.items():
            _LOGGER.info(f'>>> {key} {value}')
            CORE.add_build_flag(f'-D{key}={value}')
        
        component_build_dir = get_components_build_path() / module.full_name
        for path in module.include_dirs:
            CORE.add_build_flag(f'-I{component_build_dir / path}')

        sources = set([*module.include_dirs, *module.sources])
        for path in sources:
            dest_path = component_build_dir / path
            source_path = module.path / path
            if source_path.is_file():
                copy_file_if_outdated(source_path, dest_path)
            elif source_path.is_dir():
                copy_tree_if_outdated(source_path, dest_path, patterns=['*.cpp', '*.hpp', '*.cc', '*.hh', '*.c', '*.h', '*.inl'])
            else:
                raise ValueError(f'unknown source path type {path}')

