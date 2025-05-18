from dataclasses import dataclass
from enum import Enum, unique
from pathlib import Path
import abc
from typing import Final
import proto_schema_parser.ast as proto

import esphome.components.tbd_reflection as tbr

REQUEST_ID_FIELD_NAME = 'request_id'
ENDPOINT_FIELD_NAME = 'endpoint'
STATUS_FIELD_NAME = 'status'
PAYLOAD_FIELD_NAME = 'payload'

REQUEST_NAME_POSTFIX = '_request'
RESPONSE_NAME_POSTFIX = '_response'

EMPTY_PAYLOAD_NAME = 'void'
EMPTY_REQUEST_NAME = EMPTY_PAYLOAD_NAME + REQUEST_NAME_POSTFIX
EMPTY_RESPONSE_NAME = EMPTY_PAYLOAD_NAME + RESPONSE_NAME_POSTFIX

PARAM_TO_PROTO = {
    tbr.ParamType.INT_PARAM: 'int32',
    tbr.ParamType.UINT_PARAM: 'uint32',
    tbr.ParamType.FLOAT_PARAM: 'float',
    tbr.ParamType.UFLOAT_PARAM: 'float',
    tbr.ParamType.TRIGGER_PARAM: 'bool',
    tbr.ParamType.STR_PARAM: 'string',
}

@unique
class WrapperType(Enum):
    MESSAGE = 'MESSAGE'
    REQUEST = 'REQUEST'
    RESPONSE = 'RESPONSE'


class DtoTag(abc.ABC):
    @property
    def name(self) -> str:
        """ Name of type in C++. """

    @property
    def proto_type(self) -> proto.MessageType | str:
        """ Protobuf message object or plain type. """

    @property
    def proto_type_name(self) -> str:
        """ Name of type in Protobuf. """

    @property
    def needs_generating(self) -> bool:
        """ False if no protobuf file for this type exists. """

    @property
    def is_builtin(self) -> bool:
        """ True if type is not elementary. 
    
            Builtins do not require generating probobuffer entry generation.
        """


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
    def elements(self) -> proto.MessageElement:
        return self.proto_type.elements

    def next_field_id(self) -> int:
        return max(getattr(field, 'number', 0) for field in self.elements) + 1

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

    def scalar_messages() -> list['ParamPayload']:
        return [ParamPayload(param_type=param_type) for param_type in tbr.ParamType]

Payload = PayloadTag


# transfer messages

class DtoWrapperTag(DtoTag):
    @property
    def is_builtin(self):
        return False

    @property
    def payload(self) -> str | None:
        """ Payload type name in C++. """


class DtoWithMessagePayload(MessageDto):
    @property
    def payload(self) -> str:
        if not (payload_name := get_payload_type(self.proto_type)):
            raise ValueError(f'message response {self.name} nas not payload')
        return payload_name


class DtoWithParamPayload(MessageDto):
    def __init__(self, proto_type: proto.Message, param_type: tbr.ParamType):
        super().__init__(proto_type, None)
        self._param_type = param_type

    @property
    def payload(self) -> str:
        return self._param_type.value

    @property
    def param_type(self) -> tbr.ParamType:
        return self.param_type


class DtoWithEmptyPayload(MessageDto):
    @property
    def payload(self) -> None:
        return None


# requests

class RequestTag(DtoWrapperTag):
    pass



class EmptyRequest(DtoWithEmptyPayload, RequestTag):
    pass

empty_request = EmptyRequest(
    proto_type=proto.Message(
        name=EMPTY_REQUEST_NAME,
        elements=[
            proto.Field(name=REQUEST_ID_FIELD_NAME, number=1, type='uint32'),
            proto.Field(name=ENDPOINT_FIELD_NAME, number=2, type='uint32'),
            # proto.Field(name=PAYLOAD_FIELD_NAME, number=3, type='google.protobuf.Any'),
        ]
    ),
    file_path=None,
)


class MessageRequest(DtoWithMessagePayload, RequestTag):
    pass


class ParamRequest(DtoWithParamPayload, RequestTag):
    pass


Request = RequestTag


def request_for_payload(payload: Payload) -> Request:
    message = proto.Message(
        name=payload.name + REQUEST_NAME_POSTFIX,
        elements=[
            *empty_request.elements,
            proto.Field(
                name=PAYLOAD_FIELD_NAME,
                type=payload.proto_type_name,
                number=empty_request.next_field_id(),
            )
        ]
    )
    
    match payload:
        case MessagePayload():
           return MessageRequest(proto_type=message, file_path=None) 
        case ParamPayload():
            return ParamRequest(proto_type=message, param_type=payload.param_type)
        case _:
            raise ValueError(f'can not create response for payload class {type(payload)}')


# responses

class ResponseTag(DtoWrapperTag):
    pass


class EmptyResponse(DtoWithEmptyPayload, ResponseTag):
    pass

empty_response = EmptyResponse(
    proto_type=proto.Message(
        name=EMPTY_RESPONSE_NAME,
        elements=[
            proto.Field(name=REQUEST_ID_FIELD_NAME, number=1, type='uint32'),
            proto.Field(name=STATUS_FIELD_NAME, number=2, type='uint32'),
            # proto.Field(name=PAYLOAD_FIELD_NAME, number=3, type='google.protobuf.Any')
        ]
    ),
    file_path=None,
)


class MessageResponse(DtoWithMessagePayload, ResponseTag):
    pass


class ParamResponse(DtoWithParamPayload, ResponseTag):
    pass


Response = ResponseTag


def response_for_payload(payload: Payload) -> Response:
    message = proto.Message(
        name=payload.name + RESPONSE_NAME_POSTFIX,
        elements=[
            *empty_response.elements,
            proto.Field(
                name=PAYLOAD_FIELD_NAME,
                type=payload.proto_type_name,
                number=empty_response.next_field_id(),
            )
        ]
    )
    
    match payload:
        case MessagePayload():
           return MessageResponse(proto_type=message, file_path=None) 
        case ParamPayload():
            return ParamResponse(proto_type=message, param_type=payload.param_type)
        case _:
            raise ValueError(f'can not create response for payload class {type(payload)}')
 
# 

AnyMessageDto = MessageDto
AnyDto = DtoTag

# constructors

def get_payload_type(message: proto.Message) -> str | None:
    for element in message.elements:
        if not isinstance(element, proto.Field):
            continue

        if element.name == PAYLOAD_FIELD_NAME:
            return element.type
    return None


def dto_from_message(message: proto.Message, file_path: Path) -> AnyMessageDto:
    message_name = message.name

    if message_name.endswith(REQUEST_NAME_POSTFIX) and (get_payload_type(message)):
        type_name = message_name[:-len(REQUEST_NAME_POSTFIX)]
        if type_name in tbr.PARAM_TYPES:
            return ParamRequest(proto_type=message, param_type=tbr.PARAM_TYPES[type_name])
        else:
            return MessageRequest(proto_type=message, file_path=file_path)
        
    elif message_name.endswith(RESPONSE_NAME_POSTFIX) and get_payload_type(message):
        type_name = message_name[:-len(RESPONSE_NAME_POSTFIX)]
        if type_name in tbr.PARAM_TYPES:
            return ParamResponse(proto_type=message, param_type=tbr.PARAM_TYPES[type_name])    
        else:
            return MessageResponse(proto_type=message, file_path=file_path)
    else:
        return MessagePayload(proto_type=message, file_path=file_path)


__all__ = [
    'WrapperType',
    'MessagePayload',
    'ParamPayload',
    'Payload',
    'empty_request',
    'MessageRequest',
    'ParamRequest',
    'Request',
    'request_for_payload',
    'empty_response',
    'MessageResponse',
    'ParamResponse',
    'Response',
    'response_for_payload',
    'AnyMessageDto',
    'AnyDto',
    'dto_from_message',
]