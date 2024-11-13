import json
import sys
import typer
from subprocess import run

from tbdtools.project import ProjectRoot, get_platform
from tbdtools.project.build_info import get_readable_device_capabilities


firmware_group = typer.Typer()


@firmware_group.command('caps')
def capabilities_cmd(ctx: typer.Context):
    """ show device capabilities information """

    print(get_readable_device_capabilities())


@firmware_group.command('platform')
def capabilities_cmd(ctx: typer.Context,
    verbose: bool = typer.Option(False, '-v', '--verbose'),
):
    """ show active build target platform """

    platform = get_platform()
    if verbose:
        print(f'ID:          {platform.name}')
        print(f'description: {platform.value}')
    else:
        print(platform.name)


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
