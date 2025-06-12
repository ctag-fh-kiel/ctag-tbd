from enum import Enum, unique


@unique
class ParamType(Enum):
    """ Parameter types for reflection.
    .
        Reflectable class fields or function arguments are limited to a subset of C++ types or structs composed of
        these types. Using other types will either cause generation errors or be ignored. Use cases of parameters
        could be:

        - externally setting fields on classes
        - externally invoking functions.
    """

    INT_PARAM     = 'int_par'
    UINT_PARAM    = 'uint_par'
    TRIGGER_PARAM = 'trigger_par'
    FLOAT_PARAM   = 'float_par'
    UFLOAT_PARAM  = 'ufloat_par'
    STR_PARAM     = 'str_par'

PARAM_TYPES = {param_type.value: param_type for param_type in ParamType}

__all__ = ['ParamType', 'PARAM_TYPES']
