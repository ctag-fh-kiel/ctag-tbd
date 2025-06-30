from pathlib import Path

import tbd_core.buildgen as tbb
from tbd_core.api import MessagePayload, ParamPayload, Api, jilter, Endpoint, FiltersBase
from tbd_core.api.api_generator_base import ApiGeneratorBase


TYPESCRIPT_SRC_DIR = Path('src')


class TSFilters(FiltersBase):
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
    def request_func_args(self, endpoint: Endpoint):
        arg_list = []
        if endpoint.has_args:
            for arg_name, arg_type in endpoint.args.items():
                arg_type = self.payload_type(arg_type)
                arg_list.append(f'{arg_name}: {arg_type}')

        return ', '.join(arg_list)


    @jilter
    def forward_args(self, endpoint: Endpoint) -> list[str]:
        if not endpoint.has_args:
            return []
        if len(endpoint.args) == 1:
            arg_name = next(iter(endpoint.args))
            return [f'input: {arg_name}']

        arg_list = []
        for arg_name in endpoint.args:
            arg_list.append(f'{arg_name}: {arg_name}')
        return arg_list

    @jilter
    def request_func_return(self, endpoint: Endpoint) -> str:
        if not endpoint.output:
            return 'void'
        return self.payload_type(endpoint.output)


class TSGenerator(ApiGeneratorBase):
    def __init__(self, api: Api):
        templates_path = Path(__file__).parent / 'ts_files'
        super(TSGenerator, self).__init__(api, templates_path, TSFilters(api))

    def write_client(self, out_dir: Path, messages_file: Path):
        # symlinks will result in bad esbuild module lookups
        tbb.update_build_tree_if_outdated(self._templates, out_dir, symlink=False, ignore=['*.j2'])
        tbb.update_build_file_if_outdated(messages_file, out_dir / TYPESCRIPT_SRC_DIR / messages_file.name, symlink=False)

        self._write_typescript_client_class(out_dir)

    def _write_typescript_client_class(self, out_srcs_dir: Path):
        source = self.render(TYPESCRIPT_SRC_DIR / 'tbd_client.ts.j2')

        out_srcs_dir.mkdir(exist_ok=True, parents=True)
        out_file = out_srcs_dir / TYPESCRIPT_SRC_DIR / 'client.ts'
        with open(out_file, 'w') as f:
            f.write(source)


__all__ = ['TSGenerator']