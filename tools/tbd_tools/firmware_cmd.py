import sys
import typer
from subprocess import run

from project import ProjectStructure


firmware_group = typer.Typer()


@firmware_group.command('build')
def build_cmd(ctx: typer.Context):
    """ build firmware """
    dirs: ProjectStructure = ctx.obj
    run(['idf.py', '-B', dirs.firmware_build, 'build'], stdout=sys.stdout, stderr=sys.stderr)

@firmware_group.command('clean')
def build_cmd(ctx: typer.Context):
    """ remove build files """
    dirs: ProjectStructure = ctx.obj
    run(['idf.py', '-B', dirs.firmware_build, 'clean'], stdout=sys.stdout, stderr=sys.stderr)

@firmware_group.command('fullclean')
def build_cmd(ctx: typer.Context):
    """ remove all build files and build config """
    dirs: ProjectStructure = ctx.obj
    run(['idf.py', '-B', dirs.firmware_build, 'fullclean'], stdout=sys.stdout, stderr=sys.stderr)


__all__ = ['firmware_group']
