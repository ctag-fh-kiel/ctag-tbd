from pathlib import Path
from typing import Optional
import typer

from tbdtools.cmd_utils import get_build_dir
from tbdtools.project import (
    get_platform,
    get_project,
    find_project_root, 
    get_version_info, 
    get_build_info, 
    write_build_info_header, 
    pretty_print_project_structure,
)


project_group = typer.Typer()


@project_group.command('version')
def version_cmd():
    """ show version """

    print(get_version_info())


@project_group.command('root')
def root_cmd():
    """ show project root dir """

    print(find_project_root())


@project_group.command('build-info')
def build_info_cmd():
    """ gather current project information to include in build """

    platform = get_platform()
    print(get_build_info(platform=platform))


@project_group.command('create-build-info')
def crate_build_info_cmd(
    out_path: Optional[Path] = typer.Option(None, '-o', '--out-file')                         
):
    """ write build information header """

    if out_path is None:
        out_path = get_build_dir().generated_sources() / 'version.cpp'
    
    platform = get_platform()
    write_build_info_header(out_path, platform=platform) 


@project_group.command('structure')
def structure_cmd(
    relative: bool = typer.Option(False)
):
    """ write build information header """

    dirs = get_project()
    if relative:
        dirs = dirs.relative()

    print(pretty_print_project_structure(dirs))


@project_group.command('path-to')
def path_to_cmd(
    relative: bool = typer.Option(False),
    elem: str = typer.Argument(...),
):
    """ write build information header """

    pos = get_project()
    if relative:
        pos = pos.relative()

    for path_segment in elem.split('.'):
        pos = getattr(pos, path_segment)
    print(pos())


__all__ = ['project_group']
