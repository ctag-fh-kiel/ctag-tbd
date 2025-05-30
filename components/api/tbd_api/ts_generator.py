from .dtos import MessagePayload, ParamPayload
from .generator_base import GeneratorBase, jilter
from .enpoints import Endpoint

class TSGenerator(GeneratorBase):
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
        return self._env.get_template('request_func.ts.j2').render(endpoint=endpoint)

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
    def result_value(self, endpoint: Endpoint) -> str:
        if not endpoint.output:
            return 'null'
        else:
            return 'response.output'

    @jilter
    def request_func_return(self, endpoint: Endpoint) -> str:
        if not endpoint.output:
            return 'null'
        return self.payload_type(endpoint.output)

    def request(self, endpoint: Endpoint):
        pass



__all__ = ['TSGenerator']