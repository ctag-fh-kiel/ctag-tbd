from collections import OrderedDict
from dataclasses import dataclass, field
from enum import StrEnum, unique
from typing import Protocol

from .parameters import ParamType
from .attributes import Attributes


ComponentID = int
FileID = int
NamespaceID = int
FunctionID = int
ArgumentID = int
ClassID = int
PropertyID = int


class Typed(Protocol):
    def typename(self) -> str: ...
    def __str__(self) -> str: ...
    def __repr__(self) -> str: ...


@dataclass(kw_only=True)
class Param(Typed):
    param_type: ParamType
    is_mappable: bool

    @property
    def typename(self) -> str:
        return self.param_type.value

    def __str__(self) -> str:
        return str(self.typename)

    def __repr__(self) -> str:
        return f'Param({self.typename})'


@dataclass(kw_only=True)
class UnknownType(Typed):
    type: str

    @property
    def typename(self) -> str:
        return self.type

    def __str__(self) -> str:
        return self.type

    def __repr__(self) -> str:
        return f'UnknownType({self.type})'


CppType = Param | ClassID | UnknownType


@dataclass(kw_only=True)
class FileEntry:
    component: ComponentID
    file: str

Files = OrderedDict[FileID, FileEntry]


@dataclass(kw_only=True)
class EntryBase:
    files: list[FileID]
    parent: NamespaceID | ClassID | FunctionID
    attrs: Attributes | None
    friendly_name: str | None = None
    description: str | None = None

Components = OrderedDict[ComponentID, str]


@dataclass(kw_only=True)
class NamespaceEntry(EntryBase):
    namespace_name: str

NamespaceEntries = OrderedDict[NamespaceID, NamespaceEntry]


@unique
class ArgumentCategory(StrEnum):
    CONST = 'CONST'
    VALUE = 'VALUE'
    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    INVALID = 'INVALID'


@dataclass(kw_only=True)
class ArgumentEntry(EntryBase):
    arg_name: str
    category: ArgumentCategory
    type: CppType

ArgumentEntries = OrderedDict[ArgumentID, ArgumentEntry]


@dataclass(kw_only=True)
class FunctionEntry(EntryBase):
    func_name: str
    arguments: list[ArgumentID]
    return_type: CppType | None

    @property
    def name(self) -> str:
        return self.friendly_name if self.friendly_name else self.func_name

FunctionEntries = OrderedDict[FunctionID, FunctionEntry]


@dataclass(kw_only=True)
class PropertyEntry(EntryBase):
    field_name: str
    type: CppType
    
    @property
    def name(self) -> str:
        return self.friendly_name if self.friendly_name else self.field_name

PropertyEntries = OrderedDict[PropertyID, PropertyEntry]


@dataclass(kw_only=True)
class ClassEntry(EntryBase):
    cls_name: str
    bases: list[CppType]
    properties: list[PropertyID]

    @property
    def name(self) -> str:
        return self.friendly_name if self.friendly_name else self.cls_name

ClassEntries = OrderedDict[ClassID, ClassEntry]


@dataclass
class Reflectables:
    components: Components = field(default_factory=OrderedDict)
    files: Files = field(default_factory=OrderedDict)
    namespaces: NamespaceEntries = field(default_factory=OrderedDict)
    functions: FunctionEntries = field(default_factory=OrderedDict)
    arguments: ArgumentEntries = field(default_factory=OrderedDict)
    classes: ClassEntries = field(default_factory=OrderedDict)
    properties: PropertyEntries = field(default_factory=OrderedDict)

    def __ior__(self, other: 'Reflectables') -> 'Reflectables':
        self.components |= other.components
        self.files |= other.files
        self.namespaces |= other.namespaces
        self.functions |= other.functions
        self.arguments |= other.arguments
        self.classes |= other.classes
        self.properties |= other.properties
        return self


__all__ = [
    'ComponentID',
    'FileID',
    'NamespaceID',
    'FunctionID',
    'ArgumentID',
    'ClassID',
    'PropertyID',
    'Typed',
    'UnknownType',
    'Param',
    'CppType',
    'EntryBase',
    'FileEntry',
    'NamespaceEntry',
    'NamespaceEntries',
    'ArgumentCategory',
    'ArgumentEntry',
    'ArgumentEntries',
    'PropertyEntry',
    'PropertyEntries',
    'FunctionEntry',
    'FunctionEntries',
    'ClassEntry',
    'ClassEntries',
    'Reflectables',
]
