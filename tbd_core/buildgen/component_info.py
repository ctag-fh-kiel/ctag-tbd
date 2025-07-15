import logging
from enum import unique, StrEnum
from pathlib import Path
from dataclasses import dataclass, field
from typing import OrderedDict

from .registry import has_tbd_global, set_tbd_global, get_tbd_domain, get_tbd_global
from .build_generator import get_target_platform, DefineValue
from .files import get_tbd_source_root, get_generated_include_path, get_build_path
from tbd_core.buildgen.build_deps import ExternalDependency, SystemLibrary, ExternalLibrary

_LOGGER = logging.getLogger(__name__)


COMPONENTS_DOMAIN = 'components'


@unique
class AutoReflection(StrEnum):
    OFF = 'OFF'
    HEADERS = 'HEADERS'
    SOURCES = 'SOURCES'
    ALL = 'ALL'


@dataclass(frozen=True)
class ComponentInfo:
    full_name: str  # including tbd prefix if present
    path: Path
    reflect: AutoReflection
    needs_exceptions: bool = False

    _defines: dict[str, DefineValue] = field(default_factory=dict)
    _includes: list[Path] = field(default_factory=list)
    _sources: list[Path] = field(default_factory=list)
    _external_dependencies: list[ExternalDependency] = field(default_factory=list)

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
        """ :deprecated: contains headers to be included on all build targets """
        path = self.path / 'common' / 'include'
        return path if path.is_dir() else None

    @property
    def platform_include_dir(self) -> Path | None:
        """ contains headers for this specific architecture (host, esp32, ...) """
        platform = get_target_platform()
        path = self.path / platform / 'include'
        return path if path.is_dir() else None
    
    @property
    def source_dir(self) -> Path | None:
        path = self.path / 'src'
        return path if path.is_dir() else None

    @property
    def esphome_dir(self) -> Path | None:
        path = self.path / 'esphome'
        return path if path.is_dir() else None

    @property
    def platform_source_dir(self) -> Path | None:
        platform = get_target_platform()
        path = self.path / platform
        return path if path.is_dir() else None

    @property
    def tests_dir(self) -> Path | None:
        path = self.path / 'test'
        return path if path.is_dir() else None

    @property
    def include_dirs(self) -> list[Path]:
        return [_dir.relative_to(self.path) for _dir in self._includes]
    
    @property
    def sources(self) -> list[Path]:
        return [_dir.relative_to(self.path) for _dir in self._sources]

    @property
    def defines(self) -> dict[str, str]:
        return self._defines

    def add_define(self, key: str, value: DefineValue = 1) -> None:
        self._defines[key] = value

    @property
    def external_dependencies(self) -> list[ExternalDependency]:
        return self._external_dependencies

    def add_external_library(self, name: str, version: str | None = None, repository: str | None = None) -> None:
        self._external_dependencies.append(ExternalLibrary(
            name=name,
            version=version,
            repository=repository,
        ))

    def add_system_library(self, name: str) -> None:
        self._external_dependencies.append(SystemLibrary(name=name))

    def add_dependency(self, lib: ExternalDependency) -> None:
        self._external_dependencies.append(lib)

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
                self.esphome_dir,
            ]
            if path is not None
        ]
    
    def get_default_source_dirs(self) -> list[Path]:
        return [path for path in [
                self.source_dir,
                self.platform_source_dir,
                self.esphome_dir,
            ]
            if path is not None
        ]
    
    def __hash__(self):
        return hash(self.full_name)
    
    def __repr__(self):
        return f'Component({self.full_name})'
    
    def add_module_header(self):
        gen_include_path = get_build_path() / get_generated_include_path() / 'tbd' / self.name
        gen_include_path.mkdir(parents=True, exist_ok=True)
        module_header = gen_include_path / 'module.hpp'

        # tbd_module should not use external python dependencies, therefore this is done on foot
        with open(module_header, 'w') as f:
            f.write('#pragma once\n')
            f.write(f'namespace tbd::{self.name} {'{'}\n')
            f.write(f'    constexpr const char* tag = "{self.full_name}";\n')
            f.write('}\n')


def get_or_create(
            init_file: str, *,
            reflect: AutoReflection,
            needs_exceptions,
    ) -> tuple[bool, 'ComponentInfo']:

        module_path = Path(init_file).absolute().parent
        full_module_name = module_path.name
        if has_tbd_global(full_module_name, domain=COMPONENTS_DOMAIN):
            return False, get_tbd_global(full_module_name, domain=COMPONENTS_DOMAIN)

        return True, ComponentInfo(
            full_name=full_module_name,
            path=module_path,
            reflect=reflect,
            needs_exceptions=needs_exceptions,
        )


def register_tbd_component(component: ComponentInfo):
    name = component.full_name
    if has_tbd_global(name, domain=COMPONENTS_DOMAIN):
        raise ValueError(f'component {component.full_name} already registered')
    set_tbd_global(name, component, domain=COMPONENTS_DOMAIN)


def get_tbd_components() -> OrderedDict[str, ComponentInfo]:
    return OrderedDict((name, info) for name, info in get_tbd_domain(COMPONENTS_DOMAIN).items())


def new_tbd_component(
        init_file: str, *,
        auto_include: bool = True,
        auto_sources: bool = True,
        auto_reflect: AutoReflection = AutoReflection.OFF,
        needs_exceptions=False,
) -> ComponentInfo:
    """ Convenience function to create a TBD component.

        arguments:

        :param str init_file: the `__file__` special variable of the component's `__init__.py`

        :param bool auto_include: add default include paths
        :param bool auto_sources: add default source paths
        :param bool auto_reflect: reflect all headers by default
        :param bool needs_exceptions: this module needs exceptions enabled (host platform only)

        :return ComponentInfo: the newly created component
    """

    is_new, component = get_or_create(init_file, reflect=auto_reflect, needs_exceptions=needs_exceptions)
    if not is_new:
        return component

    _LOGGER.info(f'adding TBD component {component.name}: {component.path}')

    if auto_include:
        include_dirs = component.get_default_include_dirs()
        for include_dir in include_dirs:
            component.add_include_dir(include_dir)
            _LOGGER.info(f'additional include dir: {include_dir}')

    if auto_sources:
        include_dirs = component.get_default_source_dirs()
        for source_dir in include_dirs:
            component.add_source_dir(source_dir)
            _LOGGER.info(f'additional source dir: {source_dir}')

    component.add_module_header()

    register_tbd_component(component)
    return component


__all__ = [
    'AutoReflection',
    'ExternalDependency',
    'ComponentInfo',
    'get_tbd_components',
    'new_tbd_component',
]