from pathlib import Path

import tbd_core.buildgen as tbb
from tbd_core.api import Endpoint, jilter, ApiGeneratorBase, Event, Api, FiltersBase
from tbd_core.api.idc_interfaces import IDCFunc

import tbd_core.buildgen as tbd
from tbd_core.reflection.reflectables import ScopePath, Param

RPC_HANDLER_NAMESPACE = ScopePath.root().add_namespace('tbd').add_namespace('api').add_namespace('rpc_handlers')
EVENT_DISPATCHER_NAMESPACE = ScopePath.root().add_namespace('tbd').add_namespace('api').add_namespace('event_dispatchers')
EVENT_HANDLER_NAMESPACE = ScopePath.root().add_namespace('tbd').add_namespace('api').add_namespace('event_handlers')


class CppFilters(FiltersBase):
    @jilter
    def func_args(self, idc: IDCFunc, *, with_output: bool = True) -> str:
        arg_list = []
        if idc.has_inputs:
            for _input in idc.inputs:
                arg_list.append(f'const {_input.typename}& {_input.arg_name}')

        if with_output and isinstance(idc, Endpoint) and idc.has_output:
            arg_list.append(f'{idc.output.typename}& output')
        return ', '.join(arg_list)

    @jilter
    def func_arg_types(self, idc: IDCFunc, *, with_output: bool = True) -> str:
        arg_list = []
        if idc.has_inputs:
            for _input in idc.inputs:
                arg_list.append(f'const {_input.typename}&')

        if with_output and isinstance(idc, Endpoint) and idc.has_output:
            arg_list.append(f'{idc.output.typename}&')
        return ', '.join(arg_list)

    @jilter
    def func_plain_arg_types(self, idc: IDCFunc, *, with_output: bool = True) -> str:
        arg_list = []
        if idc.has_inputs:
            for _input in idc.inputs:
                arg_list.append(_input.typename)

        if with_output and isinstance(idc, Endpoint) and idc.has_output:
            arg_list.append(idc.output)
        return ', '.join(arg_list)

    @jilter
    def shadow_declaration(self, idc: IDCFunc) -> str:
        namespace = idc.scope.parent
        ret = idc.return_type if idc.return_type else 'void'
        name = idc.func_name
        args = self.func_args(idc)
        return f'namespace {namespace} {{ {ret} {name}({args}); }}'

    # RPCs

    @jilter
    def client_rpc_method(self, endpoint: Endpoint) -> str:
        method_name = endpoint.func_name
        inputs_args = f'{self.func_args(endpoint, with_output=False)}, ' if endpoint.has_inputs else ''
        callback = f'Callback<{endpoint.output}> callback' if endpoint.has_output else 'VoidCallback callback'

        return f'{method_name}({inputs_args}{callback})'

    @jilter
    def invoke_handler(self, endpoint: Endpoint) -> str:
        args = self.forward_args(endpoint, obj='in_message', single_arg_name='value', output_arg_name='out_value')
        return f'{endpoint.full_name}({args})'

    @staticmethod
    @jilter
    def rpc_handler_name(endpoint: Endpoint) -> str:
        return endpoint.endpoint_name

    @staticmethod
    @jilter
    def rpc_handler_full_name(endpoint: Endpoint) -> str:
        return str(RPC_HANDLER_NAMESPACE.add_function(endpoint.endpoint_name))

    # events

    @staticmethod
    @jilter
    def event_handler_name(event: Event) -> str:
        return event.event_name

    @staticmethod
    @jilter
    def event_handler_full_name(event: Event) -> str:
        return str(EVENT_HANDLER_NAMESPACE.add_function(event.event_name))

    @staticmethod
    @jilter
    def emitter_name(event: Event) -> str:
        return event.event_name

    @staticmethod
    @jilter
    def dispatcher_name(event: Event) -> str:
        return event.event_name

    @staticmethod
    @jilter
    def dispatcher_full_name(event: Event) -> str:
        return str(EVENT_DISPATCHER_NAMESPACE.add_function(event.event_name))

    @jilter
    def invoke_dispatcher_from_args(self, event: Event) -> str:
        args = self.forward_args(event)
        dispatcher = self.dispatcher_full_name(event)
        return f'{dispatcher}({args})'

    @jilter
    def invoke_dispatcher_from_message(self, event: Event) -> str:
        args = self.forward_args(event, obj='in_message', single_arg_name='value')
        dispatcher = self.dispatcher_full_name(event)
        return f'{dispatcher}({args})'


class CppGenerator(ApiGeneratorBase):
    def __init__(self, api: Api):
        super().__init__(api, tbd.get_tbd_components_root() / 'api' / 'tbd_api', CppFilters(api))

    def write_arduino_client(self, out_dir: Path):
        client_component_dir = tbd.get_tbd_components_root() / 'api' / 'tbd_api' / 'clients' / 'arduino'
        tbb.batch_update_build_files_if_outdated(
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
        tbb.batch_update_build_files_if_outdated(
            client_component_dir,
            out_dir,
            [
                'src/main.cpp',
                'platformio.ini',
            ],
            ignore=['*.pb.h', '*.pb.c'],
        )

        tbb.batch_update_build_files_if_outdated(
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
        tbb.batch_update_build_files_if_outdated(
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

        # self.write_protos(client_protos_file)
        # generate_protos(client_cpp_path, client_cpp_path, [client_protos_file.name])

        source = self.render('src/api_message_transcoding.hpp.j2', client_types=True)
        client_cpp_path.mkdir(exist_ok=True, parents=True)
        out_file = client_cpp_path / 'api_message_transcoding.hpp'
        with open(out_file, 'w') as f:
            f.write(source)




__all__ = ['CppGenerator']