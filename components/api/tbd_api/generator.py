from pathlib import Path
from typing import Final
import jinja2 as ji
from .registry import ApiRegistry, Endpoint, Message
import proto_schema_parser.ast as proto
import proto_schema_parser.generator as protog

EMPTY_REQUEST_TYPE = 'EmptyRequest'
EMPTY_RESPONSE_TYPE = 'EmptyResponse'
PAYLOAD_FIELD_NAME = 'payload'
NO_MESSAGE_INDEX = 'NO_MESSAGE'


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
        env.filters['get_request'] = self._get_request_name
        env.filters['get_response'] = self._get_response_name
        env.filters['endpoint_type'] = endpoint_type
        env.filters['request_id'] = lambda endpoint: self._api_registry.get_request_id(endpoint.in_message) if endpoint.in_message else NO_MESSAGE_INDEX
        env.filters['response_id'] = lambda endpoint: self._api_registry.get_response_id(endpoint.out_message) if endpoint.out_message else NO_MESSAGE_INDEX

        self._env: Final = env

    def write_protos(self, out_file: Path):
        api_registry = self._api_registry
        imports = [proto.Import(proto_file.name) for proto_file in api_registry.proto_files]

        request_types = [request.raw for request in api_registry.request_types if not request.path]
        response_types = [response.raw for response in api_registry.response_types if not response.path]

        wrappers = protog.Generator().generate(proto.File(
            syntax='proto3',
            file_elements=[*imports, *request_types, *response_types]
        ))

        out_file.parent.mkdir(exist_ok=True, parents=True)
        with open(out_file, 'w') as f:
            f.write(wrappers)

    def write_endpoints(self, out_folder: Path):
        env = self._env
        template = env.get_template('api_all_endpoints.j2.cpp')

        source = template.render(
            registry=self._api_registry
        )

        out_folder.mkdir(exist_ok=True, parents=True)
        out_file = out_folder / 'api_all_endpoints.cpp'
        with open(out_file, 'w') as f:
            f.write(source)     

    def _get_request_name(self, endpoint: Endpoint) -> str:
        return self._api_registry.get_request(endpoint.in_message).name
    
    def _get_response_name(self, endpoint: Endpoint) -> str:
        return self._api_registry.get_response(endpoint.out_message).name

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