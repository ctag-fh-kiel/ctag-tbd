import subprocess
from pathlib import Path

from typing import Callable

from tbd_core.api import Api, Endpoint, GeneratorBase, jilter, MessagePayload, ParamPayload, Event
import tbd_core.buildgen as tbd
from tbd_core.api.idc_interfaces import IDCHandler

MODULE_NAME = 'tbd_client'
BASE_ENDPOINTS_FILE = 'base_endpoints.py'
PROJECT_FILE = 'pyproject.toml'


class PyGenerator(GeneratorBase):
    def __init__(self, api: Api):
        templates_path = Path(__file__).parent / 'py_files'
        super(PyGenerator, self).__init__(api, templates_path)

    def payload_type(self, payload_type: str) -> str:
        payload = self._api.get_payload(payload_type)
        match payload:
            case MessagePayload():
                return f'dtos.{payload_type}'
            case ParamPayload():
                return payload_type
            case _:
                raise Exception(f'unknown payload type: {type(payload)}')


    @jilter
    def request_func(self, endpoint: Endpoint):
        return self._env.get_template('request_func.py.j2').render(endpoint=endpoint)

    @jilter
    def func_args(self, idc: IDCHandler):
        arg_list = ['self']
        if idc.has_args:
            for arg_name, arg_type in idc.args.items():
                arg_type = self.payload_type(arg_type)
                arg_list.append(f'{arg_name}: {arg_type}')

        return ', '.join(arg_list)

    @jilter
    def event_signature(self, event: Event):
        arg_list = []
        if event.has_args:
            for arg_name, arg_type in event.args.items():
                arg_list.append(self.payload_type(arg_type))

        return ', '.join(arg_list)

    @jilter
    def unwrap_response(self, endpoint: Endpoint) -> str:
        response = self._api.get_response(endpoint)
        return 'result.value' if response.is_wrapper else 'result'

    @jilter
    def request_func_return(self, endpoint: Endpoint) -> str:
        if not endpoint.output:
            return 'None'
        return self.payload_type(endpoint.output)

    def write_client(self, out_dir: Path, messages_file: Path):
        out_dir.mkdir(parents=True, exist_ok=True)
        out_module_dir = out_dir / MODULE_NAME
        out_module_dir.mkdir(exist_ok=True, parents=True)

        srcs_path = Path(__file__).parent.parent
        tbd.copy_tree_if_outdated(
            srcs_path / MODULE_NAME, out_module_dir,
            patterns=['*.py'], ignore=['api_types_pb2.py', 'tbd_rpc.py', 'tbd_event.py']
        )
        tbd.copy_file_if_outdated(srcs_path / BASE_ENDPOINTS_FILE, out_module_dir / BASE_ENDPOINTS_FILE)
        tbd.copy_file_if_outdated(self._templates / PROJECT_FILE, out_dir / PROJECT_FILE)

        self._generate_protobuf(out_module_dir, messages_file)
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

    @staticmethod
    def _generate_protobuf(out_module_dir: Path, messages_file: Path):
        messages_dir = messages_file.parent
        subprocess.run(['protoc', f'--proto_path={messages_dir}',
                        f'--python_out={out_module_dir}', messages_file])


__all__ = ['PyGenerator']