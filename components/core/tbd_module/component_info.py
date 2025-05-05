from pathlib import Path
from esphome.core import CORE
from dataclasses import dataclass, field

from .files import get_tbd_source_root, get_generated_include_path

@dataclass(frozen=True)
class ComponentInfo:
    full_name: str  # including tbd prefix if present
    path: Path
    defines: dict[str, str] = field(default_factory=dict)
    _includes: list[Path] = field(default_factory=list)
    _sources: list[Path] = field(default_factory=list)

    @property
    def name(self) -> str:
        if self.full_name.lower().startswith('tbd'):
            return self.full_name[4:]
        return self.full_name
        
    @property
    def include_dir(self) -> Path | None:
        """ contains headers to be included on all build targets """
        path = self.path / 'include'
        return path if path.is_dir() else None

    @property
    def common_include_dir(self) -> Path | None:
        """ :depricated: contains headers to be included on all build targets """
        path = self.path / 'common' / 'include'
        return path if path.is_dir() else None

    @property
    def platform_include_dir(self) -> Path | None:
        """ contains headers for this specific architecture (host, esp32, ...) """
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

    @property
    def include_dirs(self) -> list[Path]:
        return [dir.relative_to(self.path) for dir in self._includes]
    
    @property
    def sources(self) -> list[Path]:
        return [dir.relative_to(self.path) for dir in self._sources]

    def add_define(self, key: str, value = 1) -> None:
        self.defines[key] = value

    def add_include_dir(self, path: Path | str, *, if_exists: bool = False) -> bool:
        absolute_path = self.ensure_component_path(path)
        if not absolute_path.is_dir():
            if if_exists:
                return False
            else:
                raise ValueError(f'include dir {absolute_path} does not exist')
        self._includes.append(absolute_path)
        return True

    def add_source_dir(self, path: Path | str, *, if_exists: bool = False) -> bool:
        normalized_path = self.ensure_component_path(path)
        if not normalized_path.is_dir():
            if if_exists:
                return False
            else:
                raise ValueError(f'source dir {normalized_path} does not exist')
            
        self._sources.append(normalized_path)
        return True

    def add_source_file(self, path: Path | str, *, if_exists: bool = False) -> bool:
        normalized_path = self.ensure_component_path(path)
        if not normalized_path.is_file():
            if if_exists:
                return False
            else:
                raise ValueError(f'source file {normalized_path} does not exist')
        self._sources.append(normalized_path)
        return True
        
    # def add_ext_source_dir(self, path: Path | str, *, category: str = 'libs', if_exists: bool = False) -> bool:
    #     path = Path(path)
    #     if not path.is_absolute():
    #         raise ValueError('external source files need to be specified with absolute paths')
        
    #     if path.is_dir():
    #         if if_exists:
    #             return False
    #         else:
    #             raise ValueError(f'external source dir {path} does not exist')
            
    #     self._ext_sources.append(path)
    #     return True

    # def add_ext_source_file(self, path: Path | str, *, if_exists: bool = False) -> bool:
    #     path = Path(path)
    #     if not path.is_absolute():
    #         raise ValueError('external source files need to be specified with absolute paths')
        
    #     if not path.is_file():
    #         if if_exists:
    #             return False
    #         else:
    #             raise ValueError(f'external source file {path} does not exist')

    #     self._ext_sources.append(path)
    #     return True

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

    def get_default_include_dirs(self) -> list[Path]:
        return [path for path in [
                self.include_dir,
                self.common_include_dir,
                self.platform_include_dir,
            ]
            if path is not None
        ]
    
    def get_default_source_dirs(self) -> list[Path]:
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
    
    def __hash__(self):
        return hash(self.full_name)
    
    def __repr__(self):
        return f'Component({self.full_name})'

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
            f.write(f'namespace tbd::{self.name} {'{'}\n')
            f.write(f'    constexpr const char* tag = "{self.full_name}";\n')
            f.write('}\n')

__all__ = ['ComponentInfo']