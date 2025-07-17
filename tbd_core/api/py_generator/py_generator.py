import subprocess
from pathlib import Path

import tbd_core.buildgen as tbb
from tbd_core.api import Api, Endpoint, ApiGeneratorBase, jilter, Event, FiltersBase
from tbd_core.api.idc_interfaces import IDCFunc
from tbd_core.reflection.db import ClassPtr
from tbd_core.reflection.reflectables import Param
from tbd_core.serialization import SerializableGenerator

MODULE_NAME = 'tbd_client'
BASE_ENDPOINTS_FILE = 'base_endpoints.py'
PROJECT_FILE = 'pyproject.toml'


class PyFilters(FiltersBase):
    # def payload_type(self, _type) -> str:
    #     payload = self._api.get_payload(payload_type)
    #     match payload:
    #         case MessagePayload():
    #             return f'dtos.{payload_type}'
    #         case ParamPayload():
    #             return payload_type
    #         case _:
    #             raise Exception(f'unknown payload type: {type(payload)}')

    @jilter
    def func_args(self, idc: IDCFunc):
        arg_list = ['self']
        if not idc.has_inputs:
            return ', '.join(arg_list)

        for _input in idc.inputs:
            input_type = _input.type
            match input_type:
                case ClassPtr():
                    arg_type = f'dtos.{input_type.cls_name}'
                case Param():
                    arg_type = input_type.param_type
                case _:
                    raise RuntimeError(f'not a valid IDC argument: {input_type}')
            arg_list.append(f'{_input.arg_name}: {arg_type}')
        return ', '.join(arg_list)

    @jilter
    def endpoint_request(self, endpoint: Endpoint) -> str | None:
        if endpoint.inputs is None:
            return None
        return self._api.get_request(endpoint).cls_name

    @jilter
    def endpoint_response(self, endpoint: Endpoint) -> str | None:
        if endpoint.output is None:
            return None
        return self._api.get_response(endpoint).cls_name

    @jilter
    def request_func_return(self, endpoint: Endpoint) -> str:
        if not endpoint.has_output:
            return 'None'
        return f'dtos.{self._api.get_response(endpoint).cls_name}'

    @jilter
    def event_signature(self, event: Event):
        if not event.has_inputs:
            return ''
        arg_list = []
        for _input in event.inputs:
            input_type = _input.type
            match input_type:
                case ClassPtr():
                    arg_type = f'dtos.{input_type.cls_name}'
                case Param():
                    arg_type = input_type.param_type
                case _:
                    raise RuntimeError(f'not a valid event argument: {input_type}')
            arg_list.append(arg_type)
        return ', '.join(arg_list)

    @jilter
    def event_payload(self, event: Event) -> str | None:
        if event.inputs is None:
            return None
        return self._api.get_payload(event).cls_name


class PyGenerator(ApiGeneratorBase):
    def __init__(self, api: Api, serializables: SerializableGenerator):
        templates_path = Path(__file__).parent / 'py_files'
        super(PyGenerator, self).__init__(api, templates_path, PyFilters(api))
        self._serializables = serializables

    def write_client(self, out_dir: Path) -> None:
        out_dir.mkdir(parents=True, exist_ok=True)
        out_module_dir = out_dir / MODULE_NAME
        out_module_dir.mkdir(exist_ok=True, parents=True)

        srcs_path = Path(__file__).parent.parent
        tbb.update_build_tree_if_outdated(
            srcs_path / MODULE_NAME, out_module_dir,
            patterns=['*.py'], ignore=['api_types_pb2.py', 'tbd_rpc.py', 'tbd_event.py', 'dtos_pb2.py'],
        )
        tbb.update_build_file_if_outdated(srcs_path / BASE_ENDPOINTS_FILE, out_module_dir / BASE_ENDPOINTS_FILE)
        tbb.update_build_file_if_outdated(self._templates / PROJECT_FILE, out_dir / PROJECT_FILE)

        self._generate_protobuf(out_module_dir)
        self._write_rpc_class(out_module_dir)
        self._write_event_class(out_module_dir)

    def _write_rpc_class(self, out_module_dir: Path):
        source = self.render('firmware_rpcs.py.j2')

        out_file = out_module_dir / 'firmware_rpcs.py'
        with open(out_file, 'w') as f:
            f.write(source)

    def _write_event_class(self, out_module_dir: Path):
        source = self.render('firmware_events.py.j2')

        out_file = out_module_dir / 'firmware_events.py'
        with open(out_file, 'w') as f:
            f.write(source)

    def _generate_protobuf(self, out_module_dir: Path) -> None:
        messages_file = out_module_dir / 'dtos.proto'
        self._serializables.write_protos(messages_file)
        subprocess.run(['protoc', f'--proto_path={out_module_dir}',
                        f'--python_out={out_module_dir}', messages_file.name])


__all__ = ['PyGenerator']