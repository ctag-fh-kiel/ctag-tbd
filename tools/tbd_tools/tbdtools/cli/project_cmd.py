import typer
from subprocess import run

from tbdtools.project import find_project_root, get_version_info, get_build_info, write_build_info_header, ProjectStructure


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

    dirs: ProjectStructure = ctx.obj
    write_build_info_header(dirs.generated_includes / 'version.hpp')    


__all__ = ['version']
