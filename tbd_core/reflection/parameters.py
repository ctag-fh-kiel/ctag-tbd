from enum import StrEnum, unique


@unique
class ParamType(StrEnum):
    """ Parameter types for reflection.

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

    @property
    def is_float(self) -> bool:
        return self in [ParamType.FLOAT_PARAM, ParamType.UFLOAT_PARAM]

    def value_field(self) -> str:
        return self.value.split('_')[0] + '_value'


PARAM_TYPES: dict[str, ParamType] = {param_type.value: param_type for param_type in ParamType}

@unique
class MappableParamType(StrEnum):
    """ Parameter types for reflection.

        These types add aliasing, to signal a parameter can be mapped in plugins.
    """

    MINT_PARAM     = 'mint_par'
    MUINT_PARAM    = 'muint_par'
    MTRIGGER_PARAM = 'mtrigger_par'
    MFLOAT_PARAM   = 'mfloat_par'
    MUFLOAT_PARAM  = 'mufloat_par'

    @property
    def is_float(self) -> bool:
        return self in [MappableParamType.MFLOAT_PARAM, MappableParamType.MUFLOAT_PARAM]

    def value_field(self) -> str:
        return self.value.split('_')[0][1:] + '_value'

MAPPABLE_PARAM_TYPES: dict[str, MappableParamType] = {param_type.value: param_type for param_type in MappableParamType}
PARAM_TYPE_FROM_MAPPABLE: dict[MappableParamType, ParamType] = {
    mappable_type: PARAM_TYPES[mappable_type[1:]]
    for mappable_type in MappableParamType
}


ALL_PARAM_TYPES: dict[str, ParamType | MappableParamType] = {**PARAM_TYPES, **MAPPABLE_PARAM_TYPES}


__all__ = [
    'ParamType',
    'PARAM_TYPES',
    'MappableParamType',
    'MAPPABLE_PARAM_TYPES',
    'ALL_PARAM_TYPES',
]
