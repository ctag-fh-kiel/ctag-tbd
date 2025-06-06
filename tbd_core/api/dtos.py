import re
from pathlib import Path
import abc
from typing import Final, OrderedDict, Literal
import proto_schema_parser.ast as proto

import tbd_core.reflection as tbr

REQUEST_ID_FIELD_NAME = 'request_id'
ENDPOINT_FIELD_NAME = 'endpoint'
STATUS_FIELD_NAME = 'status'
SINGLE_ARG_FIELD_NAME = 'input'
WRAPPER_FIELD_NAME = 'value'

REQUEST_NAME_POSTFIX = '_request'
WRAPPER_NAME_POSTFIX = '_wrapper'
EVENT_NAME_POSTFIX = '_event'
WRAPPER_NAME_EXPR = re.compile(r'^\w+(?P<wrapper_type>_request|_wrapper|_event)$')

PARAM_TO_PROTO = {
    tbr.ParamType.INT_PARAM: 'int32',
    tbr.ParamType.UINT_PARAM: 'uint32',
    tbr.ParamType.FLOAT_PARAM: 'float',
    tbr.ParamType.UFLOAT_PARAM: 'float',
    tbr.ParamType.TRIGGER_PARAM: 'bool',
    tbr.ParamType.STR_PARAM: 'string',
}

def is_data_field(element: proto.MessageElement):
    match element:
        case proto.Field():
            return True
        case proto.Group():
            raise ValueError('groups not supported')
        case proto.OneOf():
            raise ValueError('OneOf fields not supported')
    return False


# message base types

class DtoTag(abc.ABC):
    @property
    def name(self) -> str:
        """ Name of type in C++. """
        raise RuntimeError('Not implemented')

    @property
    def proto_type(self) -> proto.MessageType | str:
        """ Protobuf message object or plain type. """
        raise RuntimeError('Not implemented')

    @property
    def proto_type_name(self) -> str:
        """ Name of type in Protobuf. """
        raise RuntimeError('Not implemented')

    @property
    def needs_generating(self) -> bool:
        """ False if no protobuf file for this type exists. """
        raise RuntimeError('Not implemented')

    @property
    def is_builtin(self) -> bool:
        """ True if type is not elementary.
    
            Builtins do not require generating probobuffer entry generation.
        """
        raise RuntimeError('Not implemented')


class MessageDto:
    def __init__(self, proto_type: proto.Message, file_path: Path | None):
        self._proto_type: Final = proto_type
        self._file_path: Final = file_path

    @property
    def name(self) -> str:
        return self.proto_type.name

    @property
    def proto_type(self) -> proto.Message:
        return self._proto_type
    
    @property
    def proto_type_name(self) -> str:
        return self._proto_type.name
    
    @property
    def needs_generating(self) -> bool:
        return self.file_path is None
    
    @property
    def is_builtin(self) -> bool:
        return False

    # specific properties

    @property
    def file_path(self) -> Path | None:
        return self._file_path

    @property
    def file_name(self) -> str:
        return self.file_path.stem

    @property
    def fields(self) -> list[proto.Field]:
        return [element for element in self.proto_type.elements if is_data_field(element)]

    def next_field_id(self) -> int:
        if not self.fields:
            return 1
        return max(getattr(field, 'number', 0) for field in self.fields) + 1

# payloads

class PayloadTag(DtoTag):
    pass

class MessagePayload(MessageDto, PayloadTag):
    """ Protobuffer RPC payload type. 
    
        Message Payloads can be both used as RPC arguments and return values.
    """

class ParamPayload(PayloadTag):
    """ Scalar RPC payload type. 
    
        Scalar payloads can be both used as RPC arguments and return values.
    """
    def __init__(self, param_type: tbr.ParamType):
        self._param_type: Final = param_type

    @property
    def name(self) -> str:
        return self.param_type.value

    @property
    def proto_type(self) -> str:
        return PARAM_TO_PROTO[self.param_type]

    @property
    def proto_type_name(self) -> str:
        return PARAM_TO_PROTO[self.param_type]

    @property
    def needs_generating(self) -> bool:
        return False
    
    @property
    def is_builtin(self) -> bool:
        return True
    
    # specific methods

    @property
    def param_type(self) -> tbr.ParamType:
        return self._param_type

    @staticmethod
    def param_messages() -> list['ParamPayload']:
        return [ParamPayload(param_type=param_type) for param_type in tbr.ParamType]

Payload = PayloadTag


# requests

class RequestBase(MessageDto, DtoTag):
    @property
    def is_builtin(self) -> bool:
        return False

    @property
    def num_args(self) -> int:
        """ Number of arguments. """
        raise RuntimeError('Not implemented')

    @property
    def args(self) -> OrderedDict[str, str] | None:
        """ Payload type name in C++. """
        raise RuntimeError('Not implemented')


class SingleArgRequest(RequestBase):
    @property
    def num_args(self) -> int:
        return 1

    @property
    def args(self) -> OrderedDict[str, str] | None:
        return None


class MultiArgRequest(RequestBase):
    @property
    def num_args(self) -> int:
        return 0

    @property
    def args(self) -> OrderedDict[str, str] | None:
        return None


class ParamRequest(RequestBase):
    def __init__(self, proto_type: proto.Message, param_type: tbr.ParamType):
        super().__init__(proto_type, None)
        self._param_type = param_type

    @property
    def payload(self) -> str:
        return self._param_type.value

    @property
    def param_type(self) -> tbr.ParamType:
        return self.param_type


Request = RequestBase

def request_for_single_arg(payload: Payload) -> Request:
    return _request_for_single_arg(payload, REQUEST_NAME_POSTFIX)

def event_for_single_arg(payload: Payload) -> Request:
    return _request_for_single_arg(payload, EVENT_NAME_POSTFIX)

def _request_for_single_arg(payload: Payload, postfix: str) -> Request:
    message = proto.Message(
        name=payload.name + postfix,
        elements=[
            proto.Field(
                name=SINGLE_ARG_FIELD_NAME,
                type=payload.proto_type_name,
                number=1,
            )
        ]
    )

    match payload:
        case MessagePayload():
            return SingleArgRequest(proto_type=message, file_path=None)
        case ParamPayload():
            return ParamRequest(proto_type=message, param_type=payload.param_type)
        case _:
            raise ValueError(f'can not create response for payload class {type(payload)}')

def request_for_arguments(endpoint_name: str, args: OrderedDict[str, Payload]) -> MultiArgRequest:
    return _request_for_arguments(endpoint_name, args, REQUEST_NAME_POSTFIX)

def event_for_arguments(event_name: str, args: OrderedDict[str, Payload]) -> MultiArgRequest:
    return _request_for_arguments(event_name, args, EVENT_NAME_POSTFIX)

def _request_for_arguments(endpoint_name: str, args: OrderedDict[str, Payload], postfix: str) -> MultiArgRequest:
    arg_fields = [proto.Field(
        name=arg_name,
        type=arg.proto_type_name,
        number=offset + 1,
    ) for offset, (arg_name, arg) in enumerate(args.items())]
    message = proto.Message(
        name=endpoint_name + postfix,
        elements=[*arg_fields]
    )
    return MultiArgRequest(proto_type=message, file_path=None)


# responses

class ResponseBase(MessageDto, DtoTag):
    @property
    def is_builtin(self):
        return False

    @property
    def payload(self) -> str | None:
        """ Payload type name in C++. """
        raise RuntimeError('Not implemented')

    @property
    def is_wrapper(self):
        raise RuntimeError('Not implemented')



class ParamResponse(ResponseBase):
    def __init__(self, proto_type: proto.Message, param_type: tbr.ParamType):
        super().__init__(proto_type, None)
        self._param_type = param_type

    @property
    def payload(self) -> str:
        return self._param_type.value

    @property
    def param_type(self) -> tbr.ParamType:
        return self.param_type

    @property
    def is_wrapper(self):
        return True


class MessageResponse(ResponseBase):
    def __init__(self, payload: MessageDto):
        super().__init__(payload.proto_type, payload.file_path)

    @property
    def payload(self) -> str | None:
        return self.name

    @property
    def is_wrapper(self):
        return False

Response = ResponseBase


def response_for_output(payload: Payload) -> Response:
    match payload:
        case MessagePayload():
           return MessageResponse(payload)
        case ParamPayload():
            message = proto.Message(
                name=payload.name + WRAPPER_NAME_POSTFIX,
                elements=[
                    proto.Field(
                        name=WRAPPER_FIELD_NAME,
                        type=payload.proto_type_name,
                        number=1,
                    )
                ]
            )
            return ParamResponse(proto_type=message, param_type=payload.param_type)
        case _:
            raise ValueError(f'can not create response for payload class {type(payload)}')
 
# 

AnyMessageDto = MessageDto
AnyDto = DtoTag

# constructors

def get_output_type(message: proto.Message) -> str | None:
    for element in message.elements:
        if not is_data_field(element):
            continue

        if element.name == WRAPPER_FIELD_NAME:
            return element.type
    return None


def dto_from_message(message: proto.Message, file_path: Path) -> AnyMessageDto:
    """ Create a DTO abstraction from a plain Protobuf message. """

    if WRAPPER_NAME_EXPR.match(message.name):
        raise ValueError(f'message {message.name}: requests and responses can not be declared explicitly and '
                         f'postfixes {REQUEST_NAME_POSTFIX}, {WRAPPER_NAME_POSTFIX} are prohibited for messages.')

    return MessagePayload(proto_type=message, file_path=file_path)



__all__ = [
    'MessagePayload',
    'ParamPayload',
    'Payload',
    'SingleArgRequest',
    'MultiArgRequest',
    'ParamRequest',
    'Request',
    'request_for_single_arg',
    'request_for_arguments',
    'event_for_single_arg',
    'event_for_arguments',
    'MessageResponse',
    'ParamResponse',
    'Response',
    'response_for_output',
    'AnyMessageDto',
    'AnyDto',
    'dto_from_message',
]