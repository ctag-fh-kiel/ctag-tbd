from enum import Enum, unique


@unique
class ParamType(Enum):
    INT_PARAM     = 'int_par'
    UINT_PARAM    = 'uint_par'
    TRIGGER_PARAM = 'trigger_par'
    FLOAT_PARAM   = 'float_par'
    UFLOAT_PARAM  = 'ufloat_par'
    STR_PARAM     = 'str_par'

PARAM_TYPES = {param_type.value: param_type for param_type in ParamType}

__all__ = ['ParamType', 'PARAM_TYPES']
