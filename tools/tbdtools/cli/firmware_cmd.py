import sys
import typer
from subprocess import run

from tbdtools.cmd_utils import get_ctx, get_build_dir
from tbdtools.project import get_readable_device_capabilities


firmware_group = typer.Typer()


@firmware_group.command('caps')
def capabilities_cmd(_ctx: typer.Context):
    """ show device capabilities information """

    platform = get_ctx(_ctx).platform
    print(get_readable_device_capabilities(platform=platform))


@firmware_group.command('platform')
def capabilities_cmd(_ctx: typer.Context,
    verbose: bool = typer.Option(False, '-v', '--verbose'),
):
    """ show active build target platform """

    platform = get_ctx(_ctx).platform
    if verbose:
        print(f'ID:          {platform.name}')
        print(f'description: {platform.value}')
    else:
        print(platform.name)


@firmware_group.command('build-dir')
def capabilities_cmd(_ctx: typer.Context):
    """ show device capabilities information """

    print(get_build_dir(_ctx)())


@firmware_group.command('configure', help='create application build files')
def configure_cmd(_ctx: typer.Context):
    """ configure firmware """
    build_dir = get_build_dir(_ctx)
    platform = get_ctx(_ctx).platform.name
    run(['cmake', 
         '-B', build_dir(), 
         '-G', 'Ninja', 
         f'-DTBD_PLATFORM={platform}'], 
         stdout=sys.stdout, stderr=sys.stderr)


@firmware_group.command('build', help='build the application')
def build_cmd(_ctx: typer.Context):
    """ build firmware """
    build_dir = get_build_dir(_ctx)
    platform = get_ctx(_ctx).platform.name
    run(['cmake', '--build', build_dir()], stdout=sys.stdout, stderr=sys.stderr)


@firmware_group.command('emu', help='run the application in qemu')
def emu_cmd(_ctx: typer.Context,
    gdb: bool = typer.Option(False)
):
    """ build firmware """
    build_dir = get_build_dir(_ctx)
    platform = get_ctx(_ctx).platform.name

    args = [
                'idf.py',
                '-B', build_dir(),
                f'-DTBD_PLATFORM={platform}',
                'qemu',
                # '--gdb',
                # 'monitor'
            ]
    if gdb:
        args.append('gdb')
    else:
        args.append('monitor')

    run(args, stdout=sys.stdout, stderr=sys.stderr)


@firmware_group.command('reconfigure')
def reconfigure_cmd(_ctx: typer.Context):
    """ reconfigure firmware """
    build_dir = get_build_dir(_ctx)
    platform = get_ctx(_ctx).platform.name
    run(['cmake', 
         '-B', build_dir(), 
         '-G', 'Ninja', 
         f'-DTBD_PLATFORM={platform}',
         '--fresh'
         ], stdout=sys.stdout, stderr=sys.stderr)


@firmware_group.command('clean')
def clean_cmd(_ctx: typer.Context):
    """ remove build files """
    build_dir = get_build_dir(_ctx)
    run(['idf.py', '-B', build_dir(), 'clean'], stdout=sys.stdout, stderr=sys.stderr)


@firmware_group.command('fullclean')
def fullclean_cmd(_ctx: typer.Context):
    """ remove all build files and build config """
    build_dir = get_build_dir(_ctx)
    run(['idf.py', '-B', build_dir(), 'fullclean'], stdout=sys.stdout, stderr=sys.stderr)


@firmware_group.command('flash')
def flash_cmd(_ctx: typer.Context):
    """ remove all build files and build config """
    build_dir = get_build_dir(_ctx)
    print('BILD DIR', build_dir())
    run(['idf.py', '-B', build_dir(), 'flash'], stdout=sys.stdout, stderr=sys.stderr)


@firmware_group.command('flash-app')
def flash_app_cmd(_ctx: typer.Context):
    """ remove all build files and build config """
    build_dir = get_build_dir(_ctx)
    print('BILD DIR', build_dir())
    run(['idf.py', '-B', build_dir(), 'app-flash'], stdout=sys.stdout, stderr=sys.stderr)


@firmware_group.command('monitor')
def flash_cmd(_ctx: typer.Context):
    """ remove all build files and build config """
    build_dir = get_build_dir(_ctx)
    run(['idf.py', '-B', build_dir(), 'monitor'], stdout=sys.stdout, stderr=sys.stderr)


__all__ = ['firmware_group']
