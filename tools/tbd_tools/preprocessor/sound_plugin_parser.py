from dataclasses import dataclass
import re
from typing import Final, List

import cxxheaderparser.simple as cpplib

from .reflectable_parser import ReflectableFinder
from .reflectables import ReflectableDescription

    
@dataclass
class SoundPluginDescription(ReflectableDescription):
    _name_expr: Final[re.Pattern] = re.compile(r'ctagSoundProcessor(?P<name>\w+)')

    @property
    def name(self):
        cls_name = self.cls_name
        if (match := SoundPluginDescription._name_expr.match(cls_name)):
            return match.group('name')
        return cls_name


class SoundPluginFinder(ReflectableFinder[SoundPluginDescription]):
    @staticmethod
    def _is_collected_class(cls: cpplib.ClassScope) -> bool:
        for base in cls.class_decl.bases:
            parent_name = base.typename.segments[-1].name
            if parent_name == 'ctagSoundProcessor':
                return True
        return False

    @staticmethod
    def _postprocess_reflectable(reflectable: ReflectableDescription) -> SoundPluginDescription:
        return SoundPluginDescription(reflectable.cls_name, reflectable.properties)


SoundPlugins = List[SoundPluginDescription]


__all__ = ['SoundPluginDescription', 'SoundPluginFinder', 'SoundPlugins']
