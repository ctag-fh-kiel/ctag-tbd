from pathlib import Path
from typing import Final
import jinja2 as ji
from .enpoints import Endpoint
from .registry import ApiRegistry
from .dtos import ParamPayload
import proto_schema_parser.ast as proto
import proto_schema_parser.generator as protog

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


class ApiGenerator:
    def __init__(self, api_registry: ApiRegistry):
        self._api_registry: Final = api_registry

        template_path = Path(__file__).parent / 'templates'
        env = ji.Environment(loader=ji.FileSystemLoader(template_path), autoescape=ji.select_autoescape())
        env.filters['callback'] = callback_name
        env.filters['cpp_header'] = lambda file_name: file_name.stem + PROTO_CPP_POSTFIX
        env.filters['get_request'] = self._get_request_name
        env.filters['get_response'] = self._get_response_name
        env.filters['endpoint_type'] = endpoint_type
        env.filters['endpoint_id'] = self._get_endpoint_id
        env.filters['request_id'] = self._get_request_id
        env.filters['response_id'] = self._get_response_id
        env.filters['python_module'] = lambda path: f'{path.stem}_pb2'
        env.filters['py_arg'] = self._get_py_arg
        env.filters['py_return'] = self._get_py_return
        env.filters['py_request'] = self._get_py_request
        env.filters['py_response'] = self._get_py_response

        self._env: Final = env

    @property
    def wrappers_proto(self): 
        return f'{PROTOS_FILE_NAME}.proto'


    def write_protos(self, out_dir: Path):
        api_registry = self._api_registry

        payload_types = [payload.proto_type for payload in api_registry.payload_types if not payload.is_builtin]
        for payload in payload_types:
            print(payload.name)

        request_types = [request.proto_type for request in api_registry.request_types if not request.is_builtin]
        response_types = [response.proto_type for response in api_registry.response_types if not response.is_builtin]

        wrappers = protog.Generator().generate(proto.File(
            syntax='proto3',
            file_elements=[*payload_types, *request_types, *response_types]
        ))

        out_file = out_dir / self.wrappers_proto
        out_dir.mkdir(exist_ok=True, parents=True)
        with open(out_file, 'w') as f:
            f.write(wrappers)

    def write_endpoints(self, out_folder: Path):
        env = self._env
        template = env.get_template('api_all_endpoints.cpp.j2')

        source = template.render(
            registry=self._api_registry
        )

        out_folder.mkdir(exist_ok=True, parents=True)
        out_file = out_folder / 'api_all_endpoints.cpp'
        with open(out_file, 'w') as f:
            f.write(source)   

    def write_python_client(self, out_folder: Path):
        env = self._env
        template = env.get_template('tbd_client.py.j2')

        source = template.render(
            registry=self._api_registry
        )

        out_folder.mkdir(exist_ok=True, parents=True)
        out_file = out_folder / 'tbd_client.py'
        with open(out_file, 'w') as f:
            f.write(source)     

    ## filters ##

    # python filters

    def _get_py_arg(self, endpoint: Endpoint):
        if not endpoint.in_message:
            raise ValueError(f'endpoint {endpoint.name} has no input')
        message_name = endpoint.in_message
        arg = self._api_registry.get_payload(message_name)
        if isinstance(arg, ParamPayload):
            return arg.name
        return f'{PROTOS_FILE_NAME}{PROTO_PY_POSTFIX}.{arg.name}'

    def _get_py_return(self, endpoint: Endpoint):
        if not endpoint.out_message:
            raise ValueError(f'endpoint {endpoint.name} has no input')
        message_name = endpoint.out_message
        result = self._api_registry.get_payload(message_name)

        if isinstance(result, ParamPayload):
            return result.name
        return f'{PROTOS_FILE_NAME}{PROTO_PY_POSTFIX}.{result.name}'

    def _get_py_request(self, endpoint: Endpoint):
        if not endpoint.in_message:
            raise ValueError(f'endpoint {endpoint.name} has no input')
        message_name = endpoint.in_message
        request = self._api_registry.get_request(message_name)
        return f'{PROTOS_FILE_NAME}{PROTO_PY_POSTFIX}.{request.name}'
    
    def _get_py_response(self, endpoint: Endpoint):
        if not endpoint.out_message:
            raise ValueError(f'endpoint {endpoint.name} has no output')
        message_name = endpoint.out_message
        response = self._api_registry.get_response(message_name)
        return f'{PROTOS_FILE_NAME}{PROTO_PY_POSTFIX}.{response.name}'

    def _get_endpoint_id(self, endpoint: Endpoint):
        return self._api_registry.get_endpoint_id(endpoint.name)

    def _get_request_id(self, endpoint: Endpoint):
        return self._api_registry.get_request_id(endpoint.in_message) if endpoint.in_message else NO_MESSAGE_INDEX

    def _get_response_id(self, endpoint: Endpoint):
        return self._api_registry.get_response_id(endpoint.out_message) if endpoint.out_message else NO_MESSAGE_INDEX

    def _get_request_name(self, endpoint: Endpoint) -> str:
        return 'void_request' if not endpoint.in_message else self._api_registry.get_request(endpoint.in_message).name
    
    def _get_response_name(self, endpoint: Endpoint) -> str:
        return 'void_response' if not endpoint.out_message else self._api_registry.get_response(endpoint.out_message).name

    def _get_request_size(self, endpoint: Endpoint) -> str:
        if endpoint.in_message:
            return f'{self._get_request_name(endpoint)}_size'
        else:
            return '0'

    def _get_response_size(self, endpoint: Endpoint) -> str:
        if endpoint.out_message:
            return f'{self._get_response_name(endpoint)}_size'
        else:
            return '0'

__all__ = ['ApiGenerator']