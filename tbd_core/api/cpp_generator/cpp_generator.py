from pathlib import Path
import proto_schema_parser.ast as proto
import proto_schema_parser.generator as protog

from tbd_core.api import Endpoint, jilter, ApiGeneratorBase, Event, Api, generate_protos, FiltersBase
from tbd_core.api.idc_interfaces import IDCHandler

import tbd_core.buildgen as tbd


class CppFilters(FiltersBase):
    @jilter
    def func_args(self, idc: IDCHandler, *, with_output: bool = True) -> str:
        arg_list = []
        if idc.has_args:
            for arg_name, arg_type in idc.args.items():
                arg_list.append(f'const {arg_type}& {arg_name}')

        if with_output and isinstance(idc, Endpoint) and idc.has_output:
            arg_list.append(f'{idc.output}& output')
        return ', '.join(arg_list)

    @jilter
    def func_arg_types(self, idc: IDCHandler, *, with_output: bool = True) -> str:
        arg_list = []
        if idc.has_args:
            for arg_type in idc.args.values():
                arg_list.append(f'const {arg_type}&')

        if with_output and isinstance(idc, Endpoint) and idc.has_output:
            arg_list.append(f'{idc.output}&')
        return ', '.join(arg_list)

    @jilter
    def func_plain_arg_types(self, idc: IDCHandler, *, with_output: bool = True) -> str:
        arg_list = []
        if idc.has_args:
            for arg_type in idc.args.values():
                arg_list.append(arg_type)

        if with_output and isinstance(idc, Endpoint) and idc.has_output:
            arg_list.append(idc.output)
        return ', '.join(arg_list)

    @jilter
    def shadow_declaration(self, idc: IDCHandler) -> str:
        namespace = idc.scope.parent
        ret = idc.return_type
        name = idc.name
        args = self.func_args(idc)
        return f'namespace {namespace} {{ {ret} {name}({args}); }}'

    @jilter
    def client_rpc_method(self, endpoint: Endpoint) -> str:
        method_name = endpoint.name
        inputs_args = f'{self.func_args(endpoint, with_output=False)}, ' if endpoint.has_args else ''
        callback = f'Callback<{endpoint.output}> callback' if endpoint.has_output else 'VoidCallback callback'

        return f'{method_name}({inputs_args}{callback})'

    @jilter
    def invoke_dispatcher_from_args(self, event: Event) -> str:
        args = self.forward_args(event)
        dispatcher_name = self.dispatcher_name(event)
        return f'tbd::api::{dispatcher_name}({args})'

    @jilter
    def invoke_dispatcher_from_message(self, event: Event) -> str:
        args = self.forward_args(event, prefix='in_message.', single_arg_name='input')
        dispatcher_name = self.dispatcher_name(event)
        return f'tbd::api::{dispatcher_name}({args})'

    @jilter
    def invoke_handler(self, endpoint: Endpoint) -> str:
        args = self.forward_args(endpoint, prefix='in_message.', single_arg_name='input', output_arg_name='out_value')
        return f'{endpoint.full_name}({args})'

    @staticmethod
    @jilter
    def rpc_handler_name(endpoint: Endpoint) -> str:
        return f'handle_rpc__{endpoint.name}'

    @staticmethod
    @jilter
    def event_handler_name(endpoint: Endpoint) -> str:
        return f'handle_event__{endpoint.name}'

    @staticmethod
    @jilter
    def emitter_name(event: Event) -> str:
        return f'{event.func.name}'

    @staticmethod
    @jilter
    def dispatcher_name(event: Event) -> str:
        return f'dispatch__{event.name}'


class CppGenerator(ApiGeneratorBase):
    def __init__(self, api: Api):
        super().__init__(api, tbd.get_tbd_components_root() / 'api' / 'tbd_api', CppFilters(api))

    def write_protos(self, protos_file_name: Path) -> None:
        payload_types = [payload.proto_type for payload in self._api.payload_types if not payload.is_builtin]
        request_types = [request.proto_type for request in self._api.request_types if not request.is_builtin]
        wrapper_types = [response.proto_type for response in self._api.response_types
                         if not response.is_builtin and response.is_wrapper]
        event_payloads = [request.proto_type for request in self._api.event_payloads]

        wrappers = protog.Generator().generate(proto.File(
            syntax='proto3',
            file_elements=[
                *payload_types,
                *request_types,
                *wrapper_types,
                *event_payloads,
            ]
        ))

        out_dir = protos_file_name.parent
        out_dir.mkdir(exist_ok=True, parents=True)
        with open(protos_file_name, 'w') as f:
            f.write(wrappers)

        generate_protos(out_dir, out_dir, [protos_file_name.name])

    def write_arduino_client(self, out_dir: Path):
        client_component_dir = tbd.get_tbd_components_root() / 'api' / 'tbd_api' / 'clients' / 'arduino'
        tbd.copy_batch(
            client_component_dir,
            out_dir,
            [
                'api/module.hpp',
                'client/module.hpp',
                'client/client.hpp',
                'client/tbd_client.hpp',
                'logging.hpp',
            ],
            sub_dir='include/tbd'
        )
        tbd.copy_batch(
            client_component_dir,
            out_dir,
            [
                'src/main.cpp',
                'platformio.ini',
            ],
            ignore=['*.pb.h', '*.pb.c'],
        )

        tbd.copy_batch(
            tbd.get_tbd_components_root() / 'api' / 'tbd_api',
            out_dir,
            [
                'messages.hpp',
                'packet.hpp',
                'packet_parser.hpp',
                'packet_stream_parser.hpp',
                'packet_writers.hpp',
            ],
            sub_dir='include/tbd/api'
        )
        tbd.copy_batch(
            tbd.get_tbd_source_root() / 'tbd_module',
            out_dir,
            [
                'errors.hpp',
                'parameter_types.hpp',
            ],
            sub_dir='include/tbd',
        )
        self._write_arduino_client_classes(out_dir)
        self._write_transcoding(out_dir)

    def _write_arduino_client_classes(self, out_dir: Path):
        client_hpp_path = Path('clients/arduino/include/tbd/client')
        client_cpp_path = Path('clients/arduino/src')

        rcp_hpp_source = self.render(client_hpp_path / 'tbd_rpc.hpp.j2')
        rcp_cpp_source = self.render(client_cpp_path / 'tbd_rpc.cpp.j2')
        event_hpp_source = self.render(client_hpp_path / 'tbd_event.hpp.j2')
        event_cpp_source = self.render(client_cpp_path / 'tbd_event.cpp.j2')

        client_hpp_out_dir = out_dir / 'include' / 'tbd' / 'client'
        client_hpp_out_dir.mkdir(parents=True, exist_ok=True)

        client_cpp_out_dir = out_dir / 'src'
        client_cpp_out_dir.mkdir(parents=True, exist_ok=True)

        rpc_hpp_file = client_hpp_out_dir / 'tbd_rpc.hpp'
        with open(rpc_hpp_file, 'w') as f:
            f.write(rcp_hpp_source)

        rpc_cpp_file = client_cpp_out_dir / 'tbd_rpc.cpp'
        with open(rpc_cpp_file, 'w') as f:
            f.write(rcp_cpp_source)

        event_hpp_file = client_hpp_out_dir / 'tbd_event.hpp'
        with open(event_hpp_file, 'w') as f:
            f.write(event_hpp_source)

        event_cpp_file = client_cpp_out_dir / 'tbd_event.cpp'
        with open(event_cpp_file, 'w') as f:
            f.write(event_cpp_source)

    def _write_transcoding(self, out_dir: Path):
        client_cpp_path = out_dir / 'src'
        client_protos_file = client_cpp_path / 'api_types.proto'

        self.write_protos(client_protos_file)
        generate_protos(client_cpp_path, client_cpp_path, [client_protos_file.name])

        source = self.render('src/api_message_transcoding.hpp.j2', client_types=True)
        client_cpp_path.mkdir(exist_ok=True, parents=True)
        out_file = client_cpp_path / 'api_message_transcoding.hpp'
        with open(out_file, 'w') as f:
            f.write(source)




__all__ = ['CppGenerator']