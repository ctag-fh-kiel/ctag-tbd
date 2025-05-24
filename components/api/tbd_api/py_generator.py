from unittest import case

from .enpoints import Endpoint
from .generator_base import GeneratorBase, jilter
from .dtos import MessagePayload, ParamPayload
# {% macro set_payload() %}
# if isinstance(request.payload, proto.Message):
#     request.payload.MergeFrom(req_message)
# else:
#     request.payload = req_message
# {% endmacro %}



class PyGenerator(GeneratorBase):
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
    def request_func_args(self, endpoint: Endpoint):
        arg_list = ['self']
        if endpoint.has_args:
            for arg_name, arg_type in endpoint.args.items():
                arg_type = self.payload_type(arg_type)
                arg_list.append(f'{arg_name}: {arg_type}')

        return ', '.join(arg_list)


    @jilter
    def forward_args(self, endpoint: Endpoint):
        if not endpoint.has_args:
            return ''
        if len(endpoint.args) == 1:
            return 'request.input'

        arg_list = []
        for arg_name in endpoint.args:
            arg_list.append(f'request.{arg_name} = {arg_name}')

        return '\n'.join(arg_list)

    @jilter
    def result_value(self, endpoint: Endpoint) -> str:
        if not endpoint.output:
            return 'None'
        else:
            return 'result.output'

    @jilter
    def request_func_return(self, endpoint: Endpoint) -> str:
        if not endpoint.output:
            return 'None'
        return self.payload_type(endpoint.output)

    def request(self, endpoint: Endpoint):
        pass



__all__ = ['PyGenerator']