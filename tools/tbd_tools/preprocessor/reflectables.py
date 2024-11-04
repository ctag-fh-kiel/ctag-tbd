from dataclasses import dataclass
from enum import Enum, unique
from typing import List, Set


@unique
class PropertyType(Enum):
    """ valid C++ types for properties """

    int_type = 'int32_t'
    float_type = 'float'
    string_type = 'std::string'

type_map = {t.value: t.name for t in PropertyType}

def get_property_type(cpp_type: str):
    if cpp_type not in type_map:
        raise ValueError(f'{cpp_type} is not a supported property type')
    

@dataclass
class PropertyDescription:
    
    name: str
    type: str
    is_read_only: bool


Properties = List[PropertyDescription]


@dataclass
class ReflectableDescription:
    cls_name: str
    properties: List[PropertyDescription]

    @property
    def meta_name(self):
        return f'{self.cls_name}Meta'


Headers = Set[str]


__all__ = [
    'PropertyType', 
    'get_property_type', 
    'PropertyDescription', 
    'Properties', 
    'ReflectableDescription',
    'Headers'
]
