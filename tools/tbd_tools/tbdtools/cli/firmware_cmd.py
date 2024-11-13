import sys
import typer
from subprocess import run

from tbdtools.project import ProjectRoot
from tbdtools.project.build_info import Platform


firmware_group = typer.Typer()


@firmware_group.command('build')
def build_cmd(ctx: typer.Context):
    """ build firmware """
    dirs: ProjectRoot = ctx.obj
    run(['idf.py', '-B', dirs.build.firmware(), 'build'], stdout=sys.stdout, stderr=sys.stderr)

@firmware_group.command('reconfigure')
def reconfigure_cmd(ctx: typer.Context):
    """ build firmware """
    dirs: ProjectRoot = ctx.obj
    run(['idf.py', '-B', dirs.build.firmware(), 'reconfigure'], stdout=sys.stdout, stderr=sys.stderr)

@firmware_group.command('clean')
def clean_cmd(ctx: typer.Context):
    """ remove build files """
    dirs: ProjectRoot = ctx.obj
    run(['idf.py', '-B', dirs.build.firmware(), 'clean'], stdout=sys.stdout, stderr=sys.stderr)

@firmware_group.command('fullclean')
def fullclean_cmd(ctx: typer.Context):
    """ remove all build files and build config """
    dirs: ProjectRoot = ctx.obj
    run(['idf.py', '-B', dirs.build.firmware(), 'fullclean'], stdout=sys.stdout, stderr=sys.stderr)


__all__ = ['firmware_group']
