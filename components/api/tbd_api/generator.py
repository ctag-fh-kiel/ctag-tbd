from pathlib import Path
from typing import Final

import proto_schema_parser.ast as proto
import proto_schema_parser.generator as protog

from .enpoints import Endpoint
from .api import Api
from .dtos import ParamPayload
from .cpp_generator import CppGenerator
from .py_generator import PyGenerator

EMPTY_REQUEST_TYPE = 'EmptyRequest'
EMPTY_RESPONSE_TYPE = 'EmptyResponse'
PAYLOAD_FIELD_NAME = 'payload'
NO_MESSAGE_INDEX = 'NO_MESSAGE'
PROTO_POSTFIX = '.proto'
PROTO_CPP_POSTFIX = '.pb.h'
PROTO_PY_POSTFIX = '_pb2'
PROTOS_FILE_NAME = 'api_types'


def callback_name(endpoint: Endpoint) -> str:
    return f'handle_{endpoint.name}'

def endpoint_type(endpoint: Endpoint) -> str:
    return endpoint.type.value

def handler_type(endpoint: Endpoint) -> str:
    return endpoint.type.value




class ApiWriter:
    def __init__(self, api: Api):
        self._api: Final = api

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
        gen = CppGenerator(self._api)
        source = gen.render('api_all_endpoints.cpp.j2')

        out_folder.mkdir(exist_ok=True, parents=True)
        out_file = out_folder / 'api_all_endpoints.cpp'
        with open(out_file, 'w') as f:
            f.write(source)   

    def write_python_client(self, out_folder: Path):
        gen = PyGenerator(self._api)
        source = gen.render('tbd_client.py.j2')

        out_folder.mkdir(exist_ok=True, parents=True)
        out_file = out_folder / 'client.py'
        with open(out_file, 'w') as f:
            f.write(source)

    ## filters ##

    # python filters

    def _get_py_arg(self, endpoint: Endpoint):
        if not endpoint.args:
            raise ValueError(f'endpoint {endpoint.name} has no input')
        message_name = endpoint.args
        arg = self._api.get_payload(message_name)
        if isinstance(arg, ParamPayload):
            return arg.name
        return f'{PROTOS_FILE_NAME}{PROTO_PY_POSTFIX}.{arg.name}'

    def _get_py_return(self, endpoint: Endpoint):
        if not endpoint.output:
            raise ValueError(f'endpoint {endpoint.name} has no input')
        message_name = endpoint.output
        result = self._api.get_payload(message_name)

        if isinstance(result, ParamPayload):
            return result.name
        return f'{PROTOS_FILE_NAME}{PROTO_PY_POSTFIX}.{result.name}'

    def _get_py_request(self, endpoint: Endpoint):
        if not endpoint.args:
            raise ValueError(f'endpoint {endpoint.name} has no input')
        message_name = endpoint.args
        request = self._api.get_request(message_name)
        return f'{PROTOS_FILE_NAME}{PROTO_PY_POSTFIX}.{request.name}'
    
    def _get_py_response(self, endpoint: Endpoint):
        if not endpoint.output:
            raise ValueError(f'endpoint {endpoint.name} has no output')
        message_name = endpoint.output
        response = self._api.get_response(message_name)
        return f'{PROTOS_FILE_NAME}{PROTO_PY_POSTFIX}.{response.name}'

    def _get_endpoint_id(self, endpoint: Endpoint):
        return self._api.get_endpoint_id(endpoint.name)

    def _get_request_id(self, endpoint: Endpoint):
        return self._api.get_request_id(endpoint.args) if endpoint.args else NO_MESSAGE_INDEX

    def _get_response_id(self, endpoint: Endpoint):
        return self._api.get_response_id(endpoint.output) if endpoint.output else NO_MESSAGE_INDEX

    def _get_request_name(self, endpoint: Endpoint) -> str:
        return 'void_request' if not endpoint.args else self._api.get_request(endpoint.args).name
    
    def _get_response_name(self, endpoint: Endpoint) -> str:
        return 'void_response' if not endpoint.output else self._api.get_response(endpoint.output).name

    def _get_request_size(self, endpoint: Endpoint) -> str:
        if endpoint.args:
            return f'{self._get_request_name(endpoint)}_size'
        else:
            return '0'

    def _get_response_size(self, endpoint: Endpoint) -> str:
        if endpoint.output:
            return f'{self._get_response_name(endpoint)}_size'
        else:
            return '0'

__all__ = ['ApiWriter']