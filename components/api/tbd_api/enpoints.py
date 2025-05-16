from enum import Enum, unique
from dataclasses import dataclass
from pathlib import Path

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
    """ Remotely callable procedure. """

    func: tbr.FunctionDescription
    in_message: str | None
    out_message: str | None

    @property
    def name(self) -> str:
        return self.func.name

    @property
    def full_name(self) -> tbr.ScopeDescription:
        """ Full path to c++ function. """
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
    def type(self) -> EndpointType:
        if self.in_message and self.out_message:
            return EndpointType.FUNCTION
        elif self.out_message:
            return EndpointType.GETTER
        elif self.in_message:
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

        FUNCTION: argument and return 
            `tbd::Error f(const ArgType&, ResType&);`
        GETTER: only argument
            `tbd::Error f(const ArgType&);`
        SETTER: only return
            `tbd::Error f(ResType&);
        TRIGGER: no argument and no return
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

__all__ = [
    'Endpointtype',
    'Endpoint',
    'endpoint_from_function',
]