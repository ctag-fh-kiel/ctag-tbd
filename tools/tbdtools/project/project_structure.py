from dataclasses import fields
from enum import Enum, unique
from pathlib import Path
import copy

from git import Optional
from pydantic.dataclasses import dataclass


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
    capabilities: ProjectDir
    esp_sdkconfigs: ProjectDir
    platforms: ProjectDir
    storage: ProjectDir


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

    def platform_dir(self, platform: str) -> Path:
        return self.path / platform
    
    def platform_tree(self, platform: str) -> PlatformBuildDir:
        build_root = self.path
        firmware_root = self.firmware.path
        platform_build_root = build_root / platform
        return self.firmware.relative_to(firmware_root, platform_build_root)

    def __getattr__(self, name: str):
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
    'ProjectFileTreeElement',
    'ProjectDir', 
    'ProjectRoot',
    'ProjectFile',
    'PlatformBuildDir',
]
