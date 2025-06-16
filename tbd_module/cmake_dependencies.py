import json
from pathlib import Path
from typing import Any
from pydantic import TypeAdapter
from esphome.core import TimePeriod, CORE
import esphome.git as git
import subprocess
from .library_manifest import *



def cmake_dependency(
        name: str, 
        url: str, 
        ref: str, 
        *,
        refresh:TimePeriod = TimePeriod(days=1), 
        cmake_parameters: dict[str, Any] = {}
    ) -> str:

    repo_dir, *_ = git.clone_or_update(
        url=url,
        ref=ref,
        refresh=refresh,
        domain=f'cmake_libs/{name}',
    )
    commands = generate_compilation_commands_with_cmake(repo_dir, cmake_parameters)
    write_library_json_from_compilation_commands(repo_dir, name, ref, commands)
    CORE.add_platformio_option('lib_deps', [f'{name}=file:///{repo_dir}'])
    return repo_dir


def generate_compilation_commands_with_cmake(repo_dir: Path, cmake_parameters: dict[str, str] = {}) -> list[BuildCommand]:
    cmake_parameters = { 'BUILD_TESTING': 'OFF', **cmake_parameters}

    cmake_file = repo_dir / 'CMakeLists.txt'
    parameters = [f'-D{param}={value}' for param, value in cmake_parameters.items()]
    cmd = ['cmake', '-DCMAKE_EXPORT_COMPILE_COMMANDS=1', f'-B{repo_dir}'] + parameters + [cmake_file.parent]
    ret = subprocess.run(cmd, cwd=repo_dir, capture_output=True, check=False)
    if ret.returncode != 0 and ret.stderr:
        err_str = ret.stderr.decode("utf-8")
        raise RuntimeError(err_str)

    with open(repo_dir / 'compile_commands.json', 'r') as f:
        json_data = json.load(f)

    if not isinstance(json_data, list):
        raise ValueError(f'expected compile command to be list, got {type(json_data)}')

    return [BuildCommand(**command) for command in json_data]


def write_library_json_from_compilation_commands(repo_dir: Path, name: str, version: str, commands: list[BuildCommand]):
    includes = set()
    defines = set()
    files = set()

    for command in commands:
        command_parts = command.command.split()
        for arg in command_parts:
            if arg.startswith('-D'):
                defines.add(arg)
            if arg.startswith('-I'):
                include_path = Path(arg[2:])
                include_path = f'-I{include_path.relative_to(repo_dir) if include_path.absolute() else include_path}'
                includes.add(include_path)
        
        src_file = Path(command.file)
        command_file = f'+<{src_file.relative_to(repo_dir) if src_file.absolute else src_file}>'
        files.add(command_file)

    build_obj = Build(flags=[*includes, *defines], srcFilter=files)

    library_obj = Library(name=name, version=version, build=build_obj)
    library_data = TypeAdapter(Library).dump_json(library_obj, indent=4, by_alias=True)
    with open(repo_dir / 'library.json', 'wb') as f:
        f.write(library_data)


__all__ = ['cmake_dependency']
