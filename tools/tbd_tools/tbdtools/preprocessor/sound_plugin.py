from enum import Enum, unique
from git import List, Optional
from pydantic import ConfigDict
from pydantic.dataclasses import dataclass, Field


@unique
class PropertyType(Enum):
    bool_type  = 'bool'
    int_type   = 'int'
    group_type = 'group'


@dataclass
class SoundPluginProperty:
    id: str
    name: str
    type: PropertyType
    min: Optional[int] = None
    max: Optional[int] = None
    step: Optional[int] = None
    params: Optional[List['SoundPluginProperty']] = None


@dataclass
class SoundPlugin:
    id: str
    name: str
    is_stereo: bool = Field(alias='isStereo')
    hint: str = Field()
    params: List[SoundPluginProperty] = Field()



@dataclass
class SoundPluginPatchProperty:
    id: str
    current: int
    cv: Optional[int] = None
    trig: Optional[int] = None


@dataclass
class SoundPluginPatch:
    name: str
    params: List[SoundPluginPatchProperty]


@dataclass
class SoundPluginPatches:
    active_patch: int = Field(alias='activePatch')
    patches: List[SoundPluginPatch] = Field()


__all__ = [
    'SoundPluginProperty',
    'SoundPlugin',
    'SoundPluginPatchProperty',
    'SoundPluginPatch',
    'SoundPluginPatches'
]
