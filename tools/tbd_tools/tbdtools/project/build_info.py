from dataclasses import dataclass
from enum import Enum, unique
import os
from datetime import datetime
from pathlib import Path
import re
from typing import Tuple

import jinja2 as ji

from .tbd_project import get_project_repo, find_project_root


@unique
class Platform(Enum):
    V1   = 'TBD mk 1 rev 1'
    V2   = 'TBD mk 1 rev 2'
    STR  = 'Strämpler'
    AEM  = 'Tangible Waves AEM'
    MK2  = 'TBD mk 2'
    BBA  = 'TBD BBA'


def _platform_from_config_string(config_str: str) -> Platform:
    for item in Platform:
        if item.name == config_str:
            return item.value
    raise ValueError(f'unknown platform {config_str}')


@dataclass
class BuildInfo:
    hardware: str
    firmware: str
    commit: str
    post_commit_changes: bool
    ahead_of_release: int
    build_date: int


@dataclass
class Version:
    tag: str
    commit: str
    post_commit_changes: bool
    ahead_of_release: int


def get_version_info() -> Version:
    git_repo = get_project_repo()
    commit = git_repo.head.commit
    is_dirty = git_repo.is_dirty()

    tags = {tag.commit: tag.name for tag in git_repo.tags}
    for i, prev_commit in enumerate(git_repo.iter_commits()):
        if tag := tags.get(prev_commit):
            return Version(tag, commit.hexsha, is_dirty, i)
    return Version('unknown', commit.hexsha, is_dirty, -1)

_config_platform_expr = re.compile(r'CONFIG_TBD_PLATFORM_(?P<platform>\w+)\s*=\s*(?P<value>\w+)')


def get_hardware_version() -> str:
    project_root = find_project_root()    
    with open(project_root / 'sdkconfig', 'r') as f:
        active_platforms = [match.group('platform') for line in f 
                            if (match :=_config_platform_expr.match(line)) and match.group('value') == 'y']
    active_platforms = [_platform_from_config_string(platform) for platform in active_platforms]

    if len(active_platforms) == 1:
        return active_platforms[0]
    raise ValueError(f'could not determine platform, found {active_platforms} in config')


def get_build_info() -> BuildInfo:
    version = get_version_info()
    hardware = get_hardware_version()
    build_date = datetime.now().replace(microsecond=0).isoformat()
    return BuildInfo(
        hardware=hardware, 
        firmware=version.tag, 
        commit=version.commit, 
        post_commit_changes=version.post_commit_changes,
        ahead_of_release=version.ahead_of_release, 
        build_date=build_date
    )


def write_build_info_header(build_info_header_path: Path):
    template_path = Path(__file__).parent / 'templates'
    env = ji.Environment(loader=ji.FileSystemLoader(template_path), autoescape=ji.select_autoescape())    

    build_info = get_build_info()

    template = env.get_template('build_info_header.jinja.hpp')
    header = template.render(build_info=build_info)
    build_info_header_path.parent.mkdir(parents=True, exist_ok=True)
    print(build_info_header_path.parent)
    with open(build_info_header_path, 'w') as f:
        f.write(header)    


__all__ = ['get_version_info', 'get_hardware_version', 'get_build_info', 'write_build_info_header']
