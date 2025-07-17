from pathlib import Path

import tbd_core.buildgen as tbb
from tbd_core.api import Api, jilter, Endpoint, FiltersBase
from tbd_core.api.api_generator_base import ApiGeneratorBase
from tbd_core.serialization import SerializableGenerator

TYPESCRIPT_SRC_DIR = Path('src')


class TSFilters(FiltersBase):
    # def payload_type(self, payload_type: str) -> str:
    #     payload = self._api.get_payload(payload_type)
    #     match payload:
    #         case MessagePayload():
    #             return f'dtos.{payload_type}'
    #         case ParamPayload():
    #             return payload_type
    #         case _:
    #             raise Exception(f'unknown payload type: {type(payload)}')

    @jilter
    def request_func_args(self, endpoint: Endpoint):
        arg_list = []
        if endpoint.has_inputs:
            for _input in endpoint.inputs:
                # arg_type = self.payload_type()
                arg_list.append(f'{_input.arg_name}: {_input.cls_name}')

        return ', '.join(arg_list)


    @jilter
    def forward_args(self, endpoint: Endpoint) -> list[str]:
        if not endpoint.has_inputs:
            return []
        inputs = endpoint.inputs
        if len(inputs) == 1:
            input_name = inputs[0].arg_name
            return [f'input: {input_name}']

        arg_list = []
        for _input in inputs:
            arg_list.append(f'{_input.arg_name}: {_input.cls_name}')
        return arg_list

    @jilter
    def request_func_return(self, endpoint: Endpoint) -> str:
        if not endpoint.has_output:
            return 'void'
        return endpoint.output.cls_name


class TSGenerator(ApiGeneratorBase):
    def __init__(self, api: Api, serializables: SerializableGenerator):
        templates_path = Path(__file__).parent / 'ts_files'
        super(TSGenerator, self).__init__(api, templates_path, TSFilters(api))
        self._serializables = serializables

    def write_client(self, out_dir: Path):
        # symlinks will result in bad esbuild module lookups
        tbb.update_build_tree_if_outdated(self._templates, out_dir, symlink=False, ignore=['*.j2'])
        self._serializables.write_protos(out_dir / TYPESCRIPT_SRC_DIR / 'dtos.proto')

        self._write_typescript_client_class(out_dir)

    def _write_typescript_client_class(self, out_srcs_dir: Path):
        source = self.render(TYPESCRIPT_SRC_DIR / 'tbd_client.ts.j2')

        out_srcs_dir.mkdir(exist_ok=True, parents=True)
        out_file = out_srcs_dir / TYPESCRIPT_SRC_DIR / 'client.ts'
        with open(out_file, 'w') as f:
            f.write(source)


__all__ = ['TSGenerator']