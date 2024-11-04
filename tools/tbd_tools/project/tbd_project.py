from dataclasses import dataclass
import os
from pathlib import Path


@dataclass
class ProjectStructure:
    project: Path

    @property
    def build(self):
        return self.project / 'build'
    
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
    

def get_tbd_project_dir():
    if (project_dir := os.getenv('TBD_PROJECT_DIR')) is not None:
        return project_dir
    return os.getcwd()


__all__ = ['ProjectStructure', 'get_tbd_project_dir']
