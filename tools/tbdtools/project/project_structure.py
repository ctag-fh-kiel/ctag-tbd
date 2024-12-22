from dataclasses import fields
from enum import Enum, unique
from pathlib import Path
import copy

from git import Optional
from pydantic.dataclasses import dataclass


# TODO: determine from files in config/platforms
@unique
class Platform(Enum):
    v1        = 'v1'
    v2        = 'v2'
    str       = 'str'
    aem       = 'aem'
    mk2       = 'mk2'
    bba1      = 'bba1'
    bba2      = 'bba2'
    dada      = 'dada'
    dadax     = 'dadax'
    desktop   = 'desktop'
    simulator = 'simulator'
    emu       = 'emu'


# TODO: read form json file
def get_platform_description(platform: Platform) -> str:
    descriptions = {
        'v1'       : 'TBD mk1 rev2 WM8978, ESP ADC',
        'v2'       : 'TBD mk1 rev2 WM8731, ESP ADC',
        'str'      : 'CTAG Strämpler WM8731, MCP3208',
        'aem'      : 'AE Modular WM8974, ESP ADC',
        'mk2'      : 'TBD MK2 WM8978, STM32 CVs/Trigs',
        'bba1'     : 'TBD BBA MIDI, es8388',
        'bba2'     : 'TBD BBA MIDI, aic3254',
        'dada'     : 'TBD DADA MIDI and MIDI host',
        'dadax'     : 'TBD DADA MIDI and MIDI host with control panel',
        'desktop'  : 'Desktop App',
        'simulator': 'TBD firmware simulator',
        'emu'      : 'TBD ESP32 emulator'
    }
    return descriptions[platform.name]


@dataclass(kw_only=True)
class ProjectFileTreeElement:
    project_root: Optional[Path]
    description: Optional[str] = None

    def __getattribute__(self, name: str):
        """ translate paths to absolute paths if project path attribute exists """

        attr = super().__getattribute__(name)
        if isinstance(attr, Path) and name != 'project_root':
            project = super().__getattribute__('project_root')
            if project is not None:
                return project / attr
            
        # ignore non path attributes
        return attr
    
    def relative(self) -> 'ProjectFileTreeElement':
        """ make subtree relative to project root """
        return self.change_root(None)

    def change_root(self, path: Optional[Path]) -> 'ProjectFileTreeElement':
        """ change project root for subtree """
        relative = copy.copy(self)
        
        relative.project_root = path
        for field in fields(relative):
            if isinstance(attr := getattr(relative, field.name), ProjectFileTreeElement):
                setattr(relative, field.name, attr.change_root(path))
        return relative      
    
    def relative_to(self, path: Path, new_root: Optional[Path] = None) -> 'ProjectFileTreeElement':
        relative = copy.copy(self)

        for field in fields(relative):
            attr = getattr(relative, field.name)
            if isinstance(attr, Path) and field.name != 'project_root':
                if attr.is_relative_to(path):
                    relative_path = attr.relative_to(path)
                    setattr(relative, field.name, relative_path)
            if isinstance(attr, ProjectFileTreeElement):
                setattr(relative, field.name, attr.relative_to(path, new_root))

        if new_root is not None:
            relative.project_root = new_root
        else:
            relative.project_root = path
        return relative   


@dataclass
class ProjectDir(ProjectFileTreeElement):
    path: Path

    def __call__(self):
        return self.path


@dataclass
class ProjectFile(ProjectFileTreeElement):
    file: Path

    def __call__(self) -> Path:
        return self.file


@dataclass
class CppLibrary(ProjectDir):
    headers: ProjectDir
    sources: ProjectDir


@dataclass
class ProjectConfig(ProjectDir):
    cmake: ProjectFile


@dataclass
class ProjectSources(ProjectDir):
    sounds: CppLibrary
    sound_registry: CppLibrary


@dataclass
class TbdTools(ProjectDir):
    package: ProjectDir


@dataclass
class ToolsRoot(ProjectDir):
    tbd_tools: TbdTools


@dataclass
class DocsRoot(ProjectDir):
    config: ProjectDir
    cli: ProjectFile


@dataclass
class ResourcesRoot(ProjectDir):
    plugin_definitions: ProjectDir


@dataclass
class BuildDocs(ProjectDir):
    html: ProjectDir
    cpp: ProjectDir 


@dataclass
class PlatformBuildDir(ProjectDir):
    generated_sources: ProjectDir


@dataclass
class BuildRoot(ProjectDir):
    docs: BuildDocs
    firmware: PlatformBuildDir

    def platform_dir(self, platform: Platform) -> Path:
        return self.path / platform.name
    
    def platform_tree(self, platform: Platform) -> PlatformBuildDir:
        build_root = self.path
        firmware_root = self.firmware.path
        platform_build_root = build_root / platform.name
        return self.firmware.relative_to(firmware_root, platform_build_root)

    def __getattr__(self, name: str):
        if name in Platform._member_names_:
            return self.platform_tree(Platform[name])
        return super().__getattribute__(name)


@dataclass
class ProjectRoot(ProjectFileTreeElement):
    config: ProjectConfig
    src: ProjectSources
    tools: ToolsRoot
    docs: DocsRoot
    resources: ResourcesRoot
    build: BuildRoot   


__all__ = [
    'Platform',
    'get_platform_description',
    'ProjectFileTreeElement',
    'ProjectDir', 
    'ProjectRoot',
    'ProjectFile',
    'PlatformBuildDir',
]
