from enum import Enum, unique
from pathlib import Path
import dataclasses
import logging
from collections import OrderedDict

import cxxheaderparser.types as cpptypes
import proto_schema_parser.parser as protop
import proto_schema_parser.ast as proto

import esphome.components.tbd_reflection as tbr


_LOGGER = logging.getLogger(__file__)

REQUEST_WRAPPER_POSTFIX = 'Request'
RESPONSE_WRAPPER_POSTFIX = 'Response'

EMPTY_WRAPPER_NAME = 'Empty'
EMPTY_REQUEST_TYPE = EMPTY_WRAPPER_NAME + REQUEST_WRAPPER_POSTFIX
EMPTY_RESPONSE_TYPE = EMPTY_WRAPPER_NAME + RESPONSE_WRAPPER_POSTFIX
PAYLOAD_FIELD_NAME = 'payload'
HANDLER_RETURN_TYPE = 'uint32_t'

PROTO_HEADER_POSTFIX = '.pb.h'


@unique
class HandlerType(Enum):
    FUNCTION = 'FUNCTION'
    GETTER = 'GETTER'
    SETTER = 'SETTER'
    TRIGGER = 'TRIGGER'


@unique
class WrapperType(Enum):
    MESSAGE = 'MESSAGE'
    REQUEST = 'REQUEST'
    RESPONSE = 'RESPONSE'


@dataclasses.dataclass
class Endpoint:
    func: tbr.FunctionDescription
    in_message: str | None
    out_message: str | None

    @property
    def name(self) -> str:
        return self.func.name

    @property
    def full_name(self) -> tbr.ScopeDescription:
        return self.func.full_name

    @property
    def description(self) -> str:
        return self.func.description

    @property
    def source(self) -> Path:
        return self.func.header

    @property
    def has_arg(self):
        return self.in_message is not None

    @property
    def has_return(self):
        return self.out_message is not None

    @property
    def type(self) -> HandlerType:
        if self.in_message and self.out_message:
            return HandlerType.FUNCTION
        elif self.out_message:
            return HandlerType.GETTER
        elif self.in_message:
            return HandlerType.SETTER
        else:
            return HandlerType.TRIGGER


@dataclasses.dataclass
class Message:
    raw: proto.Message
    path: Path | None

    @property
    def name(self) -> str:
        return self.raw.name

    @property
    def header(self) -> str:
        return self.path.stem
    
    @property
    def elements(self) -> list[proto.MessageElement]:
        return self.raw.elements

    @property
    def payload_type(self) -> str | None:
        for element in self.elements:
            if not isinstance(element, proto.Field):
                continue

            if element.name == PAYLOAD_FIELD_NAME:
                return element.name
        return None

    @property
    def message_name(self) -> str:
        message_name = self.name
        if message_name in [EMPTY_REQUEST_TYPE, EMPTY_RESPONSE_TYPE]:
            return EMPTY_WRAPPER_NAME
        elif message_name.endswith(REQUEST_WRAPPER_POSTFIX) and self.payload_type:
            return message_name[:-len(REQUEST_WRAPPER_POSTFIX)]
        elif message_name.endswith(RESPONSE_WRAPPER_POSTFIX) and self.payload_type:
            return message_name[:-len(RESPONSE_WRAPPER_POSTFIX)]
        else:
            return self.name

    @property
    def wrapper_type(self) -> WrapperType:
        message_name = self.name
        if message_name == EMPTY_REQUEST_TYPE:
            return WrapperType.REQUEST
        elif message_name == EMPTY_RESPONSE_TYPE:
            return WrapperType.RESPONSE
        elif message_name.endswith(REQUEST_WRAPPER_POSTFIX) and self.payload_type:
            return WrapperType.REQUEST
        elif message_name.endswith(RESPONSE_WRAPPER_POSTFIX) and self.payload_type:
            return WrapperType.RESPONSE
        else:
            return WrapperType.MESSAGE

    def next_field_id(self) -> int:
        return max(getattr(field, 'number', 0) for field in self.elements) + 1

    def __hash__(self):
        return hash(self.path)



def get_arg_type(arg: cpptypes.Parameter) -> tuple[bool, str]:
    arg_type = arg.type
    if not isinstance(arg_type, cpptypes.Reference):
        raise ValueError('handler arguments have to be references')
    arg_type = arg_type.ref_to
    return arg_type.const, arg_type.typename.format()


def get_endpoint(func: tbr.FunctionDescription) -> Endpoint | None:
    handler_attrs = [attr for attr in func.attrs if attr.name == 'tbd::endpoint']
    if len(handler_attrs) == 0:
        return None
    elif len(handler_attrs) > 1:
        raise ValueError('multiple handler attributes on function')
            
    if func.return_type != HANDLER_RETURN_TYPE:
        raise ValueError(f'handlers must return "{HANDLER_RETURN_TYPE}"')
    
    num_args = len(func.arguments)
    if num_args == 0:
        in_message = None
        out_message = None
    elif num_args == 1:
        is_const, arg_type = get_arg_type(func.arguments[0])
        if is_const:
            in_message = arg_type
            out_message = None
        else:
            in_message = None
            out_message = arg_type

    elif num_args == 2:
        is_fst_const, fst_arg = get_arg_type(func.arguments[0])
        if not is_fst_const:
            raise ValueError('input argument of handler has to be const reference')
        in_message = fst_arg

        is_snd_const, snd_arg = get_arg_type(func.arguments[1])
        if is_snd_const:
            raise ValueError('output argument of handler has to be non-const const reference')
        out_message = snd_arg

    else:
        raise ValueError('endpoints must have one or two arguments')
    
    return Endpoint(
        func=func,
        in_message=in_message,
        out_message=out_message,
    )


class ApiRegistry:
    def __init__(self):
        self._endpoints: list[Endpoint] = []
        self._message_types: OrderedDict[str, Message] = OrderedDict()
        self._request_types: OrderedDict[str, Message] = OrderedDict()
        self._response_types: OrderedDict[str, Message] = OrderedDict()
        self._predefined_wrapper_names: set[str] = set()
        self._required_proto_files: set[Path] = set()

        self._cached_request_types: list[Message] | None = None
        self._cached_response_types: list[Message] | None = None

    @property
    def endpoints(self) -> list[Endpoint]:
        return self._endpoints
    
    @property
    def message_types(self) -> list[Message]:
        return self._message_types

    @property
    def proto_files(self) -> set[Path]:
        return self._required_proto_files
    
    @property
    def proto_headers(self) -> str:
        return [f'{proto_file.stem}{PROTO_HEADER_POSTFIX}' for proto_file in self.proto_files]

    @property
    def request_types(self) -> list[Message]:
        if not self._cached_request_types:
            required_request_names = set(endpoint.in_message for endpoint in self.endpoints if endpoint.in_message)
            required_requests = [request for request in self._request_types.values() if request.message_name in required_request_names]
            self._cached_request_types = [self._request_types[EMPTY_WRAPPER_NAME], *required_requests]
        return self._cached_request_types
    
    @property
    def response_types(self) -> list[Message]:
        if not self._cached_response_types:
            required_response_names = set(endpoint.out_message for endpoint in self.endpoints if endpoint.out_message)
            required_respoonses = [request for request in self._response_types.values() if request.message_name in required_response_names]
            self._cached_response_types = [self._response_types[EMPTY_WRAPPER_NAME], *required_respoonses]
        return self._cached_response_types
    
    def get_message(self, message_name: str) -> Message:
        return self._message_types[message_name]
    
    def get_message_id(self, message_name: str) -> Message:
        return next(index for index, name in enumerate(self._message_types) if name == message_name)
    
    def get_request(self, message_name: str) -> Message:
        return self._request_types[message_name]

    def get_request_id(self, message_name: str) -> Message:
        return next(index for index, request in enumerate(self.request_types) if request.message_name == message_name)

    def get_response(self, message_name: str) -> Message:
        return self._response_types[message_name]

    def get_response_id(self, message_name: str) -> Message:
        return next(index for index, response in enumerate(self.response_types) if response.message_name == message_name)

    def add_message_types(self, proto_path: Path | str):
        proto_path = Path(proto_path)
        if not proto_path.exists():
            raise ValueError(f'invalid message descriptions file/path {proto_path}')
        
        with open(proto_path, 'r') as f:
            data = f.read()

        messages = [message for message in protop.Parser().parse(data).file_elements if isinstance(message, proto.Message)]
        for message in messages:
            self._add_message(Message(raw=message, path=proto_path))

    def add_endpoints(self, source: Path | str) -> None:
        source = Path(source)
        if not source.exists():
            raise ValueError(f'endpoint source {source} does not exist')
        collector = tbr.ReflectableFinder()
        collector.add_from_file(source)
        for func in collector.funcs:
            self._add_endpoint(func)

    def _add_message(self, message: Message) -> None:
        wrapper_type = message.wrapper_type
        message_name = message.message_name
        if wrapper_type == WrapperType.REQUEST and message_name:
            if message_name in self._request_types:
                raise ValueError(f'duplicate request type {message.name}')
            self._request_types[message_name] = message
            self._cached_request_types = None
        elif wrapper_type == WrapperType.RESPONSE and message_name:
            if message_name in self._response_types:
                raise ValueError(f'duplicate response type {message.name}')
            self._response_types[message_name] = message
            self._cached_response_types = None
        else:
            if message_name in self._message_types:
                raise ValueError(f'duplicate message type {message_name}')
            self._message_types[message_name] = message

    def _add_endpoint(self, func: tbr.FunctionDescription):
        if not (endpoint := get_endpoint(func)):
            return
        
        in_message_name = endpoint.in_message
        if in_message_name:
            in_message = self.get_message(in_message_name)
            if (request_wrapper := self._request_types.get(in_message_name)) and request_wrapper.path:
                self._required_proto_files.add(request_wrapper.path)
            else:
                request_wrapper = self._request_types[in_message_name] = Message(
                    self._make_request_wrapper(in_message),
                    path=None,
                )
                self._cached_response_types = None
            self._required_proto_files.add(in_message.path)

        out_message_name = endpoint.out_message
        if out_message_name:
            out_message = self.get_message(out_message_name)
            if (response_wrapper := self._response_types.get(out_message_name)) and response_wrapper.path: 
                self._required_proto_files.add(response_wrapper.path)
            else:
                response_wrapper = self._response_types[out_message_name] = Message(
                    self._make_request_wrapper(out_message),
                    path=None,
                )
                self._cached_response_types = None
            self._required_proto_files.add(out_message.path)

        self.endpoints.append(endpoint)

    def _make_request_wrapper(self, message: Message) -> proto.Message:
        empty_request = self._request_types[EMPTY_WRAPPER_NAME]
        payload_id = empty_request.next_field_id()

        return proto.Message(
            name=message.name + REQUEST_WRAPPER_POSTFIX,
            elements=[
                *empty_request.elements,
                proto.Field(
                    name=PAYLOAD_FIELD_NAME,
                    type=message.name,
                    number=payload_id,
                )
            ]
        ) 
    
    def _make_response_wrapper(self, message: Message) -> proto.Message:
        empty_response = self._response_types[EMPTY_WRAPPER_NAME]
        payload_id = empty_response.next_field_id()

        return proto.Message(
            name=message.name + RESPONSE_WRAPPER_POSTFIX,
            elements=[
                *empty_response.elements,
                proto.Field(
                    name=PAYLOAD_FIELD_NAME,
                    type=message.name,
                    number=payload_id,
                )
            ]
        ) 
    
__all__ = ['ApiRegistry', 'Endpoint', 'Message']