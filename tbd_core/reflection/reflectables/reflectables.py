from collections import OrderedDict
from dataclasses import dataclass, field
from enum import StrEnum, unique

from pathlib import Path

from .attributes import Attributes


CppTypeId = int | str


@dataclass(kw_only=True)
class EntryBase:
    header: int
    parent: int
    attrs: Attributes | None
    friendly_name: str | None = None
    description: str | None = None

Modules = OrderedDict[int, str]


@dataclass(kw_only=True)
class FilesEntry:
    module: int
    header: str
    impls: list[str] | None

Files = OrderedDict[int, FilesEntry]


@dataclass(kw_only=True)
class NamespaceEntry(EntryBase):
    namespace_name: str

NamespaceEntries = OrderedDict[int, NamespaceEntry]


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
    type: CppTypeId

ArgumentEntries = OrderedDict[int, ArgumentEntry]


@dataclass(kw_only=True)
class FunctionEntry(EntryBase):
    func_name: str
    arguments: list[int]
    return_type: CppTypeId | None

    @property
    def name(self) -> str:
        return self.friendly_name if self.friendly_name else self.func_name

FunctionEntries = OrderedDict[int, FunctionEntry]


@dataclass(kw_only=True)
class PropertyEntry(EntryBase):
    field_name: str
    type: CppTypeId
    
    @property
    def name(self) -> str:
        return self.friendly_name if self.friendly_name else self.field_name

PropertyEntries = OrderedDict[int, PropertyEntry]


@dataclass(kw_only=True)
class ClassEntry(EntryBase):
    cls_name: str
    bases: list[CppTypeId]
    properties: list[int]

    @property
    def name(self) -> str:
        return self.friendly_name if self.friendly_name else self.cls_name

ClassEntries = OrderedDict[int, ClassEntry]


@dataclass
class Reflectables:
    modules: Modules = field(default_factory=OrderedDict)
    headers: Files = field(default_factory=OrderedDict)
    namespaces: NamespaceEntries = field(default_factory=OrderedDict)
    functions: FunctionEntries = field(default_factory=OrderedDict)
    arguments: ArgumentEntries = field(default_factory=OrderedDict)
    classes: ClassEntries = field(default_factory=OrderedDict)
    properties: PropertyEntries = field(default_factory=OrderedDict)

    def __ior__(self, other: 'Reflectables') -> 'Reflectables':

        self.headers |= other.headers
        self.namespaces |= other.namespaces
        self.functions |= other.functions
        self.arguments |= other.arguments
        self.classes |= other.classes
        self.properties |= other.properties
        return self


__all__ = [
    'CppTypeId',
    'EntryBase',
    'FilesEntry',
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
