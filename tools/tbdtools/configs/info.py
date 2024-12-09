from typing import Literal
from pydantic.dataclasses import dataclass
from enum import Enum


@dataclass
class PlatformConfigInfo:
    name: str
    full_name: str
    description: str
    mark: int
    rev: int


@dataclass
class PlatformConfigConfig:
    system: Literal['esp32', 'desktop']
    arch: Literal['esp32', 'esp32s3', 'desktop']


@dataclass
class PlatformConfig:
    config_version: int
    info: PlatformConfigInfo
