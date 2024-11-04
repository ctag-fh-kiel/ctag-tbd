from pathlib import Path
import typer

from project import ProjectStructure, find_project_root

def common_callback(
    ctx: typer.Context,
    project_dir: Path =  typer.Option(...,'-d', '--project-dir', default_factory=find_project_root)
):
    ctx.obj = ProjectStructure(project_dir)
    

__all__ = ['common_callback']