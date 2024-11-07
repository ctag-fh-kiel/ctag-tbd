import json
import typer
from subprocess import run

from tbdtools.project import (
    find_project_root, 
    get_version_info, 
    get_build_info, 
    write_build_info_header, 
    pretty_print_project_structure,
    ProjectRoot,
)


project_group = typer.Typer()


@project_group.command('version')
def version_cmd(ctx: typer.Context):
    """ show version """

    print(get_version_info())

@project_group.command('root')
def root_cmd(ctx: typer.Context):
    """ show project root dir """

    print(find_project_root())

@project_group.command('build-info')
def build_info_cmd(ctx: typer.Context):
    """ gather current project information to include in build """

    print(get_build_info())


@project_group.command('create-build-info')
def crate_build_info_cmd(ctx: typer.Context):
    """ write build information header """

    dirs: ProjectRoot = ctx.obj
    write_build_info_header(dirs.build.firmware.generated_sources() / 'version.cpp')    


@project_group.command('structure')
def structure_cmd(ctx: typer.Context,
    absolute: bool = typer.Option(False)
):
    """ write build information header """

    dirs: ProjectRoot = ctx.obj
    print(pretty_print_project_structure(dirs, absolute))


@project_group.command('path-to')
def path_to_cmd(ctx: typer.Context,
    absolute: bool = typer.Option(False),
    elem: str = typer.Argument(...),
):
    """ write build information header """

    dirs: ProjectRoot = ctx.obj
    pos = dirs
    for path_segment in elem.split('.'):
        pos = getattr(pos, path_segment)
    print(pos())

__all__ = ['project_group']
