from enum import Enum, unique
from dataclasses import dataclass
from typing import Final
from zlib import crc32

from tbd_core.reflection.db import FunctionPtr, ArgumentPtr, ReflectableDB
from tbd_core.reflection.reflectables import Attribute, ArgumentCategory, AttributeOptions

# expected return types
ENDPOINT_RETURN_TYPES = [None, 'Error']
EVENT_RETURN_TYPES = [None]
RESPONDER_RETURN_TYPES = [None, 'Error']
SINK_RETURN_TYPES = [None]

EVENT_PARAM_NAME = 'event'

# known attributes
ENDPOINT_ATTR = 'tbd::endpoint'
EVENT_ATTR = 'tbd::event'
RESPONDER_ATTR = 'tbd::responder'
SINK_ATTR = 'tbd::sink'


class IDCFunc(FunctionPtr):
    def __init__(self, idc_attr: str, _id: int, _db: ReflectableDB):
        super().__init__(_id, _db)
        self._idc_attr: Final[str] = idc_attr

    @property
    def options(self) -> AttributeOptions:
        for attr in self.attrs.values():
            if attr.name == self._idc_attr:
                return attr.options
        raise RuntimeError(f'IDC attribute {self._idc_attr} is missing from attribute list')

    @property
    def has_inputs(self) -> bool:
        return self.inputs is not None

    @property
    def inputs(self) -> list[ArgumentPtr] | None:
        if not self.has_arguments:
            return None
        args = self.arguments
        if args[-1].category == ArgumentCategory.OUTPUT:
            args = args[:-1]
        return args if len(args) > 0 else None

    @property
    def input_typenames(self) -> list[str] | None:
        if not self.has_inputs:
            return None
        return [_type.typename for _type in self.inputs]

    @property
    def input_cls_names(self) -> list[str] | None:
        if not self.has_inputs:
            return None
        return [_type.cls_name for _type in self.inputs]

    def arg_signatures(self) -> str:
        arg_signatures = ''
        if self.has_arguments:
            args = []
            for argument in self.arguments:
                args.append(f'{argument.arg_name}:{argument.typename}')
            arg_signatures = ','.join(args)
        return arg_signatures


@unique
class EndpointType(Enum):
    FUNCTION = 'FUNCTION'
    GETTER = 'GETTER'
    SETTER = 'SETTER'
    TRIGGER = 'TRIGGER'


@dataclass
class Endpoint(IDCFunc):
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

    def __init__(self, func: FunctionPtr):
        super().__init__(ENDPOINT_ATTR, func._id, func._db)

    @property
    def endpoint_name(self) -> str:
        return self.func_name

    @property
    def has_output(self) -> bool:
        return self.output is not None

    @property
    def output(self) -> ArgumentPtr | None:
        args = self.arguments
        if not args:
            return None
        if args[-1].category != ArgumentCategory.OUTPUT:
            return None
        return args[-1]
    
    @property
    def output_typename(self) -> str | None:
        if (output := self.output) is not None:
            return output.typename
        return None

    @property
    def output_cls_name(self) -> str | None:
        if (output := self.output) is not None:
            return output.cls_name
        return None

    @property
    def type(self) -> EndpointType:
        if self.has_inputs and self.has_output:
            return EndpointType.FUNCTION
        elif self.has_output:
            return EndpointType.GETTER
        elif self.has_inputs:
            return EndpointType.SETTER
        else:
            return EndpointType.TRIGGER

    def signature(self) -> str:
        arg_signatures = self.arg_signatures()
        output_signature = 'void'
        if self.has_output:
            output_signature = self.output
        return f'{self.name}:rpc({arg_signatures})->{output_signature}'

    def __repr__(self) -> str:
        return f'Endpoint({self.func_name})'

@dataclass
class Event(IDCFunc):
    def __init__(self, func: FunctionPtr):
        super().__init__(EVENT_ATTR, func._id, func._db)

    @property
    def event_name(self) -> str:
        name = self.options.get(EVENT_PARAM_NAME)
        return name if name is not None else self.func_name

    def signature(self) -> str:
        arg_signatures = self.arg_signatures()
        return f'{self.name}:event({arg_signatures})->void'

    def hash(self):
        return crc32(self.signature().encode())

    def __repr__(self) -> str:
        return f'Event({self.event_name, self.func_name})'

@dataclass
class EventSink(FunctionPtr):
    def __init__(self, func: FunctionPtr):
        super().__init__(func._id, func._db)

    def __repr__(self) -> str:
        return f'EventSink({self.func_name})'

@dataclass
class Responder(IDCFunc):
    def __init__(self, func: FunctionPtr):
        super().__init__(RESPONDER_ATTR, func._id, func._db)
        self.payload_type: str | None = None

    @property
    def event_name(self) -> str:
        name = self.options.get(EVENT_PARAM_NAME)
        return name if name is not None else self.func_name

    @property
    def has_payload(self) -> bool:
        return self.payload_type is not None

    def __repr__(self) -> str:
        return f'Responder({self.func_name})'


def check_idc_args(func_args: list[ArgumentPtr], *, allow_output: bool) -> None:
    if not func_args:
        return None

    *func_args, last_func_arg = func_args
    for func_arg in func_args:
        if func_arg.category != ArgumentCategory.INPUT:
            raise ValueError('input argument of IDC has to be const reference')

    if last_func_arg.category not in [ArgumentCategory.INPUT, ArgumentCategory.OUTPUT]:
        raise ValueError('last argument of IDC has to be reference')

    if not allow_output and last_func_arg.category == ArgumentCategory.OUTPUT:
        raise ValueError('IDC type does not expect output argument')

    return None


def idc_from_function(func: FunctionPtr) -> Endpoint | Event | Responder | EventSink | None:
    handler_attrs = [attr for attr_name, attr in func.attrs.items() if attr_name in [ENDPOINT_ATTR, EVENT_ATTR, RESPONDER_ATTR, SINK_ATTR]]
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


def endpoint_from_function(func: FunctionPtr, attr: Attribute) -> Endpoint:
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
    if func.return_type and func.return_type.typename not in ENDPOINT_RETURN_TYPES:
        raise ValueError(f'endpoint functions have to return {ENDPOINT_RETURN_TYPES} type')

    check_idc_args(func.arguments, allow_output=True)
    return Endpoint(func=func)


def event_from_function(func: FunctionPtr, attr: Attribute) -> Event:
    if func.return_type:
        raise ValueError(f'event triggers declarations have to return void')

    check_idc_args(func.arguments, allow_output=False)

    event_name = attr.options.get(EVENT_PARAM_NAME)
    if event_name is not None and not isinstance(event_name, str):
        raise ValueError('event name must be a string')

    return Event(func=func)


def responder_from_function(func: FunctionPtr, attr: Attribute) -> Responder:
    if func.return_type and func.return_type.typename not in RESPONDER_RETURN_TYPES:
        raise ValueError(f'event responder functions have to return {RESPONDER_RETURN_TYPES} type')

    check_idc_args(func.arguments, allow_output=False)

    event_name = attr.options.get(EVENT_PARAM_NAME)
    if event_name is None or not isinstance(event_name, str):
        raise ValueError('responders need to be referencing an event')

    return Responder(func=func)


def sink_from_function(func: FunctionPtr, attr: Attribute) -> EventSink:
    if func.return_type:
        raise ValueError(f'sink functions have to return void')

    if len(func.arguments) != 2:
        raise ValueError('sink function must have two arguments')

    buffer_arg, size_arg = func.argument_typenames
    if buffer_arg != 'const uint8_t*':
        raise ValueError('first sink function argument must be "const uint8_t*" buffer pointer')
    if size_arg != 'size_t':
        raise ValueError('second sink function argument must be "size_t" data length')

    return EventSink(func=func)


__all__ = [
    'IDCFunc',
    'EndpointType',
    'Endpoint',
    'Event',
    'Responder',
    'EventSink',
    'idc_from_function',
]