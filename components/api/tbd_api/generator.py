import subprocess
from pathlib import Path
from typing import Final

import esphome.components.tbd_module as tbd

from tbd_core.api import Api
from tbd_core.api.cpp_generator import CppGenerator
from tbd_core.api.py_generator import PyGenerator
from tbd_core.api.ts_generator import TSGenerator
from tbd_core.serialization import SerializableGenerator


class ApiWriter:
    def __init__(self, api: Api, serializables: SerializableGenerator):
        self._api: Api = api
        self._serializables: SerializableGenerator = serializables
        self._in_srcs: Final = Path(__file__).parent

    def write_endpoints(self, headers_dir: Path, sources_dir: Path):
        gen = CppGenerator(self._api)

        source = gen.render('src/endpoint_index.hpp.j2')
        headers_dir.mkdir(exist_ok=True, parents=True)
        out_file = headers_dir / 'endpoint_index.hpp'
        with open(out_file, 'w') as f:
            f.write(source)

        source = gen.render('src/endpoint_index.cpp.j2')
        sources_dir.mkdir(exist_ok=True, parents=True)
        out_file = sources_dir / 'endpoint_index.cpp'
        with open(out_file, 'w') as f:
            f.write(source)

    def write_events(self, headers_dir: Path, sources_dir: Path):
        gen = CppGenerator(self._api)
        all_events_hpp = gen.render('src/event_index.hpp.j2')
        all_events_cpp = gen.render('src/event_index.cpp.j2')
        all_actions_hpp = gen.render('src/action_index.hpp.j2')

        sources_dir.mkdir(exist_ok=True, parents=True)
        with open(headers_dir / 'event_index.hpp', 'w') as f:
            f.write(all_events_hpp)
        with open(sources_dir / 'event_index.cpp', 'w') as f:
            f.write(all_events_cpp)
        with open(headers_dir / 'action_index.hpp', 'w') as f:
            f.write(all_actions_hpp)

    def write_arduino_client(self, out_dir: Path):
        gen = CppGenerator(self._api)
        gen.write_arduino_client(out_dir)

    def write_python_client(self, out_dir: Path):
        gen = PyGenerator(self._api, self._serializables)
        gen.write_client(out_dir)

    def write_typescript_client(self, out_dir: Path):
        gen = TSGenerator(self._api, self._serializables)
        gen.write_client(out_dir)




__all__ = ['ApiWriter']
