from typing import Union

from pydantic.dataclasses import dataclass
from pydantic import Field


LIBRARY_SCHEMA = 'https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/schema/library.json'


@dataclass(frozen=True)
class BuildCommand:
    directory: str
    command: str
    file: str
    output: str | None = None


@dataclass(frozen=True)
class Build:
    flags: list[str]    
    srcFilter: list[str]


@dataclass(frozen=True)
class LibraryJson:
    name: str
    version: str
    build: Build
    schema: str = Field(serialization_alias='$schema', default=LIBRARY_SCHEMA)


@dataclass(frozen=True)
class ExternalLibrary:
    name: str
    version: str | None = None
    repository: str | None = None
    build: Build | None = None

@dataclass(frozen=True)
class SystemLibrary:
    name: str

ExternalDependency = Union[SystemLibrary, ExternalLibrary]

__all__ = [
    'BuildCommand',
    'Build',
    'ExternalDependency',
    'ExternalLibrary',
    'SystemLibrary',
    'LibraryJson',
]
