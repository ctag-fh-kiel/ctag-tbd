import sys
import typer
from subprocess import run

from tbdtools.project import ProjectRoot


firmware_group = typer.Typer()


@firmware_group.command('build')
def build_cmd(ctx: typer.Context):
    """ build firmware """
    dirs: ProjectRoot = ctx.obj
    run(['idf.py', '-B', dirs.build.firmware(), 'build'], stdout=sys.stdout, stderr=sys.stderr)

@firmware_group.command('clean')
def build_cmd(ctx: typer.Context):
    """ remove build files """
    dirs: ProjectRoot = ctx.obj
    run(['idf.py', '-B', dirs.build.firmware(), 'clean'], stdout=sys.stdout, stderr=sys.stderr)

@firmware_group.command('fullclean')
def build_cmd(ctx: typer.Context):
    """ remove all build files and build config """
    dirs: ProjectRoot = ctx.obj
    run(['idf.py', '-B', dirs.build.firmware(), 'fullclean'], stdout=sys.stdout, stderr=sys.stderr)


__all__ = ['firmware_group']
