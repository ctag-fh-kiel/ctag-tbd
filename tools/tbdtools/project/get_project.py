from dataclasses import fields

import os
from pathlib import Path
from typing import Dict, Optional, Union

import git
import yaml

from .project_structure import ProjectDir, ProjectFile, ProjectRoot

    

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
    

PathTreeNode = Dict[str, Union[str, 'PathTreeNode']]

def traverse_path_tree(
        tree: PathTreeNode, 
        project_path: Path = None, 
        no_path: bool = False,
        current_path: Optional[Path] = None, 
    ):

    def format_output_path(path: Path, absolute = False):      
        if not absolute:
            path = path.relative_to(Path('/'))
        if no_path:
            return str(path)
        
        return path
    
    def empty_node(**args):
        project_root = format_output_path(project_path, True) if project_path is not None else None
        return {'description': None, 'project_root': project_root, **args}

    def project_relative_path(path: str, current_path: Path):
        if current_path is None:
            if path.startswith('.'):
                raise TypeError(f'root element {path} can not have relative path')
            
            if not path.startswith('/'):
                raise TypeError(f'root element {path} should start with "/')

            current_path = Path('/')
        if path.startswith('.'):
            return current_path / path
        return Path(path)
        

    retval = empty_node()
    for node_name, node in tree.items():
        if isinstance(node, dict):
            if not (path := node.get('path')):
                raise ValueError(f'folder {node_name} missing path')
            child_path = project_relative_path(path, current_path)
            retval[node_name] = traverse_path_tree(node, project_path, no_path, child_path)
        elif isinstance(node, str):
            if node_name == 'description':
                leaf = node
            elif node_name == 'path':
                leaf = format_output_path(project_relative_path(node, current_path.parent))
            elif node.endswith('/'):
                leaf = empty_node(
                    path=format_output_path(project_relative_path(node, current_path))
                )
            else: 
                leaf = empty_node(
                    file=format_output_path(project_relative_path(node, current_path))
                )
            retval[node_name] = leaf
        else:
            raise TypeError(f'type {type(node)} of node {node_name} type not allowed')
    return retval


def pretty_print_project_structure(root: ProjectRoot, depth: int = 0) -> str:
    def format_description(description: Optional[str]) -> str:
        return f'[{description}]' if description is not None else '' 

    attrs = [(field.name, getattr(root, field.name)) for field in fields(root)]
    folders = [attr for attr in attrs if isinstance(attr[1], ProjectDir)]
    files = [attr for attr in attrs if isinstance(attr[1], ProjectFile)]

    offset = ' ' * depth
    retval = []
    for name, folder in folders:
        description = format_description(folder.description)

        retval.append(f'{offset}📁{name} {folder.path}   {description}')
        nested = pretty_print_project_structure(folder, depth + 2)
        if nested:
            retval.append(nested)
    for name, file in files:
        description = format_description(file.description)
        retval.append(f'{offset}📄{name} {file.file}   {description}')
    return '\n'.join(retval)


_project_root: Optional[ProjectRoot] = None

def get_project() -> ProjectRoot:
    if _project_root is None:
        raise ValueError('project has not been initialized')
    return _project_root

def init_project_structure(project_root: Path, absolute: bool = True) -> None:
    global _project_root
    project_description_file_name = Path(__file__).parent / 'resources' / 'project_structure.yml'
    with open(project_description_file_name, 'r') as f:
        try:
            config = yaml.safe_load(f)
            if absolute:
                structure = traverse_path_tree(config, project_root)
            else:
                structure = traverse_path_tree(config)

            _project_root = ProjectRoot(**structure)
        except yaml.YAMLError as exc:
            print("Error in configuration file:", exc)

__all__ = [
    'get_project_repo', 
    'find_project_root',
    'pretty_print_project_structure',
    'get_project',
    'init_project_structure',
]
