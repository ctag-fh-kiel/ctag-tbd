""" Main helper component for all TBD esphome components.

    
"""

from pathlib import Path
import logging
from esphome.core import CORE
from dataclasses import dataclass
from functools import lru_cache


_LOGGER = logging.getLogger(__name__)


@lru_cache
def get_components_root():
    return Path(__file__).parent.parent.parent

@lru_cache
def get_tbd_source_root() -> Path:
    return get_components_root().parent

@lru_cache
def get_vendor_root():
    return get_tbd_source_root / 'vendor'

@lru_cache
def get_generated_sources_path() -> Path:
    source_path = Path(CORE.build_path) / 'src' / 'generated'
    source_path.mkdir(parents=True, exist_ok=True)
    CORE.add_platformio_option('build_src_filter', [f'+<{source_path}>'])
    return source_path

@lru_cache
def get_generated_include_path() -> Path:
    include_path = get_generated_sources_path() / 'include'
    include_path.mkdir(parents=True, exist_ok=True)
    CORE.add_build_flag(f'-I{include_path}')
    return include_path


@dataclass(frozen=True)
class ComponentInfo:
    full_name: str  # including tbd prefix if present
    path: Path

    @property
    def name(self) -> str:
        if self.full_name.lower().startswith('tbd'):
            return self.full_name[4:]
        return self.full_name
        
    @property
    def include_dir(self) -> Path | None:
        path = self.path / 'include'
        return path if path.is_dir() else None

    @property
    def common_include_dir(self) -> Path | None:
        path = self.path / 'common' / 'include'
        return path if path.is_dir() else None

    @property
    def platform_include_dir(self) -> Path | None:
        platform = CORE.target_platform
        path = self.path / platform / 'include'
        return path if path.is_dir() else None
    
    @property
    def source_dir(self) -> Path | None:
        path = self.path / 'src'
        return path if path.is_dir() else None
    
    @property
    def platform_source_dir(self) -> Path | None:
        platform = CORE.target_platform
        path = self.path / platform
        return path if path.is_dir() else None

    def add_include_dir(self, path: Path | str, *, if_exists: bool = False) -> bool:
        absolute_path = self.ensure_component_path(path)
        if not absolute_path.is_dir():
            if if_exists:
                return False
            else:
                raise ValueError(f'include path {absolute_path} does not exist')
            
        CORE.add_build_flag(f'-I{absolute_path}')  
        return True

    def add_source_dir(self, path: Path | str, *, if_exists: bool = False) -> bool:
        normalized_path = self.ensure_component_path(path)
        if not normalized_path.is_dir():
            if if_exists:
                return False
            else:
                raise ValueError(f'source path {normalized_path} does not exist')
            
        CORE.add_platformio_option('build_src_filter', [f'+<{normalized_path}/>'])
        return True

    def add_source_file(self, path: Path | str, *, if_exists: bool = False):
        normalized_path = self.ensure_component_path(path)
        if not normalized_path.is_file():
            if if_exists:
                return False
            else:
                raise ValueError(f'source file {normalized_path} does not exist')
        
        CORE.add_platformio_option('build_src_filter', [f'+<{normalized_path}>'])
        return True

    def normalize_path(self, path: Path | str) -> Path:
        path = self.ensure_component_path(path)
        return path.relative_to(get_tbd_source_root())

    def ensure_component_path(self, path: Path | str) -> Path:
        path = Path(path)
        if path.is_absolute():
            if not path.is_relative_to(self.path):
                raise ValueError(f'path {path} is not in module path {self.path}')
            return path
        else:
            return self.path / path

    def get_include_dirs(self) -> list[Path]:
        return [path for path in [
                self.include_dir,
                self.common_include_dir,
                self.platform_include_dir,
            ]
            if path is not None
        ]
    
    def get_source_dirs(self) -> list[Path]:
        return [path for path in [
                self.source_dir,
                self.platform_source_dir
            ]
            if path is not None
        ]

    @staticmethod
    def enable_exceptions():
        """ Enable exceptions on host platform hack.

            :warning: Only available on host platform.

            On the host platform some libraries like rtaudio will simply not work without
            having exceptions enabled, so we trick esphome into enabling them. 

            ..note: 
                exception flag is disabled in esphome coroutine with priority `100.0` and `to_code`
                methods are executed with default priority `0.0`. Make sure to not call this method
                from any prority `>= 100.0`.
        """

        if not CORE.is_host:
            raise ValueError('exceptions can only be disabled on host platform')
        
        if '-fno-exceptions':
            CORE.build_flags.remove('-fno-exceptions')

    @staticmethod
    def for_init_file(init_file: str) -> 'ComponentInfo':
        module_path = Path(init_file).absolute().parent
        full_module_name = module_path.name 
        return ComponentInfo(full_module_name, module_path)
    
    def add_module_header(self):
        gen_include_path = get_generated_include_path() / 'tbd' / self.name
        gen_include_path.mkdir(parents=True, exist_ok=True)
        module_header = gen_include_path / 'module.hpp'

        # tbd_module should not use external python dependencies, therefore this is done on foot
        with open(module_header, 'w') as f:
            f.write('#pragma once\n')
            f.write('namespace {\n')
            f.write(f'    const char* tag = "{self.full_name}";\n')
            f.write('}\n')



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
        include_dirs = module.get_include_dirs()
        for include_dir in include_dirs:
            module.add_include_dir(include_dir)
            _LOGGER.info(f'additional include dir: {include_dir}')
    
    if auto_sources:
        include_dirs = module.get_source_dirs()
        for source_dir in include_dirs:
            module.add_source_dir(source_dir)
            _LOGGER.info(f'additional source dir: {source_dir}')

    module.add_module_header()

    # FIXME: remove this hack once esphome itself has switched from C++ 17 to C++ 20
    CORE.add_build_flag('-std=c++20')

    return module

new_tbd_component(__file__)