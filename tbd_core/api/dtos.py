import re
from enum import IntEnum, unique
from pathlib import Path
import abc
from typing import Final, OrderedDict
import proto_schema_parser.ast as proto

import tbd_core.reflection as tbr

#### defines ####

REQUEST_ID_FIELD_NAME = 'request_id'
ENDPOINT_FIELD_NAME = 'endpoint'
STATUS_FIELD_NAME = 'status'
SINGLE_ARG_FIELD_NAME = 'input'
WRAPPER_FIELD_NAME = 'value'

REQUEST_NAME_POSTFIX = '_args'
WRAPPER_NAME_POSTFIX = '_wrapper'
EVENT_NAME_POSTFIX = '_payload'
WRAPPER_NAME_EXPR = re.compile(r'^\w+(?P<wrapper_type>_request|_wrapper|_event)$')


def is_data_field(element: proto.MessageElement) -> bool:
    """ Check if element in Protobuf message AST is allowed scalar field type.

        Signals if a message element can be ignored and raises error for non-allowed data fields.

        Message elements fall into three categories:

        1. scalar data fields: primitives and nested messages
        2. non scalar fields: groups, unions, repeated fields, mapped fields
        3. non data elements: comments, enum definitions, ...

        :return True if element is allowed scalar field type, False if element can be ignored
    """
    match element:
        case proto.Field():
            if (cardinality := element.cardinality) and cardinality == proto.FieldCardinality.REPEATED:
                raise ValueError('repeated types are not supported')
            return True
        case proto.Group():
            raise ValueError('groups not supported')
        case proto.OneOf():
            raise ValueError('OneOf fields not supported')
    return False


@unique
class TranscodableCategory(IntEnum):
    VOID_TYPE    = 0   # no fields, can be transmitted as emtpy payload
    PARAM_TYPE   = 1   # TBD parameter type, subset of proto scalar types
    MESSAGE_TYPE = 2   # plain struct type, can be sent as RPC responses or be nested within other message types




# message base types

class Transcodable(abc.ABC):
    """ Base class for all encodable or decodable TBD DTOs.

        Transcodable types can extend the tbd::api::encode_message or tbd::api::decode_message templates, for transport
        encoding/decoding.
    """

    @property
    def name(self) -> str:
        """ Name of type in C++. """
        raise RuntimeError('Not implemented')

    @property
    def proto_type(self) -> proto.Message | str:
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


class TransmittableParam:
    """ Mixin for all TBD parameter types that are transmittable.

        All TBD parameter types are defined in `tbd/parameter_types.hpp` and their mapping to protobuf types is defined
        in PARAM_TO_PROTO.

        TBD transmittable parameters do not have to be directly encodable/decodable, they can also be nested within
        encodable/decodable message types (descended from Transcodable).
    """

    def __init__(self, param_type: tbr.ParamType):
        self._param_type: Final = param_type

    @property
    def name(self) -> str:
        return self._param_type.value

    @property
    def proto_type(self) -> str:
        return tbr.PARAM_TO_PROTO[self._param_type]

    @property
    def proto_type_name(self) -> str:
        return tbr.PARAM_TO_PROTO[self._param_type]

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


class TransmittableStruct:
    """ Mixin for all TBD struct types, that are transmittable.

        The fields of transmittable struct can on only be TBD param types or other transmittable structs.

        Transmittable structs do not have to be directly encodable/decodable, they can also be nested within
        encodable/decodable message types (descended from Transcodable).
    """

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

class PayloadTag(Transcodable):
    """ Api reflectable function argument types.

        This class tags payload types for use in switch clauses.

        Payload types represent API reflection function argument types. Any type taken as inputs (const reference
        arguments) or outputs (non-const reference final argument) needs to be collected by API as PayloadTag type.

        Depending on how the type and how it is used, it can:

        - be added to protobuf message descriptions
        - have tbd::api::encode_message specialization generated
        - have tbd::api::decode_message specialization generated
    """
    pass

class MessagePayload(TransmittableStruct, PayloadTag):
    """ Struct payload type. """

class ParamPayload(TransmittableParam, PayloadTag):
    """ TBD parameter payload type. """

    @staticmethod
    def param_messages() -> list['ParamPayload']:
        """ Return list of payloads for every TBD parameter type."""
        return [ParamPayload(param_type=param_type) for param_type in tbr.ParamType]

Payload = PayloadTag


# requests

class RequestBase(TransmittableStruct, Transcodable):
    """ API argument list for remote function invocation.

        The TBD API system allows calling different kinds of external function calls and calling functions in a loosely
        coupled way (through messages) withing the firmware. Requests are the transcoding format for the input
        arguments of externally callable functions.

        Requests map an argument lists to a transmittable struct type.
    """

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
    """ Argument list consisting of a single argument.

        Single arguments are treated differently from multi-argument lists, to avoid too much code bloat. If only a
        single input argument is present, a common payload is shared among all functions with the same argument and the
        argument is stored in a field named `input`.
    """
    @property
    def num_args(self) -> int:
        return 1

    @property
    def args(self) -> OrderedDict[str, str] | None:
        raise RuntimeError(f'single argument types have no argument name associations, argument field name is {SINGLE_ARG_FIELD_NAME}')


class ParamRequest(SingleArgRequest):
    """ Argument list consisting of a single TBD parameter.

        Special case of single argument lists, for TBD parameter types. Intended for transport optimization.
    """
    def __init__(self, proto_type: proto.Message, param_type: tbr.ParamType):
        super().__init__(proto_type, None)
        self._param_type = param_type

    @property
    def param_type(self) -> tbr.ParamType:
        return self._param_type


class MultiArgRequest(RequestBase):
    """ Argument list for functions with more than one argument.

        The ordering and field names of the transmittable struct match the argument list of the function
        declaration/implementation encountered by the reflection parser.
    """
    @property
    def num_args(self) -> int:
        return len(self._proto_type.elements)

    @property
    def args(self) -> OrderedDict[str, str] | None:
        return OrderedDict((field.name, field.type) for field in self._proto_type.elements)


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

class ResponseBase(TransmittableStruct, Transcodable):
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
    def __init__(self, payload: TransmittableStruct):
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

AnyMessageDto = TransmittableStruct
AnyDto = Transcodable

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