import subprocess
from pathlib import Path
from typing import Final

import proto_schema_parser.ast as proto
import proto_schema_parser.generator as protog

import esphome.components.tbd_module as tbd

from .api import Api
from .cpp_generator import CppGenerator
from .py_generator import PyGenerator
from .ts_generator import TSGenerator


# EMPTY_REQUEST_TYPE = 'EmptyRequest'
# EMPTY_RESPONSE_TYPE = 'EmptyResponse'
# PAYLOAD_FIELD_NAME = 'payload'
# NO_MESSAGE_INDEX = 'NO_MESSAGE'
# PROTO_POSTFIX = '.proto'
# PROTO_CPP_POSTFIX = '.pb.h'
# PROTO_PY_POSTFIX = '_pb2'

PROTOS_FILE_NAME = 'api_types'
PYTHON_MODULE_NAME = 'tbd_client'
TYPESCRIPT_SRC_DIR = 'src'
BASE_ENDPOINTS_FILE_NAME = 'base_endpoints.py'



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
        response_types = [response.proto_type for response in api_registry.response_types if not response.is_builtin]

        wrappers = protog.Generator().generate(proto.File(
            syntax='proto3',
            file_elements=[
                *payload_types, 
                *request_types, 
                *response_types
            ]
        ))

        out_file = out_dir / self.dtos_proto
        out_dir.mkdir(exist_ok=True, parents=True)
        with open(out_file, 'w') as f:
            f.write(wrappers)

    def write_endpoints(self, out_folder: Path):
        gen = CppGenerator(self._api, Path('src'))
        source = gen.render('api_all_endpoints.cpp.j2')

        out_folder.mkdir(exist_ok=True, parents=True)
        out_file = out_folder / 'api_all_endpoints.cpp'
        with open(out_file, 'w') as f:
            f.write(source)   

    def write_python_client(self, out_dir: Path, messages_dir: Path):
        out_module_dir = out_dir / PYTHON_MODULE_NAME
        base_endpoints_file = self._in_srcs / BASE_ENDPOINTS_FILE_NAME

        tbd.copy_tree_if_outdated(self._python_in_dir(), out_dir)
        tbd.copy_file_if_outdated(base_endpoints_file, out_module_dir / BASE_ENDPOINTS_FILE_NAME)
        self._generate_python_protobuf(out_module_dir, messages_dir)
        self._write_python_client_class(out_module_dir)

    def write_typescript_client(self, out_dir: Path, messages_dir: Path):
        # symlinks will result in bad esbuild module lookups
        tbd.copy_tree_if_outdated(self._typescript_in_dir(), out_dir, symlink=False)
        tbd.copy_file_if_outdated(messages_dir / self.dtos_proto, out_dir / TYPESCRIPT_SRC_DIR / self.dtos_proto, symlink=False)

        self._write_typescript_client_class(out_dir)

    def _python_in_dir(self) -> Path:
        return self._in_srcs / 'clients' / 'python'

    def _generate_python_protobuf(self, out_module_dir: Path, messages_dir: Path):
        subprocess.run(['protoc', f'--proto_path={messages_dir}',
                        f'--python_out={out_module_dir}', self.dtos_proto])

    def _write_python_client_class(self, out_module_dir: Path):
        template_dir = self._python_in_dir() / PYTHON_MODULE_NAME
        gen = PyGenerator(self._api, template_dir)
        source = gen.render('tbd_client.py.j2')

        out_module_dir.mkdir(exist_ok=True, parents=True)
        out_file = out_module_dir / 'client.py'
        with open(out_file, 'w') as f:
            f.write(source)

    def _typescript_in_dir(self) -> Path:
        return self._in_srcs / 'clients' / 'typescript'

    def _write_typescript_client_class(self, out_srcs_dir: Path):
        template_dir = self._typescript_in_dir() / TYPESCRIPT_SRC_DIR
        gen = TSGenerator(self._api, template_dir)
        source = gen.render('tbd_client.ts.j2')

        out_srcs_dir.mkdir(exist_ok=True, parents=True)
        out_file = out_srcs_dir / TYPESCRIPT_SRC_DIR / 'client.ts'
        with open(out_file, 'w') as f:
            f.write(source)


__all__ = ['ApiWriter']
