from enum import StrEnum, unique


@unique
class ParamOperations(StrEnum):
    NO_OP  = 'NO_OP'
    ABS_OP = 'ABS_OP'
    ADD_OP = 'ADD_OP'
    SFT_OP = 'SFT_OP'
    PAN_OP = 'PAN_OP'
