from enum import Enum, unique
from dataclasses import dataclass
from pathlib import Path
from typing import OrderedDict

import cxxheaderparser.types as cpptypes

import esphome.components.tbd_reflection as tbr


HANDLER_RETURN_TYPE = 'uint32_t'


@unique
class EndpointType(Enum):
    FUNCTION = 'FUNCTION'
    GETTER = 'GETTER'
    SETTER = 'SETTER'
    TRIGGER = 'TRIGGER'


@dataclass
class Endpoint:
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

    func: tbr.FunctionDescription
    args: OrderedDict[str, str] | None
    output: str | None
    request_type: str | None = None
    response_type: str | None = None

    @property
    def name(self) -> str:
        """ Plain function name. """
        return self.func.name

    @property
    def full_name(self) -> tbr.ScopeDescription:
        """ Full namespace path to c++ function. """
        return self.func.full_name

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

    @property
    def has_args(self) -> bool:
        return self.args is not None

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
        

def get_arg_type(arg: cpptypes.Parameter) -> tuple[bool, str]:
    arg_type = arg.type
    if not isinstance(arg_type, cpptypes.Reference):
        raise ValueError('handler arguments have to be references')
    arg_type = arg_type.ref_to
    return arg_type.const, arg_type.typename.format()


def endpoint_from_function(func: tbr.FunctionDescription) -> Endpoint | None:
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

    handler_attrs = [attr for attr in func.attrs if attr.name == 'tbd::endpoint']
    if len(handler_attrs) == 0:
        return None
    elif len(handler_attrs) > 1:
        raise ValueError('multiple handler attributes on function')
            
    if func.return_type != HANDLER_RETURN_TYPE:
        raise ValueError(f'handlers must return "{HANDLER_RETURN_TYPE}"')

    args = OrderedDict()
    output = None

    *func_args, last_func_arg = func.arguments

    # process input args
    for func_arg in func_args:
        is_const, arg_type = get_arg_type(func_arg)
        if not is_const:
            raise ValueError('output argument of handler has to be non-const const reference')
        args[func_arg.name] = arg_type

    # process last arg as either input or output
    is_const, arg_type = get_arg_type(last_func_arg)
    if is_const:
        args[last_func_arg.name] = arg_type
    else:
        output = arg_type

    
    return Endpoint(
        func=func,
        args=args if args else None,
        output=output,
    )

__all__ = [
    'EndpointType',
    'Endpoint',
    'endpoint_from_function',
]