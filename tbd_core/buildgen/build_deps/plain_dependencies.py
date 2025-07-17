from pathlib import Path
from typing import Any
from pydantic import TypeAdapter
from esphome.core import TimePeriod
import esphome.git as git
from .external_dependency import ExternalLibrary, Build, LibraryJson



def plain_dependency(
        name: str, 
        url: str, 
        ref: str, 
        *,
        refresh:TimePeriod = TimePeriod(days=1), 
        includes: list[Path] | None = None,
        sources: list[Path] | None = None,
        defines: dict[str, Any] | None = None,
    ) -> ExternalLibrary:

    includes = includes or []
    sources = sources or []
    defines = defines or {}

    repo_dir, *_ = git.clone_or_update(
        url=url,
        ref=ref,
        refresh=refresh,
        domain=f'plain_libs/{name}',
    )
    write_library_json_from_compilation_commands(repo_dir, name, ref, includes, sources, defines)
    return ExternalLibrary(name=name, repository=f'file:///{repo_dir}')

def write_library_json_from_compilation_commands(
        repo_dir: Path,
        name: str, 
        version: str, 
        includes: list[Path], 
        sources: list[Path], 
        defines: dict[str, Any]
    ):

    includes = [f'-I{include}' for include in includes]
    sources = [f'+<{source}>' for source in sources]
    defines = {f'-D{key}': value for key, value in defines.items()}
    build_obj = Build(flags=[*includes, *defines], srcFilter=sources)

    library_obj = LibraryJson(name=name, version=version, build=build_obj)
    library_data = TypeAdapter(LibraryJson).dump_json(library_obj, indent=4, by_alias=True)
    with open(repo_dir / 'library.json', 'wb') as f:
        f.write(library_data)


__all__ = ['plain_dependency']

