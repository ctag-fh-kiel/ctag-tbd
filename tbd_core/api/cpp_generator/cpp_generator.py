from tbd_core.api import Endpoint, jilter, GeneratorBase, Event, Responder
from tbd_core.api.idc_interfaces import IDCBase


def forward_args(idc: Endpoint | Event, *, prefix: str = '', single_arg_name: str | None = None) -> str:
    arg_list = []
    if idc.has_args:
        if single_arg_name and len(idc.args) == 1:
            arg_list.append(f'{prefix}{single_arg_name}')
        else:
            for arg_name, arg_type in idc.args.items():
                arg_list.append(f'{prefix}{arg_name}')

    if isinstance(idc, Endpoint) and idc.has_output:
        arg_list.append('out_value')

    return ', '.join(arg_list)


class CppGenerator(GeneratorBase):
    @staticmethod
    @jilter
    def func_args(idc: IDCBase) -> str:
        arg_list = []
        if idc.has_args:
            for arg_name, arg_type in idc.args.items():
                arg_list.append(f'const {arg_type}& {arg_name}')

        if isinstance(idc, Endpoint) and idc.has_output:
            arg_list.append(f'{idc.output}& output')
        return ', '.join(arg_list)

    @staticmethod
    @jilter
    def shadow_declaration(idc: IDCBase) -> str:
        namespace = idc.full_name.parent
        ret = idc.return_type
        name = idc.name
        args = CppGenerator.func_args(idc)
        return f'namespace {namespace} {{ {ret} {name} ({args}); }}'

    @jilter
    def callback_implementation(self, endpoint:  Endpoint) -> str:
        if endpoint.has_args and endpoint.output == 'str_par':
            return self._env.get_template('str_output_handler_wrapper.cpp.j2').render(endpoint=endpoint)
        else:
            return self._env.get_template('handler_wrappers.cpp.j2').render(endpoint=endpoint)

    @jilter
    def unwrap_response(self, endpoint: Endpoint) -> str:
        response = self._api.get_response(endpoint)
        return 'out_message.value' if response.is_wrapper else 'out_message'

    @jilter
    def forward_args(self, idc: Endpoint | Event) -> str:
        return forward_args(idc)

    @jilter
    def invoke_dispatcher_from_args(self, event: Event) -> str:
        args = forward_args(event)
        dispatcher_name = self.dispatcher_name(event)
        return f'tbd::api::{dispatcher_name}({args})'

    @jilter
    def invoke_dispatcher_from_message(self, event: Event) -> str:
        args = forward_args(event, prefix='in_message.', single_arg_name='input')
        dispatcher_name = self.dispatcher_name(event)
        return f'tbd::api::{dispatcher_name}({args})'

    @jilter
    def invoke_handler(self, endpoint: Endpoint) -> str:
        args = forward_args(endpoint, prefix='in_message.', single_arg_name='input')
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


__all__ = ['CppGenerator']