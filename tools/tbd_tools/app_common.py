from pathlib import Path
import typer

from project import ProjectStructure, get_tbd_project_dir

def common_callback(
    ctx: typer.Context,
    project_dir: Path =  typer.Option(...,'-d', '--project-dir', default_factory=get_tbd_project_dir)
):
    ctx.obj = ProjectStructure(project_dir)
    

__all__ = ['common_callback']