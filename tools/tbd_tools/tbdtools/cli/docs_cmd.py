import typer
from subprocess import run

from tbdtools.project import ProjectStructure
from tbdtools.cmd_utils import write_docs_for_cli ,get_main

docs_group = typer.Typer()


@docs_group.command('build')
def build_cmd(ctx: typer.Context):
    """ build sphinx docs """
    
    dirs: ProjectStructure = ctx.obj
    if not (dirs.code_docs / 'index.xml').is_file():
        run(['make', '-f', dirs.docs_config / 'Makefile', 'doxygen'])
    run(['make', '-f', dirs.docs_config / 'Makefile', 'html']) 


_cli_docs_header = '''
*********************
TBD Command Line Tool
*********************
'''


@docs_group.command('create-for-cli')
def create_for_cli_cmd(ctx: typer.Context):
    """ generate docs for the TBD cli """

    dirs: ProjectStructure = ctx.obj
    
    write_docs_for_cli(_cli_docs_header, get_main(), dirs.docs_cli)


__all__ = ['docs_group']
