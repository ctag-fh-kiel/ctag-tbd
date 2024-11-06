import typer
from subprocess import run

from tbdtools.project import ProjectStructure


config_group = typer.Typer()


@config_group.command('gui')
def build_cmd(ctx: typer.Context):
    """ show config GUI """
    
    dirs: ProjectStructure = ctx.obj
    run(['idf.py', '-B', dirs.firmware_build, 'menuconfig'])


__all__ = ['config_group']
