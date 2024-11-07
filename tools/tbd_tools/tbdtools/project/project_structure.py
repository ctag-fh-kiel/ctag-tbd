from dataclasses import fields
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
        if isinstance(attr, Path) and 'name' != 'project_root':
            project = super().__getattribute__('project_root')
            if project is not None:
                return project / attr
            
        # ignore non path attributes
        return attr
    
    def relative(self) -> 'ProjectFileTreeElement':
        relative = copy.copy(self)
        
        relative.project_root = None
        for field in fields(relative):
            if isinstance(attr := getattr(relative, field.name), ProjectFileTreeElement):
                setattr(relative, field.name, attr.relative())
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
class BuildFirmware(ProjectDir):
    generated_sources: ProjectDir


@dataclass
class BuildRoot(ProjectDir):
    docs: BuildDocs
    firmware: BuildFirmware


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
    'ProjectROot',
    'ProjectFile',
]
