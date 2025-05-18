from esphome.core import CORE
import logging
from .registry import get_tbd_domain
from .files import get_components_build_path, copy_file_if_outdated, copy_tree_if_outdated
from .component_info import ComponentInfo

_LOGGER = logging.getLogger(__name__)


COMPONENTS_DOMAIN = 'components'
SOURCE_EXTENSIONS = ['cpp', 'hpp', 'cc', 'hh', 'c', 'h', 'inl']
SOURCE_PATTERNS = [f'*.{extension}' for extension in SOURCE_EXTENSIONS]

def prepare_build():
    EXCEPTION_FLAG = '-fno-exceptions'
    CPP_STD_FLAG = '-std=c++20'
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

__all__ = [
    'COMPONENTS_DOMAIN',
    'SOURCE_EXTENSIONS',
    'SOURCE_PATTERNS',
    'prepare_build'
]
