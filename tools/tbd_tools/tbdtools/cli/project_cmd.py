import json
from pathlib import Path
from typing import Optional
import typer
from subprocess import run

from tbdtools.cmd_utils import get_ctx, get_build_dir
from tbdtools.project import (
    find_project_root, 
    get_version_info, 
    get_build_info, 
    write_build_info_header, 
    pretty_print_project_structure,
)


project_group = typer.Typer()


@project_group.command('version')
def version_cmd(_ctx: typer.Context):
    """ show version """

    print(get_version_info())


@project_group.command('root')
def root_cmd(_ctx: typer.Context):
    """ show project root dir """

    print(find_project_root())


@project_group.command('build-info')
def build_info_cmd(_ctx: typer.Context):
    """ gather current project information to include in build """

    platform = get_ctx(_ctx).platform
    print(get_build_info(platform=platform))


@project_group.command('create-build-info')
def crate_build_info_cmd(
    _ctx: typer.Context,
    out_path: Optional[Path] = typer.Option(None, '-o' '--out-file')                         
):
    """ write build information header """

    if out_path is None:
        out_path = get_build_dir(_ctx).generated_sources() / 'version.cpp'
    
    write_build_info_header(out_path) 


@project_group.command('structure')
def structure_cmd(_ctx: typer.Context,
    relative: bool = typer.Option(False)
):
    """ write build information header """

    dirs = get_ctx(_ctx).dirs
    if relative:
        dirs = dirs.relative()

    print(pretty_print_project_structure(dirs))


@project_group.command('path-to')
def path_to_cmd(
    _ctx: typer.Context,
    relative: bool = typer.Option(False),
    elem: str = typer.Argument(...),
):
    """ write build information header """

    pos = get_ctx(_ctx).dirs
    if relative:
        pos = pos.relative()

    for path_segment in elem.split('.'):
        pos = getattr(pos, path_segment)
    print(pos())


__all__ = ['project_group']
