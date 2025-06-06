import subprocess
from pathlib import Path
from typing import Final

import proto_schema_parser.ast as proto
import proto_schema_parser.generator as protog

import esphome.components.tbd_module as tbd

from tbd_core.api import Api
from tbd_core.api.cpp_generator import CppGenerator
from tbd_core.api.py_generator import PyGenerator
from tbd_core.api.ts_generator import TSGenerator


PROTOS_FILE_NAME = 'api_types'


class ApiWriter:
    def __init__(self, api: Api):
        self._api: Final = api
        self._in_srcs: Final = Path(__file__).parent

    @property
    def dtos_proto(self):
        return f'{PROTOS_FILE_NAME}.proto'

    def write_protos(self, out_dir: Path):
        api_registry = self._api

        payload_types = [payload.proto_type for payload in api_registry.payload_types if not payload.is_builtin]
        request_types = [request.proto_type for request in api_registry.request_types if not request.is_builtin]
        wrapper_types = [response.proto_type for response in api_registry.response_types
                         if not response.is_builtin and response.is_wrapper]
        event_payloads = [request.proto_type for request in api_registry.event_payloads]

        wrappers = protog.Generator().generate(proto.File(
            syntax='proto3',
            file_elements=[
                *payload_types,
                *request_types, 
                *wrapper_types,
                *event_payloads,
            ]
        ))

        out_file = out_dir / self.dtos_proto
        out_dir.mkdir(exist_ok=True, parents=True)
        with open(out_file, 'w') as f:
            f.write(wrappers)

    def write_messages(self, out_folder: Path):
        srcs_path = Path(__file__).parent / 'src'
        gen = CppGenerator(self._api, srcs_path)
        source = gen.render('api_message_transcoding.hpp.j2')

        out_folder.mkdir(exist_ok=True, parents=True)
        hpp_file = out_folder / 'api_message_transcoding.hpp'
        with open(hpp_file, 'w') as f:
            f.write(source)

    def write_endpoints(self, out_folder: Path):
        srcs_path = Path(__file__).parent / 'src'
        gen = CppGenerator(self._api, srcs_path)
        source = gen.render('api_all_endpoints.cpp.j2')

        out_folder.mkdir(exist_ok=True, parents=True)
        out_file = out_folder / 'api_all_endpoints.cpp'
        with open(out_file, 'w') as f:
            f.write(source)

    def write_events(self, out_folder: Path):
        srcs_path = Path(__file__).parent / 'src'
        gen = CppGenerator(self._api, srcs_path)
        source = gen.render('api_all_events.cpp.j2')

        out_folder.mkdir(exist_ok=True, parents=True)
        out_file = out_folder / 'api_all_events.cpp'
        with open(out_file, 'w') as f:
            f.write(source)

    def write_python_client(self, out_dir: Path, messages_dir: Path):
        gen = PyGenerator(self._api)
        gen.write_client(out_dir, messages_dir / self.dtos_proto)

    def write_typescript_client(self, out_dir: Path, messages_dir: Path):
        gen = TSGenerator(self._api)
        gen.write_client(out_dir, messages_dir / self.dtos_proto)




__all__ = ['ApiWriter']
