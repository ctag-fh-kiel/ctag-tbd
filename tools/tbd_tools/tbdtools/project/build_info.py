import collections
from dataclasses import asdict
from pydantic.dataclasses import dataclass
import json
from datetime import datetime
from pathlib import Path
from typing import Dict, Optional

from git import List
import jinja2 as ji
from pydantic import ConfigDict, Field
from loguru import logger

from .get_project import get_project_repo
from .project_structure import Platform, get_platform_description
from .get_platform import get_platform


def _get_template(template_name: str):
    template_path = Path(__file__).parent / 'templates'
    env = ji.Environment(loader=ji.FileSystemLoader(template_path), autoescape=ji.select_autoescape())    
    return env.get_template(template_name)


@dataclass
class BuildInfo:
    hardware: str
    firmware: str
    device_capabilities: str
    commit: str
    post_commit_changes: bool
    ahead_of_release: int
    build_date: str


@dataclass
class Version:
    tag: str
    commit: str
    post_commit_changes: bool
    ahead_of_release: int


@dataclass(config=ConfigDict(populate_by_name=True))
class DeviceInputs:
    binary: List[str] = Field(alias='t')
    analog: List[str] = Field(alias='cv')


@dataclass(config=ConfigDict(populate_by_name=True))
class DeviceHeader:
    hardware_type: str = Field(alias='HWV')
    firmware_version: str = Field(alias='FWV')
    hardware_id: str = Field(alias='p')


@dataclass
class DeviceCapabilities(DeviceHeader, DeviceInputs):
    @staticmethod
    def from_header_and_inputs(header: DeviceHeader, inputs: DeviceInputs) -> 'DeviceCapabilities':
        header_dict = asdict(header)
        inputs_dict = asdict(inputs)
        return DeviceCapabilities(**header_dict, **inputs_dict)


def _get_device_inputs(platform: Platform) -> Dict:
    if platform == Platform.v1 or platform == Platform.v2:
        inputs_type = 'mk1'
    elif platform == Platform.str:
        inputs_type = 'str'
    elif platform == Platform.aem:
        raise ValueError(f'aem platform not yet supported {platform.name}')
    elif platform == Platform.mk2:
        inputs_type = 'mk2'
    elif platform == Platform.bba1:
        inputs_type = 'bba1'
    elif platform == Platform.bba2:
        inputs_type = 'bba2'
    elif platform == Platform.desktop:
        raise ValueError(f'desktop platform not yet supported {platform.name}')
    else:
        raise ValueError(f'unsupported platform {platform.name}')
    
    inputs_file = Path(__file__).parent / 'resources' / f'io_capabilities.{inputs_type}.json'
    with open(inputs_file, 'r') as f:
        return json.load(f)


def _get_device_capabilities(platform: Platform, firmware_version: Version) -> DeviceCapabilities:   
    inputs_dict = _get_device_inputs(platform)
    header = DeviceHeader(
        hardware_type=get_platform_description(platform), 
        firmware_version=firmware_version.tag,
        hardware_id=platform.name,
    )
    inputs = DeviceInputs(**inputs_dict)
    return DeviceCapabilities.from_header_and_inputs(header, inputs)


def _get_device_capabilities_str(
        platform: Platform, firmware_version: Version, *, readable: bool = False
    ):

    capabilities = _get_device_capabilities(platform, firmware_version)

    # NOTE: the following JSON key sorting can be removed in future versions of the software
    # for max compatibility the JSON is reordered to precisely match the legacy implementation
    capabilities_dict = collections.OrderedDict([
        ('HWV', capabilities.hardware_type),
        ('FWV', capabilities.firmware_version),
        ('p', capabilities.hardware_id),
        ('t', capabilities.binary),
        ('cv', capabilities.analog)
    ])

    if readable:
        return json.dumps(capabilities_dict, indent=2)
    else:
        return json.dumps(capabilities_dict)


def get_version_info() -> Version:
    git_repo = get_project_repo()
    commit = git_repo.head.commit
    is_dirty = git_repo.is_dirty()

    tags = {tag.commit: tag.name for tag in git_repo.tags}
    for i, prev_commit in enumerate(git_repo.iter_commits()):
        if tag := tags.get(prev_commit):
            return Version(tag, commit.hexsha, is_dirty, i)
    return Version('unknown', commit.hexsha, is_dirty, -1)


def get_build_info(*, platform: Optional[Platform] = None) -> BuildInfo:
    version = get_version_info()
    if platform is None:
        platform = get_platform()
    device_capabilities = _get_device_capabilities_str(platform, version)

    build_date = datetime.now().replace(microsecond=0).isoformat()
    return BuildInfo(
        hardware=platform.name, 
        firmware=version.tag, 
        commit=version.commit, 
        post_commit_changes=version.post_commit_changes,
        ahead_of_release=version.ahead_of_release, 
        build_date=build_date,
        device_capabilities=device_capabilities
    )


def get_readable_device_capabilities(*, platform: Optional[Platform] = None):
    version = get_version_info()
    if platform is None:
        platform = get_platform()

    return _get_device_capabilities_str(platform, version, readable=True)


def write_build_info_header(
        build_info_header_path: Path, *, platform: Optional[Platform] = None
    ):
    
    build_info = get_build_info(platform=platform)

    template = _get_template('build_info_source.jinja.cpp')
    header = template.render(build_info=build_info)
    build_info_header_path.parent.mkdir(parents=True, exist_ok=True)
    logger.info(f"generating version header: {build_info_header_path.parent}")
    with open(build_info_header_path, 'w') as f:
        f.write(header)    


__all__ = [
    'BuildInfo',
    'Version',
    'get_version_info', 
    'get_readable_device_capabilities',
    'get_build_info', 
    'write_build_info_header'
]
