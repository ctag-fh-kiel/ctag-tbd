import inspect

from .enpoints import Endpoint
from .generator_base import jilter, GeneratorBase


class CppGenerator(GeneratorBase):
    @staticmethod
    @jilter
    def handler_declaration(endpoint: Endpoint) -> str:
        arg_list = []
        if endpoint.has_args:
            for arg_name, arg_type in endpoint.args.items():
                arg_list.append(f'const {arg_type}& {arg_name}')

        if endpoint.has_output:
            arg_list.append(f'{endpoint.output}& output')
        args = ', '.join(arg_list)

        return f'namespace {endpoint.full_name.parent} {{ Error {endpoint.name}({args}); }}'

    @jilter
    def callback_implementation(self, endpoint:  Endpoint) -> str:
        if endpoint.has_args and endpoint.output == 'str_par':
            return self._env.get_template('str_output_handler_wrapper.cpp.j2').render(endpoint=endpoint)
        else:
            return self._env.get_template('handler_wrappers.cpp.j2').render(endpoint=endpoint)

    @jilter
    def invoke_handler(self, endpoint: Endpoint) -> str:
        arg_list = []

        if endpoint.has_args:
            if len(endpoint.args) == 1:
                arg_list.append('in_message.input')
            else:
                for arg_name, arg_type in endpoint.args.items():
                    arg_list.append(f'in_message.{arg_name}')

        if endpoint.has_output:
            if endpoint.output == 'str_par':
                arg_list.append('response_payload')
            else:
                arg_list.append('out_message.output')
        args = ', '.join(arg_list)

        return f'{endpoint.full_name}({args})'

    @staticmethod
    @jilter
    def callback_name(endpoint: Endpoint) -> str:
        return f'handle_{endpoint.name}'

    def _filters(self):
        filters = {}
        for name, _ in inspect.getmembers(self, predicate=inspect.isroutine):
            if name.startswith('_'):
                continue
            method = self.__getattribute__(name)
            if not hasattr(method, '_is_jilter'):
                continue

            sig = inspect.signature(method)
            if len(sig.parameters) != 1:
                raise RuntimeError(f'filter {name} must have exactly 1 parameter')
            filters[name] = method
        return filters


__all__ = ['CppGenerator']