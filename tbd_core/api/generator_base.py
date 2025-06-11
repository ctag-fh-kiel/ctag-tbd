import inspect
import subprocess
from pathlib import Path
from typing import Callable, Any, Final

import jinja2 as ji

from .api import Api
from .idc_interfaces import Endpoint, Event, Responder, IDCHandler


def jilter(f: Callable[[Any], Any]) -> Callable[[Any], Any]:
    """ Mark method as jinja filter. """
    f._is_jilter = True
    return f


def forward_args(idc: IDCHandler, *,
                 prefix: str,
                 single_arg_name: str | None,
                 output_arg_name: str | None) -> str:

    arg_list = []
    if idc.has_args:
        if single_arg_name and len(idc.args) == 1:
            arg_list.append(f'{prefix}{single_arg_name}')
        else:
            arg_list = [f'{prefix}{arg_name}' for arg_name, arg_type in idc.args.items()]

    if output_arg_name and isinstance(idc, Endpoint) and idc.has_output:
        arg_list.append(f'{output_arg_name}')

    return ', '.join(arg_list)


def apply_args(idc: IDCHandler, obj: str, *,
               single_arg_name: str | None,
               output_arg_name: str | None) -> list[str]:

    arg_list = []
    if idc.has_args:
        if single_arg_name and len(idc.args) == 1:
            arg_name = next(iter(idc.args))
            arg_list.append(f'{obj}.{single_arg_name} = {arg_name}')
        else:
            arg_list = [f'{obj}.{arg_name} = {arg_name}' for arg_name, arg_type in idc.args.items()]


    if output_arg_name and isinstance(idc, Endpoint) and idc.has_output:
        arg_list.append(f'{obj}.{output_arg_name} = {output_arg_name}')

    return arg_list


def arg_dict(idc: IDCHandler, *,
             key_sep: str,
             single_arg_name: str | None,
             output_arg_name: str | None) -> list[str]:

    arg_list = []
    if idc.has_args:
        if single_arg_name and len(idc.args) == 1:
            arg_name = next(iter(idc.args))
            arg_list.append(f'{single_arg_name}{key_sep}{single_arg_name} = {arg_name}')
        else:
            arg_list = [f'{arg_name}{key_sep}{arg_name}' for arg_name, arg_type in idc.args.items()]


    if output_arg_name and isinstance(idc, Endpoint) and idc.has_output:
        arg_list.append(f'{output_arg_name}{key_sep}{output_arg_name} = {output_arg_name}')

    return arg_list


def generate_protos(out_dir: Path | str, input_dir: Path | str, input_files: list[Path | str]) -> None:
    out_dir.mkdir(parents=True, exist_ok=True)
    import tbd_core.buildgen.build_deps as build_deps
    build_deps.python_dependencies('nanopb')
    subprocess.run([
        'python', '-m', 'nanopb.generator.nanopb_generator',
        '--output-dir', out_dir,
        '--proto-path', input_dir,
        *input_files,
    ])


class FiltersBase:
    def __init__(self, api: Api):
        self._api: Final[Api] = api

    @jilter
    def forward_args(self, idc: IDCHandler, *,
                     prefix: str = '',
                     single_arg_name: str | None = None,
                     output_arg_name: str | None = None) -> str:
        return forward_args(idc, prefix=prefix, single_arg_name=single_arg_name, output_arg_name=output_arg_name)

    @jilter
    def apply_args(self, idc: IDCHandler, obj: str, *,
                   single_arg_name: str | None = None,
                   output_arg_name: str = None) -> list[str]:
        return apply_args(idc, obj, single_arg_name=single_arg_name, output_arg_name=output_arg_name)

    @jilter
    def arg_dict(self, idc: IDCHandler, *, key_sep: str,
                 single_arg_name: str | None = None,
                 output_arg_name: str = None) -> list[str]:
        return arg_dict(idc, key_sep=key_sep, single_arg_name=single_arg_name, output_arg_name=output_arg_name)

    @jilter
    def unwrap_response(self, endpoint: Endpoint, obj: str) -> str:
        response = self._api.get_response(endpoint)
        return f'{obj}.value' if response.is_wrapper else obj

    @jilter
    def endpoint_id(self, endpoint: Endpoint) -> int:
        return self._api.get_endpoint_id(endpoint.name)

    @jilter
    def request_id(self, endpoint: Endpoint) -> int:
        return -1

    @jilter
    def request_type(self, endpoint: Endpoint) -> str:
        return endpoint.request_type

    @jilter
    def response_id(self, endpoint: Endpoint) -> int:
        return -1

    @jilter
    def response_type(self, endpoint: Endpoint) -> str:
        return endpoint.response_type

    @jilter
    def event_id(self, event: Event) -> int:
        return self._api.get_event_id(event.name)

    @jilter
    def event_payload(self, event: Event) -> str:
        return event.payload_type

    @jilter
    def responders(self, event: Event) -> list[Responder]:
        responders = self._api.get_responders(event.name)
        return responders if responders else []


class GeneratorBase:
    def __init__(self, api: Api, templates_path: Path, filters: FiltersBase | None = None) -> None:
        self._templates: Final[Path] = templates_path
        self._api: Final[Api] = api
        self._env = ji.Environment(loader=ji.FileSystemLoader(templates_path), autoescape=ji.select_autoescape())
        if filters:
            self._filters = self._filters(filters)
            self._env.filters |= self._filters
        else:
            self._filters = None

    def render(self, template_file: Path | str, **args) -> str:
        template = self._env.get_template(str(template_file))
        return template.render(api=self._api, **args)

    @staticmethod
    def _filters(obj):
        filters = {}
        for name, _ in inspect.getmembers(obj, predicate=inspect.isroutine):
            if name.startswith('_'):
                continue
            method = obj.__getattribute__(name)
            if not hasattr(method, '_is_jilter'):
                continue

            # sig = inspect.signature(method)
            # if len(sig.parameters) != 1:
            #     raise RuntimeError(f'filter {name} must have exactly 1 parameter')
            filters[name] = method
        return filters


__all__ = [
    'jilter',
    'forward_args',
    'apply_args',
    'arg_dict',
    'generate_protos',
    'GeneratorBase',
    'FiltersBase',
]