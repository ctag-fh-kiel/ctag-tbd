from tbd_core.api import Endpoint, jilter, GeneratorBase


class CppGenerator(GeneratorBase):
    @staticmethod
    @jilter
    def handler_signature(endpoint: Endpoint) -> str:
        arg_list = []
        if endpoint.has_args:
            for arg_name, arg_type in endpoint.args.items():
                arg_list.append(f'const {arg_type}& {arg_name}')

        if endpoint.has_output:
            arg_list.append(f'{endpoint.output}& output')
        args = ', '.join(arg_list)

        return f'Error {endpoint.name}({args})'

    @staticmethod
    @jilter
    def handler_declaration(endpoint: Endpoint) -> str:
        return f'namespace {endpoint.full_name.parent} {{ {CppGenerator.handler_signature(endpoint)}; }}'

    @jilter
    def callback_implementation(self, endpoint:  Endpoint) -> str:
        if endpoint.has_args and endpoint.output == 'str_par':
            return self._env.get_template('str_output_handler_wrapper.cpp.j2').render(endpoint=endpoint)
        else:
            return self._env.get_template('handler_wrappers.cpp.j2').render(endpoint=endpoint)

    @jilter
    def dispatcher_implementation(self, event:  Endpoint) -> str:
        if event.has_args and event.output == 'str_par':
            raise ValueError('strings not allowed as event parameters')
        else:
            return self._env.get_template('dispatcher_wrapper.cpp.j2').render(event=event)

    @jilter
    def unwrap_response(self, endpoint: Endpoint) -> str:
        response = self._api.get_response(endpoint)
        return 'out_message.value' if response.is_wrapper else 'out_message'


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
            arg_list.append('out_value')

        args = ', '.join(arg_list)

        return f'{endpoint.full_name}({args})'

    @staticmethod
    @jilter
    def callback_name(endpoint: Endpoint) -> str:
        return f'handle_{endpoint.name}'

    @staticmethod
    @jilter
    def dispatcher_name(event: Endpoint) -> str:
        return f'dispatch_{event.name}'


__all__ = ['CppGenerator']