import typer
from subprocess import run

from tbdtools.project import ProjectRoot


config_group = typer.Typer()


@config_group.command('gui')
def build_cmd(ctx: typer.Context):
    """ show config GUI """
    
    dirs: ProjectRoot = ctx.obj
    run(['idf.py', '-B', dirs.build.firmware, 'menuconfig'])


__all__ = ['config_group']
