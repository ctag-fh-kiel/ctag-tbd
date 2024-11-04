import typer
from subprocess import run

from project import find_root_from_cwd, get_project_info


project_group = typer.Typer()


@project_group.command('version')
def version_cmd(ctx: typer.Context):
    """ show version """

    print(get_project_info())

@project_group.command('root')
def version_cmd(ctx: typer.Context):
    """ show project root dir """

    print(find_root_from_cwd())


__all__ = ['version']
