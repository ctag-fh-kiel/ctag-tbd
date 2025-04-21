# helpers to convert TBD cmake libraries to esphome modules

from pathlib import Path
import logging
from esphome.core import CORE, EsphomeError
from dataclasses import dataclass


_LOGGER = logging.getLogger(__name__)

TBD_COMPONENTS_ROOT = Path(__file__).parent.parent.parent
TBD_ROOT = TBD_COMPONENTS_ROOT.parent


TBD_VENDOR_ROOT = TBD_ROOT / 'vendor'
TBD_LIBRARY_ROOT = TBD_ROOT / 'tbd'

if CORE.is_esp32:
    TBD_PORT_ROOT = TBD_ROOT / 'ports' / 'tbd_port_esp32'
elif CORE.is_host:
    TBD_PORT_ROOT = TBD_ROOT / 'ports' / 'tbd_port_desktop'
else:
    raise EsphomeError(f'unsupported TBD platform {CORE.target_platform}')


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
        full_path = self.ensure_abolute_path(path)
        if not full_path.is_dir():
            if if_exists:
                return False
            else:
                raise ValueError(f'include path {full_path} does not exist')
            
        CORE.add_build_flag(f'-I{full_path}')  
        return True

    def add_source_dir(self, path: Path | str, *, if_exists: bool = False) -> bool:
        full_path = self.ensure_abolute_path(path)
        if not full_path.is_dir():
            if if_exists:
                return False
            else:
                raise ValueError(f'source path {full_path} does not exist')
            
        CORE.add_platformio_option('build_src_filter', [f'+<{full_path}>/**/*.cpp'])
        return True

    def add_source_file(self, path: Path | str, *, if_exists: bool = False):
        full_path = self.ensure_abolute_path(path)
        if not full_path.is_file():
            if if_exists:
                return False
            else:
                raise ValueError(f'source file {full_path} does not exist')
        
        CORE.add_platformio_option('build_src_filter', [f'+<{full_path}>'])
        return True

    def ensure_abolute_path(self, path: Path | str) -> Path:
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
    def for_init_file(init_file: str) -> 'ComponentInfo':
        module_path = Path(init_file).absolute().parent
        full_module_name = module_path.name 
        return ComponentInfo(full_module_name, module_path)
    

def get_tbd_source_root() -> Path:
    return Path(__file__).parent.parent.parent.parent



def new_tbd_component(init_file: str, *, auto_include=True, auto_sources=True):
    """ helper to easily create a new TBD module
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

    return module
