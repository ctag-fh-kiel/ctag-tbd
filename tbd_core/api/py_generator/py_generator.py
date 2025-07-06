import subprocess
from pathlib import Path

import tbd_core.buildgen as tbb
from tbd_core.api import Api, Endpoint, ApiGeneratorBase, jilter, Event, FiltersBase
from tbd_core.api.idc_interfaces import IDCFunc
from tbd_core.reflection.db import ReflectableDB
from tbd_core.serialization import Serializables, SerializableGenerator

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
        if idc.has_inputs:
            for _input in idc.inputs:
                # arg_type = self.payload_type(arg.type)
                arg_list.append(f'{_input.arg_name}: {_input.type.cls_name}')

        return ', '.join(arg_list)

    @jilter
    def event_signature(self, event: Event):
        arg_list = []
        if event.has_inputs:
            for _input in event.inputs:
                arg_list.append(_input.type.cls_name)

        return ', '.join(arg_list)

    @jilter
    def request_func_return(self, endpoint: Endpoint) -> str:
        if not endpoint.output:
            return 'None'
        return endpoint.output.cls_name


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
            patterns=['*.py'], ignore=['api_types_pb2.py', 'tbd_rpc.py', 'tbd_event.py']
        )
        tbb.update_build_file_if_outdated(srcs_path / BASE_ENDPOINTS_FILE, out_module_dir / BASE_ENDPOINTS_FILE)
        tbb.update_build_file_if_outdated(self._templates / PROJECT_FILE, out_dir / PROJECT_FILE)

        self._generate_protobuf(out_module_dir)
        self._write_rpc_class(out_module_dir)
        self._write_event_class(out_module_dir)

    def _write_rpc_class(self, out_module_dir: Path):
        source = self.render('tbd_rpc.py.j2')

        out_file = out_module_dir / 'tbd_rpc.py'
        with open(out_file, 'w') as f:
            f.write(source)

    def _write_event_class(self, out_module_dir: Path):
        source = self.render('tbd_event.py.j2')

        out_file = out_module_dir / 'tbd_event.py'
        with open(out_file, 'w') as f:
            f.write(source)

    def _generate_protobuf(self, out_module_dir: Path) -> None:
        messages_file = out_module_dir / 'dtos.proto'
        self._serializables.write_protos(messages_file)
        subprocess.run(['protoc', f'--proto_path={out_module_dir}',
                        f'--python_out={out_module_dir}', messages_file.name])


__all__ = ['PyGenerator']