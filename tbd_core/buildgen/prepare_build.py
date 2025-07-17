import logging

import tbd_core.utils as utils
from .build_generator import add_build_flag

from .files import (
    get_components_build_path,
    update_build_file_if_outdated,
    update_build_tree_if_outdated,
    get_generated_include_path,
)
from .component_info import ComponentInfo, get_tbd_components, ExternalDependency
from .build_generator import (
    set_compiler_options,
    add_define,
    add_include_dir,
    write_build_config,
    add_library,
)

_LOGGER = logging.getLogger(__name__)

def prepare_build():
    """ Prepare all modules for build.

        Copies or links source and headers files, sets up defines and include directories.
    """
    components = get_tbd_components()

    if any(component.needs_exceptions for component in components.values()):
        set_compiler_options(exceptions=True)
    else:
        set_compiler_options()

    add_build_flag('-Wno-attributes=tbd::')

    _LOGGER.info('adding global TBD defines')

    add_include_dir(get_generated_include_path())

    external_dependencies: list[ExternalDependency] = []

    for component in components.values():
        if not isinstance(component, ComponentInfo):
            raise ValueError(f'bad component type in TBD modules list {type(component)}')

        for key, value in component.defines.items():
            _LOGGER.info(f'>>> {key} {value}')
            add_define(key, value)

        component_build_dir = get_components_build_path() / component.full_name
        for path in component.include_dirs:
            add_include_dir(component_build_dir / path)

        sources = {*component.include_dirs, *component.sources}
        for path in sources:
            dest_path = component_build_dir / path
            source_path = component.path / path
            if source_path.is_file():
                update_build_file_if_outdated(source_path, dest_path)
            elif source_path.is_dir():
                update_build_tree_if_outdated(source_path, dest_path, patterns=utils.cpp_patterns())
            else:
                raise ValueError(f'unknown source path type {path}')

        external_dependencies += component.external_dependencies

    for external_dependency in external_dependencies:
        add_library(external_dependency)

def finalize_build():
    """ Final steps when the entire build is set up. """
    write_build_config()


__all__ = [
    'prepare_build',
    'finalize_build',
]
