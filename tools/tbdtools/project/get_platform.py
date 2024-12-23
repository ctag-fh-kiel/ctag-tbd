import os
from pathlib import Path
from typing import Optional
from loguru import logger
from .get_project import get_project

PLATFORM_ENVVAR = 'TBD_PLATFORM'


def _check_platform(platform: str) -> None:
    dirs = get_project()
    platform_configs: Path = dirs.config.platforms() / f'platform.{platform}.json'
    if not platform_configs.is_file():
        raise ValueError(f'platform {platform} is unknown')


def _get_platform_from_env() -> Optional[str]:
    if (platform_string := os.getenv(PLATFORM_ENVVAR)) is not None:
        _check_platform(platform_string)
        return platform_string

    logger.debug(f'{PLATFORM_ENVVAR} not set')
    return None

_platform: Optional[str] = None

def get_platform() -> str:
    if _platform is None:
        raise ValueError('platform has not been set')
    return _platform

def init_platform(platform: str) -> None:
    global _platform
    _check_platform(platform)

    _platform = platform


__all__ = ['PLATFORM_ENVVAR', 'get_platform', 'init_platform']
