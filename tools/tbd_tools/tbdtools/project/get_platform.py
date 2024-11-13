import os
from typing import Optional
import re

from loguru import logger

from .get_project import find_project_root
from .project_structure import Platform


PLATFORM_ENVVAR = 'TBD_PLATFORM'


def _platform_from_string(config_str: str) -> Platform:
    for item in Platform:
        if item.name == config_str:
            return item
    raise ValueError(f'unknown platform {config_str}')

_config_platform_expr = re.compile(r'config_tbd_platform_(?P<platform>\w+)\s*=\s*(?P<value>\w+)')


def _get_platform_from_config_file() -> Optional[Platform]:
    project_root = find_project_root()    
    config_file = project_root / 'sdkconfig'

    with open(config_file, 'r') as f:
        active_platforms = [match.group('platform') for line in f 
                            if (match :=_config_platform_expr.match(line.lower())) and match.group('value') == 'y']
    active_platforms = [_platform_from_string(platform) for platform in active_platforms]

    if len(active_platforms) > 1:
        raise ValueError(f'multiple active platforms in {config_file}: {[p.value for p in active_platforms]}')

    if len(active_platforms) < 1:
        logger.debug(f'no TBD platform active in {config_file}')
        return None
    
    return active_platforms[0]


def _get_platform_from_env() -> Optional[Platform]:
    if (platform_string := os.getenv(PLATFORM_ENVVAR)) is not None:
        return _platform_from_string(platform_string)
    
    logger.debug(f'{PLATFORM_ENVVAR} not set')
    return None

def get_platform() -> Platform:
    if (platform := _get_platform_from_env()) is not None:
        return platform


    if (platform := _get_platform_from_config_file()) is not None:
        return platform
    
    logger.warning('no TBD platform set, defaulting to "desktop"')
    return Platform.desktop


__all__ = ['PLATFORM_ENVVAR', 'get_platform']
