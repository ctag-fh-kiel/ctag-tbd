import logging

from . import get_generated_include_path, get_messages_path
from .registry import get_tbd_domain
from .files import get_components_build_path, copy_file_if_outdated, copy_tree_if_outdated
from .component_info import ComponentInfo, COMPONENTS_DOMAIN
from .build_generator import (
    set_compiler_options,
    add_define,
    add_include_dir,
    write_build_config,
)

_LOGGER = logging.getLogger(__name__)

SOURCE_EXTENSIONS = ['cpp', 'hpp', 'cc', 'hh', 'c', 'h', 'inl']
SOURCE_PATTERNS = [f'*.{extension}' for extension in SOURCE_EXTENSIONS]


def prepare_build():
    """ Prepare all modules for build.

        Copies or links source and headers files, sets up defines and include directories.
    """
    components = [component for component in get_tbd_domain(COMPONENTS_DOMAIN).values()]

    if any(component.needs_exceptions for component in components):
        set_compiler_options(exceptions=True)
    else:
        set_compiler_options()

    _LOGGER.info('adding global TBD defines')

    add_include_dir(get_generated_include_path())
    add_include_dir(get_messages_path())

    for module in get_tbd_domain(COMPONENTS_DOMAIN).values():
        if not isinstance(module, ComponentInfo):
            raise ValueError(f'bad module type in TBD modules list {type(module)}')

        for key, value in module.defines.items():
            _LOGGER.info(f'>>> {key} {value}')
            add_define(key, value)

        component_build_dir = get_components_build_path() / module.full_name
        for path in module.include_dirs:
            add_include_dir(component_build_dir / path)

        sources = {*module.include_dirs, *module.sources}
        for path in sources:
            dest_path = component_build_dir / path
            source_path = module.path / path
            if source_path.is_file():
                copy_file_if_outdated(source_path, dest_path)
            elif source_path.is_dir():
                copy_tree_if_outdated(source_path, dest_path, patterns=SOURCE_PATTERNS)
            else:
                raise ValueError(f'unknown source path type {path}')


def finalize_build():
    """ Final steps when the entire build is set up. """
    write_build_config()


__all__ = [
    'SOURCE_EXTENSIONS',
    'SOURCE_PATTERNS',
    'prepare_build',
    'finalize_build',
]
