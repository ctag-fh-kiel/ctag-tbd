from enum import Enum, unique
from dataclasses import dataclass
from pathlib import Path
from typing import OrderedDict, Literal, get_args
from zlib import crc32

import cxxheaderparser.types as cpptypes

import tbd_core.reflection as tbr
from tbd_core.reflection import Attribute

# expected return types
ENDPOINT_RETURN_TYPES = Literal['Error']
EVENT_RETURN_TYPES = Literal['void']
RESPONDER_RETURN_TYPES = Literal['void', 'Error']
SINK_RETURN_TYPES = Literal['void']

EVENT_PARAM_NAME = 'event'

# known attributes
ENDPOINT_ATTR = 'tbd::endpoint'
EVENT_ATTR = 'tbd::event'
RESPONDER_ATTR = 'tbd::responder'
SINK_ATTR = 'tbd::sink'


@dataclass
class IDCBase:
    func: tbr.FunctionDescription

    @property
    def name(self) -> str:
        """ Plain function name. """
        return self.func.name

    @property
    def full_name(self) -> tbr.ScopeDescription:
        """ Full namespace path to c++ function. """
        return self.func.full_name

    @property
    def return_type(self) -> str:
        return self.func.return_type

    @property
    def description(self) -> str:
        """ Description provided by annotation or `None` """
        return self.func.description

    @property
    def source(self) -> Path:
        """ File containing the declaration or implementation of the endpoint.

            This is the file provided to the C++ parser to extract this endpoint from.
        """
        return self.func.header

@dataclass
class IDCHandler(IDCBase):
    args: OrderedDict[str, str] | None

    @property
    def has_args(self) -> bool:
        return self.args is not None

    def arg_signatures(self) -> str:
        arg_signatures = ''
        if self.has_args:
            args = []
            for arg_name, arg_type in self.args.items():
                args.append(f'{arg_name}:{arg_type}')
            arg_signatures = ','.join(args)
        return arg_signatures


@unique
class EndpointType(Enum):
    FUNCTION = 'FUNCTION'
    GETTER = 'GETTER'
    SETTER = 'SETTER'
    TRIGGER = 'TRIGGER'


@dataclass
class Endpoint(IDCHandler):
    """ Remotely callable procedure.

        Functions annotated with `[[tbd::endpoint(...)]]` that adhere to the following
        argument convention:

        1. Return type has to be `tbd::Error` to signify failures.
        2. Multiple or no `const SomeType&` input arguments (also referred to as inputs) can be present.
        3. A single `SomeOtherType&` non const output (also referred to as return or response) argument can be present.
        4. If both are present the output argument has to be last in the argument list.
        5. Argument types need to be valid TBD DTO types (see DtoTag docs for more)

        Therefore there are four general types of endpoints:

        `FUNCTION`: inputs and output

            tbd::Error some_func(const SomeType& input, SomeOtherType& output);

        `GETTERS`: only output

            tbd::Error some_getter(SomeOtherType& output);

        `SETTERS`: only inputs

            tbd::Error some_getter(const SomeType& input);

        `TRIGGERS`: no arguments

            tbd::Error some_trigger();

    """

    output: str | None = None
    request_type: str | None = None
    response_type: str | None = None

    @property
    def has_output(self) -> bool:
        return self.output is not None

    @property
    def type(self) -> EndpointType:
        if self.has_args and self.has_output:
            return EndpointType.FUNCTION
        elif self.has_output:
            return EndpointType.GETTER
        elif self.has_args:
            return EndpointType.SETTER
        else:
            return EndpointType.TRIGGER

    def signature(self) -> str:
        arg_signatures = self.arg_signatures()
        output_signature = 'void'
        if self.has_output:
            output_signature = self.output
        return f'{self.name}:rpc({arg_signatures})->{output_signature}'


@dataclass
class Event(IDCHandler):
    payload_type: str | None = None
    _event_name: str | None = None

    @property
    def event_name(self) -> str:
        return self._event_name if self._event_name else self.name

    @property
    def has_payload(self) -> bool:
        return self.payload_type is not None

    def signature(self) -> str:
        arg_signatures = self.arg_signatures()
        return f'{self.name}:event({arg_signatures})->void'

    def hash(self):
        return crc32(self.signature().encode())


@dataclass
class EventSink(IDCBase):
    pass


@dataclass
class Responder(IDCHandler):
    event_name: str
    payload_type: str | None = None

    @property
    def has_payload(self) -> bool:
        return self.payload_type is not None


def get_arg_type(arg: cpptypes.Parameter) -> tuple[bool, str]:
    arg_type = arg.type
    if not isinstance(arg_type, cpptypes.Reference):
        raise ValueError('handler arguments have to be references')
    arg_type = arg_type.ref_to
    return arg_type.const, arg_type.typename.format()


def get_idc_args(func_args: list[cpptypes.Parameter]) -> tuple[OrderedDict[str, str] | None, str | None]:
    if not func_args:
        return None, None

    *func_args, last_func_arg = func_args
    args = OrderedDict()

    # process input args
    for func_arg in func_args:
        is_const, arg_type = get_arg_type(func_arg)
        if not is_const:
            raise ValueError('output argument of handler has to be non-const const reference')
        args[func_arg.name] = arg_type

    # process last arg as either input or
    output = None
    is_const, arg_type = get_arg_type(last_func_arg)
    if is_const:
        args[last_func_arg.name] = arg_type
    else:
        output = arg_type

    return args, output


def idc_from_function(func: tbr.FunctionDescription) -> Endpoint | Event | Responder | EventSink | None:
    handler_attrs = [attr for attr in func.attrs if attr.name in [ENDPOINT_ATTR, EVENT_ATTR, RESPONDER_ATTR, SINK_ATTR]]
    if len(handler_attrs) == 0:
        return None
    elif len(handler_attrs) > 1:
        raise ValueError('multiple handler attributes on function')

    attr = handler_attrs[0]
    if attr.name == ENDPOINT_ATTR:
        return endpoint_from_function(func, attr)
    elif attr.name == EVENT_ATTR:
        return event_from_function(func, attr)
    elif attr.name == RESPONDER_ATTR:
        return responder_from_function(func, attr)
    elif attr.name == SINK_ATTR:
        return sink_from_function(func, attr)
    return None


def endpoint_from_function(func: tbr.FunctionDescription, attr: Attribute) -> Endpoint:
    """ Convert compatible C++ function signature to RPC description.
    
        Functions with `tbd::endpoint` attributes will be considered endpoints:

            [[tbd::endpoint]]
            tbd::Error enable_magic() {
                // ...
            }

            [[tbd::endpoint(name="Set Volume")]]
            tbd::Error set_volume(tbd::ufloat_par volume) {
                // ...
            }

        There are 4 types of valid RPC signatures:

        FUNCTION: arguments and return
            `tbd::Error f(const ArgType&, ResType&);`
        GETTER: only arguments
            `tbd::Error f(const ArgType&);`
        SETTER: only return
            `tbd::Error f(ResType&);
        TRIGGER: no arguments and no return
            `tbd::Error`

        :param func: parsed c++ function signature
        :return: enpoint description or `None` if function has no endpoint attribute
    """
    valid_return_types = get_args(ENDPOINT_RETURN_TYPES)
    if func.return_type not in valid_return_types:
        raise ValueError(f'endpoint functions have to return {valid_return_types} type')

    args, output = get_idc_args(func.arguments)
    return Endpoint(
        func=func,
        args=args if args else None,
        output=output,
    )


def event_from_function(func: tbr.FunctionDescription, attr: Attribute) -> Event:
    valid_return_types = get_args(EVENT_RETURN_TYPES)
    if func.return_type not in valid_return_types:
        raise ValueError(f'event triggers declarations have to return {valid_return_types} type')

    args, output = get_idc_args(func.arguments)

    if output is not None:
        raise ValueError('events can not have non const output argument')

    event_name = attr.params.get(EVENT_PARAM_NAME)
    if event_name is not None and not isinstance(event_name, str):
        raise ValueError('event name must be a string')

    return Event(
        func=func,
        args=args if args else None,
        _event_name=event_name,
    )


def responder_from_function(func: tbr.FunctionDescription, attr: Attribute) -> Responder:
    valid_return_types = get_args(RESPONDER_RETURN_TYPES)
    if func.return_type not in valid_return_types:
        raise ValueError(f'event responder functions have to return {valid_return_types} type')

    args, output = get_idc_args(func.arguments)

    if output is not None:
        raise ValueError('responders can not have non const output argument')

    event_name = attr.params.get(EVENT_PARAM_NAME)
    if event_name is None or not isinstance(event_name, str):
        raise ValueError('responders need to be referencing an event')

    return Responder(
        func=func,
        args=args if args else None,
        event_name=event_name,
    )


def sink_from_function(func: tbr.FunctionDescription, attr: Attribute) -> EventSink:
    valid_return_types = get_args(SINK_RETURN_TYPES)
    if func.return_type not in valid_return_types:
        raise ValueError(f'sink functions have to return {valid_return_types} type')

    if len(func.arguments) != 2:
        raise ValueError('sink function must have two arguments')

    buffer_arg, size_arg = func.arguments
    if buffer_arg.type.format() != 'const uint8_t*':
        raise ValueError('first sink function argument must be "const uint8_t*" buffer pointer')
    if size_arg.type.format() != 'size_t':
        raise ValueError('second sink function argument must be "size_t" data length')

    return EventSink(
        func=func,
    )


__all__ = [
    'IDCHandler',
    'EndpointType',
    'Endpoint',
    'Event',
    'Responder',
    'EventSink',
    'idc_from_function',
]