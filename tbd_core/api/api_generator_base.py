from typing import Final
from pathlib import Path

from tbd_core.generators import jilter, GeneratorBase
from tbd_core.reflection.reflectables import Param

from .api import Api
from .idc_interfaces import Endpoint, IDCFunc, Event, Responder


def needs_param_wrapper(idc: IDCFunc) -> bool:
    return idc.has_inputs and len(idc.inputs) == 1 and isinstance(idc.inputs[0].type, Param)

def needs_output_wrapper(endpoint: Endpoint) -> bool:
    return endpoint.has_output and isinstance(endpoint.output.type, Param)

def needs_request_dto(idc: IDCFunc) -> bool:
    return idc.has_inputs and len(idc.inputs) > 1


def forward_args(idc: IDCFunc, *,
                 obj: str,
                 single_arg_name: str | None,
                 output_arg_name: str | None,
                 dot='.') -> str:

    arg_list = []
    if idc.has_inputs:
        if single_arg_name and len(idc.inputs) == 1:
            if isinstance(idc.inputs[0].type, Param):
                arg_list.append(f'{obj}{dot}{single_arg_name}')
            else:
                arg_list.append(obj)
        else:
            prefix = obj + dot if obj else ''
            arg_list = [f'{prefix}{_input.arg_name}' for _input in idc.inputs]

    if output_arg_name and isinstance(idc, Endpoint) and idc.has_output:
        arg_list.append(f'{output_arg_name}')

    return ', '.join(arg_list)


def apply_args(
        idc: IDCFunc,
        obj: str, *,
        single_arg_name: str | None,
        output_arg_name: str | None
) -> list[str]:

    arg_list = []
    if idc.has_inputs:
        if single_arg_name and len(idc.inputs) == 1 and isinstance(idc.inputs[0].type, Param):
            arg_name = idc.inputs[0].arg_name
            arg_list.append(f'{obj}.{single_arg_name} = {arg_name}')
        else:
            arg_list = [f'{obj}.{_input.arg_name} = {_input.arg_name}' for _input in idc.inputs]


    if output_arg_name and isinstance(idc, Endpoint) and idc.has_output:
        arg_list.append(f'{obj}.{output_arg_name} = {output_arg_name}')

    return arg_list


def arg_dict(idc: IDCFunc, *,
             key_sep: str,
             single_arg_name: str | None,
             output_arg_name: str | None) -> list[str]:

    arg_list = []
    if idc.has_inputs:
        if single_arg_name and len(idc.inputs) == 1 and isinstance(idc.inputs[0].type, Param):
            arg_name = idc.inputs[0].arg_name
            arg_list.append(f'{single_arg_name}{key_sep}{single_arg_name} = {arg_name}')
        else:
            arg_list = [f'{arg_name}{key_sep}{arg_name}' for arg_name, arg_type in idc.args.items()]


    if output_arg_name and isinstance(idc, Endpoint) and idc.has_output:
        arg_list.append(f'{output_arg_name}{key_sep}{output_arg_name} = {output_arg_name}')

    return arg_list


class FiltersBase:
    def __init__(self, api: Api):
        self._api: Final[Api] = api

    @staticmethod
    @jilter
    def needs_param_wrapper(idc: IDCFunc) -> bool:
        return needs_param_wrapper(idc)

    @staticmethod
    @jilter
    def needs_request_dto(idc: IDCFunc) -> bool:
        return needs_request_dto(idc)

    @jilter
    def forward_args(self, idc: IDCFunc, *,
                     obj: str = '',
                     single_arg_name: str | None = None,
                     output_arg_name: str | None = None) -> str:
        return forward_args(idc, obj=obj, single_arg_name=single_arg_name, output_arg_name=output_arg_name)

    @jilter
    def apply_args(self, idc: IDCFunc, obj: str, *,
                   single_arg_name: str | None = None,
                   output_arg_name: str = None) -> list[str]:
        return apply_args(idc, obj, single_arg_name=single_arg_name, output_arg_name=output_arg_name)

    @jilter
    def arg_dict(self, idc: IDCFunc, *, key_sep: str,
                 single_arg_name: str | None = None,
                 output_arg_name: str = None) -> list[str]:
        return arg_dict(idc, key_sep=key_sep, single_arg_name=single_arg_name, output_arg_name=output_arg_name)

    @jilter
    def unwrap_response(self, endpoint: Endpoint, obj: str) -> str:
        return f'{obj}.value' if needs_output_wrapper(endpoint) else obj

    @jilter
    def endpoint_id(self, endpoint: Endpoint) -> int:
        return self._api.get_endpoint_id(endpoint)

    @jilter
    def request_id(self, endpoint: Endpoint) -> int:
        return self._api.get_request_id(endpoint)

    @jilter
    def request_type(self, endpoint: Endpoint) -> str | None:
        request_type = self._api.get_request(endpoint)
        if request_type is None:
            return None
        return str(request_type)

    @jilter
    def output_id(self, endpoint: Endpoint) -> int:
        return self._api.get_response_id(endpoint)

    @jilter
    def response_type(self, endpoint: Endpoint) -> str | None:
        response_type = self._api.get_response(endpoint)
        if response_type is None:
            return None
        return str(response_type)

    @jilter
    def event_id(self, event: Event) -> int:
        return self._api.get_event_id(event)

    @jilter
    def event_payload(self, event: Event) -> str | None:
        if event.inputs is None:
            return None
        return self._api.get_payload(event).full_name

    @jilter
    def responders(self, event: Event) -> list[Responder]:
        responders = self._api.get_responders(event)
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