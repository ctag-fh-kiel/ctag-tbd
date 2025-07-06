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

    def write_messages(self, out_dir: Path):
        gen = CppGenerator(self._api)
        source = gen.render('src/api_message_transcoding.hpp.j2', server_types=True)

        out_dir.mkdir(exist_ok=True, parents=True)
        hpp_file = out_dir / 'api_message_transcoding.hpp'
        with open(hpp_file, 'w') as f:
            f.write(source)

    def write_endpoints(self, out_dir: Path):
        gen = CppGenerator(self._api)
        source = gen.render('src/api_all_endpoints.cpp.j2')

        out_dir.mkdir(exist_ok=True, parents=True)
        out_file = out_dir / 'api_all_endpoints.cpp'
        with open(out_file, 'w') as f:
            f.write(source)

    def write_events(self, out_dir: Path):
        gen = CppGenerator(self._api)
        all_events_cpp = gen.render('src/api_all_events.cpp.j2')
        all_dispatchers_hpp = gen.render('src/api_all_events_declarations.hpp.j2')
        all_actions_hpp = gen.render('src/api_all_esphome_actions.hpp.j2')

        out_dir.mkdir(exist_ok=True, parents=True)
        with open(out_dir / 'api_all_events.cpp', 'w') as f:
            f.write(all_events_cpp)
        with open(out_dir / 'api_all_events_declarations.hpp', 'w') as f:
            f.write(all_dispatchers_hpp)
        with open(out_dir / 'api_all_esphome_actions.hpp', 'w') as f:
            f.write(all_actions_hpp)

    def write_arduino_client(self, out_dir: Path, messages_dir: Path):
        gen = CppGenerator(self._api)
        gen.write_arduino_client(out_dir)

    def write_python_client(self, out_dir: Path, messages_dir: Path):
        gen = PyGenerator(self._api, self._serializables)
        gen.write_client(out_dir)

    def write_typescript_client(self, out_dir: Path, messages_dir: Path):
        gen = TSGenerator(self._api, self._serializables)
        gen.write_client(out_dir)




__all__ = ['ApiWriter']
