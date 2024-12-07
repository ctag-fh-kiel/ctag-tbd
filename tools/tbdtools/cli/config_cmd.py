import typer
from subprocess import run

from tbdtools.cmd_utils import get_build_dir


config_group = typer.Typer()


@config_group.command('gui')
def build_cmd(_ctx: typer.Context):
    """ show config GUI """
    
    build_dir = get_build_dir(_ctx)
    run(['idf.py', '-B', build_dir(), 'menuconfig'])


__all__ = ['config_group']
