from enum import Enum, unique
from pydantic.dataclasses import dataclass, Field

@unique
class ParamTypeJson(Enum):
    bool_type  = 'bool'
    int_type   = 'int'
    group_type = 'group'


@dataclass
class ParamJson:
    id: str
    name: str
    type: ParamTypeJson
    hint: str | None = None
    min: int | None = None
    max: int | None = None
    step: int | None = None
    params: list['ParamJson'] | None = None

    def __str__(self):
        return self.id
    
    def __repr__(self):
        return f'ParamJson({self.id})'


ParamJsonIndex = dict[str, ParamJson]


@dataclass
class PluginJson:
    id: str
    name: str
    is_stereo: bool = Field(alias='isStereo')
    hint: str = Field()
    params: list[ParamJson] = Field()

    def __str__(self):
        return self.id
    
    def __repr__(self):
        return f'Plugin({self.id})'

    def get_param_index(self) -> ParamJsonIndex:
        all_params = {}
        def add_param(param: ParamJson, path=''):
            sub_path = path + param.id
            if param.params != None:
                if param.type != ParamTypeJson.group_type:
                    raise ValueError(f'{sub_path}: only group params can have subparams, got type {param.type}')
                for sub_param in param.params:
                    add_param(sub_param, f'{sub_path}.')
            else:
                all_params[sub_path] = param
        
        for param in self.params:
            add_param(param)

        return all_params

@dataclass
class PluginPatchParamJson:
    id: str
    current: int
    cv: int | None = None
    trig: int | None = None


@dataclass
class PluginPatchJson:
    name: str
    params: list[PluginPatchParamJson]


@dataclass
class PluginPatchesJson:
    active_patch: int = Field(alias='activePatch')
    patches: list[PluginPatchJson] = Field()


__all__ = [
    'ParamTypeJson',
    'ParamJson',
    'ParamJsonIndex',
    'PluginJson',
    'PluginPatchParamJson',
    'PluginPatchJson',
    'PluginPatchesJson',
]
