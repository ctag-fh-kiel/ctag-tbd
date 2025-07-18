from abc import ABC, abstractmethod
from dataclasses import dataclass
from enum import unique, StrEnum
from pathlib import Path
from typing import Callable, Awaitable

from tbd_core.buildgen.build_deps import ExternalDependency
from .generation_stages import GenerationStages
from .registry import set_tbd_global, has_tbd_global, get_tbd_global, _set_generation_stage

BUILD_GENERATOR_GLOBAL = 'build_generator'

DefineValue = bool | float | int | str | None


GenerationJobFunction = Callable[[], None]
GenerationJob = Callable[[], Awaitable[None]]


@unique
class TargetPlatform(StrEnum):
    HOST  = 'host'
    ESP32 = 'esp32'


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
    def target_platform(self) -> TargetPlatform:
        raise NotImplementedError()

    @property
    @abstractmethod
    def board(self) -> str:
        raise NotImplementedError()

    def add_define(self, key: str, value: DefineValue = None) -> None:
        if value is None:
            self.add_build_flag(f'-D{key}')

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
    def add_library(self, lib: ExternalDependency) -> None:
        raise NotImplementedError()

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


def get_target_platform() -> TargetPlatform:
    return get_build_generator().target_platform


def is_esp32() -> bool:
    return get_target_platform() == TargetPlatform.ESP32


def is_host() -> bool:
    return get_target_platform() == TargetPlatform.HOST


def get_board() -> str:
    return get_build_generator().board

def add_define(key: str, value: DefineValue) -> None:
    get_build_generator().add_define(key, value)


def add_build_flag(flag: str) -> None:
    get_build_generator().add_build_flag(flag)


def add_include_dir(include_dir: Path) -> None:
    get_build_generator().add_include_dir(include_dir)


def add_library(lib: ExternalDependency) -> None:
    get_build_generator().add_library(lib)


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
        def stage_wrapper() -> None:
            _set_generation_stage(priority)
            func()

        job = get_build_generator().function_to_job(stage_wrapper)
        job.priority = priority.value
        return job

    return decorator


__all__ = [
    'DefineValue',
    'GenerationJob',
    'GenerationJobFunction',
    'TargetPlatform',
    'BuildGenerator',
    'write_build_config',
    'get_build_generator',
    'set_build_generator',
    'get_target_platform',
    'is_esp32',
    'is_host',
    'get_board',
    'add_define',
    'add_build_flag',
    'add_include_dir',
    'add_library',
    'set_compiler_options',
    'add_platformio_option',
    'add_platformio_block',
    'add_generation_job',
    'build_job_with_priority',
]
