from pathlib import Path
from typing import Final

from .api import Api
from .idc_interfaces import Endpoint, Event, Responder, IDCHandler
from tbd_core.generators import jilter, GeneratorBase


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


class ApiGeneratorBase(GeneratorBase[FiltersBase]):
    def __init__(self, api: Api, templates_path: Path, filters: FiltersBase | None = None) -> None:
        super().__init__(templates_path, filters)
        self._api: Final[Api] = api

    def render(self, template_file: Path | str, **args) -> str:
        return super().render(template_file, api=self._api, **args)


__all__ = [
    'jilter',
    'forward_args',
    'apply_args',
    'arg_dict',
    'ApiGeneratorBase',
    'FiltersBase',
]