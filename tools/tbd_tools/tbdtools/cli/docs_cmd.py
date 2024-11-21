import typer
from subprocess import run

from tbdtools.cmd_utils import get_ctx
from tbdtools.project import ProjectRoot
from tbdtools.cmd_utils import write_docs_for_cli ,get_main


from sphinx.cmd.make_mode import Make as SphinxMake
docs_group = typer.Typer()


@docs_group.command('makefile-build')
def makefile_build_cmd(_ctx: typer.Context):
    """ build sphinx docs """
    
    dirs = get_ctx(_ctx).dirs
    if not (dirs.build.docs.cpp() / 'index.xml').is_file():
        run(['make', '-f', dirs.docs.config() / 'Makefile', 'doxygen'])
    run(['make', '-f', dirs.docs.config() / 'Makefile', 'html']) 


@docs_group.command('build')
def build_cmd(_ctx: typer.Context):
    dirs = get_ctx(_ctx).dirs

    print(dirs.docs.config)

    options = ['--conf-dir', str(dirs.docs.config())]
    builder = SphinxMake(
        source_dir=str(dirs.docs()),
        build_dir=str(dirs.build.docs()), 
        opts=options
    )
    builder.run_generic_build('html')



_cli_docs_header = '''*********************
TBD Command Line Tool
*********************'''


@docs_group.command('create-for-cli')
def create_for_cli_cmd(_ctx: typer.Context):
    """ generate docs for the TBD cli """

    dirs = get_ctx(_ctx).dirs
    
    write_docs_for_cli(_cli_docs_header, get_main(), dirs.docs.cli())


__all__ = ['docs_group']