import typer
from subprocess import run

from project import ProjectStructure


docs_group = typer.Typer()


@docs_group.command('build')
def build_cmd(ctx: typer.Context):
    """ build sphinx docs """
    dirs: ProjectStructure = ctx.obj
    if not (dirs.code_docs / 'index.xml').is_file():
        run(['make', '-f', dirs.docs_config / 'Makefile', 'doxygen'])
    run(['make', '-f', dirs.docs_config / 'Makefile', 'html']) 


__all__ = ['docs_group']
