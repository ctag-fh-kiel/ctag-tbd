from abc import ABC, abstractmethod
from enum import unique, Enum
from pathlib import Path
from typing import Callable, Awaitable

from .registry import set_tbd_global, has_tbd_global, get_tbd_global


BUILD_GENERATOR_GLOBAL = 'build_generator'

DefineValue = bool | float | int | str


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

    DEFAULT    =  0.0
    COMPONENTS = -10.0
    TESTS      = -20.0
    REFLECTION = -30.0
    ERRORS     = -40.0
    API        = -50.0
    FINALIZE   = -60.0


GenerationJobFunction = Callable[[], None]
GenerationJob = Callable[[], Awaitable[None]]


class BuildGenerator(ABC):
    def __init__(self):
        super().__init__()
        self._platformio_raw: list[str] = []

    @abstractmethod
    def write_config(self):
        raise NotImplementedError()

    @property
    @abstractmethod
    def build_path(self) -> Path:
        raise NotImplementedError()

    @property
    @abstractmethod
    def target_platform(self) -> str:
        raise NotImplementedError()

    def add_define(self, key: str, value: DefineValue) -> None:
        match value:
            case bool():
                _value = 0 if not value else 1
            case float():
                _value = f'{value}f'
            case int():
                _value = value
            case str():
                _value = value
            case _:
                raise ValueError('defines must be bool or int or str')
        self.add_build_flag(f'-D{key}={_value}')

    @abstractmethod
    def add_build_flag(self, flag: str) -> None:
        raise NotImplementedError()

    def add_include_dir(self, include_dir: Path) -> None:
        self.add_build_flag(f'-I{include_dir}')

    @abstractmethod
    def set_compiler_options(self, *, std: int = 20, exceptions: bool = False) -> None:
        raise NotImplementedError()

    @abstractmethod
    def add_platformio_option(self, key: str, value: str | list[str]) -> None:
        raise NotImplementedError()

    def add_platformio_block(self, block: list[str] | str) -> None:
        if isinstance(block, str):
            self._platformio_raw.append(block)
        elif isinstance(block, list):
            self._platformio_raw.extend(block)
        else:
            raise ValueError('platformio blocks must be str or list')

    @abstractmethod
    def add_job(self, job):
        raise NotImplementedError()

    @abstractmethod
    def function_to_job(self, func):
        raise NotImplementedError()


def get_build_generator() -> BuildGenerator:
    if not has_tbd_global(BUILD_GENERATOR_GLOBAL):
        raise RuntimeError('no build generator set, did you forget to include esphome.components.tbd_module?')
    return get_tbd_global(BUILD_GENERATOR_GLOBAL)


def set_build_generator(build_generator: BuildGenerator) -> None:
    if has_tbd_global(BUILD_GENERATOR_GLOBAL):
        return
    set_tbd_global(BUILD_GENERATOR_GLOBAL, build_generator)


def write_build_config():
    get_build_generator().write_config()


def get_target_platform() -> str:
    return get_build_generator().target_platform


def add_define(key: str, value: DefineValue) -> None:
    get_build_generator().add_define(key, value)


def add_build_flag(flag: str) -> None:
    get_build_generator().add_build_flag(flag)


def add_include_dir(include_dir: Path) -> None:
    get_build_generator().add_include_dir(include_dir)


def set_compiler_options(*, std: int = 20, exceptions: bool = False) -> None:
    get_build_generator().set_compiler_options(std=std, exceptions=exceptions)


def add_platformio_option(key: str, value: str | list[str]) -> None:
    get_build_generator().add_platformio_option(key, value)


def add_platformio_block(block: list[str] | str) -> None:
    get_build_generator().add_platformio_block(block)


def add_generation_job(job: GenerationJob):
    get_build_generator().add_job(job)


def build_job_with_priority(priority: GenerationStages) -> Callable[[GenerationJobFunction], GenerationJob]:
    def decorator(func: GenerationJobFunction) -> GenerationJob:
        job = get_build_generator().function_to_job(func)
        job.priority = priority.value
        return job

    return decorator


__all__ = [
    'DefineValue',
    'GenerationStages',
    'GenerationJob',
    'GenerationJobFunction',
    'BuildGenerator',
    'write_build_config',
    'get_build_generator',
    'set_build_generator',
    'get_target_platform',
    'add_define',
    'add_build_flag',
    'add_include_dir',
    'set_compiler_options',
    'add_platformio_option',
    'add_platformio_block',
    'add_generation_job',
    'build_job_with_priority',
]