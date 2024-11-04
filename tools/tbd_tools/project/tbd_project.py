from dataclasses import dataclass
import os
from pathlib import Path
from typing import Optional

import git


@dataclass
class ProjectStructure:
    project: Path

    @property
    def build(self):
        return self.project / 'build'
    
    @property
    def build_firmware(self):
        return self.build / 'firmware'
    
    @property
    def generated_includes(self):
        return self.build_firmware / 'gen_include'
    
    @property
    def docs(self):
        return self.project / 'docs'

    @property
    def docs_config(self):
        return self.docs / 'config'

    @property
    def docs_build(self):
        return self.build / 'docs'

    @property
    def code_docs(self):
        return self.docs_build / 'code_xml'
    
    @property
    def firmware_build(self):
        return self.build / 'firmware'
    
    @property
    def plugins(self):
        return self.project / 'components' / 'tbd_sounds' / 'include' / 'tbd' / 'sounds'
    
    @property
    def plugins_config(self):
        return self.project / 'components' / 'tbd_sounds' / 'Kconfig.projbuild'
    
    @property
    def plugin_registry(self):
        return self.project / 'components' / 'tbd_sound_registry' / 'include' / 'tbd' / 'sound_registry'
    
    @property
    def tools(self):
        return self.project / 'tools'

    @property
    def simulator(self):
        return self.tools / 'tbd_simulator'
    
    @property
    def simulator_build(self):
        return self.build / 'tbd_simulator'
    

def get_project_repo() -> git.Repo:
    cwd = Path(os.getcwd()) 
    return git.Repo(cwd, search_parent_directories=True)


def _find_root_from_env() -> Optional[Path]:
    if (project_dir := os.getenv('TBD_PROJECT_DIR')) is not None:
        return Path(project_dir)
    return None


def _find_root_from_repo() -> Path:
    try:
        git_repo = get_project_repo()
        git_root = git_repo.git.rev_parse("--show-toplevel")
        return Path(git_root)
    except:
        return None


def find_project_root() -> Path:
    if (project_root := _find_root_from_env()) is not None:
        return project_root
    if (project_root := _find_root_from_repo()) is not None:
        return project_root
    return Path(os.getcwd())
    


__all__ = ['ProjectStructure', 'get_project_repo', 'find_project_root']
