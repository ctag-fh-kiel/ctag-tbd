from dataclasses import dataclass
from pathlib import Path
from typing import Iterator
from zlib import crc32

import cxxheaderparser.types as cpptypes
import cxxheaderparser.simple as cpplib

from tbd_core.reflection.reflectables import (
    Attributes,
    ArgumentCategory,
    NamespaceEntry,
    ClassEntry,
    PropertyEntry,
    ArgumentEntry,
    ScopePath,
    FunctionEntry,
)


ANONYMOUS_STRUCT_PREFIX = '__struct_'


def is_anon_struct_field(type_name: str) -> bool:
    return type_name.startswith(ANONYMOUS_STRUCT_PREFIX)


def normalize_class_name(cls: cpplib.ClassScope) -> str:
    segments = cls.class_decl.typename.segments
    if len(segments) != 1:
        raise ValueError('expected class name to consist of a single segment')
    segment = segments[0]
    if isinstance(segment, cpptypes.AnonymousName):
        anon_id = segment.id
        return f'{ANONYMOUS_STRUCT_PREFIX}{anon_id}'
    return segment.name


def normalize_field_type(field_type: cpptypes.DecoratedType) -> str:
    if isinstance(field_type, cpptypes.Type):
        segments = field_type.typename.segments
        if len(segments) == 1 and isinstance(segments[0], cpptypes.AnonymousName):
            anon_id = segments[0].id
            return f'{ANONYMOUS_STRUCT_PREFIX}{anon_id}'
        return field_type.typename.format()
    return field_type.format()


def name_and_description_from_attrs(attrs: Attributes | None) -> tuple[str | None, str | None]:
    """ Given a list of attributes, return the first 'name' and 'description' field values if present. """

    name = None
    description = None

    if attrs is None:
        return name, description

    for attr in attrs:
        attr_params = attr.params
        if 'name' in attr_params:
            name = attr_params['name']
        if 'description' in attr_params:
            description = attr_params['description']
    return name, description


def ref_for_file(relative_file_path: Path) -> int:
    return crc32(str(relative_file_path).encode())

@dataclass
class FilesContext:
    module: str
    header: str
    impls: list[str] | None


@dataclass
class ScopeContext:
    parent: int
    file: Path
    scope: ScopePath

    @property
    def header(self) -> int:
        return ref_for_file(self.file)

    @property
    def full_name(self) -> str:
        return self.scope.path

    def ref(self) -> int:
        return self.scope.hash()


@dataclass
class FieldContext(ScopeContext):
    _field: cpptypes.Field

    @property
    def name(self) -> str:
        return self._field.name

    @property
    def attrs(self) -> Attributes:
        return self._field.attrs

    @property
    def type(self) -> str:
        return normalize_field_type(self._field.type)

    def entry(self) -> PropertyEntry:
        name, description = name_and_description_from_attrs(self.attrs)
        return PropertyEntry(
                header=self.header,
                parent=self.parent,
                attrs=self.attrs,
                friendly_name=name,
                description=description,
                type=self.type,
                field_name=self.name
            )

@dataclass
class ClassContext(ScopeContext):
    _cls: cpplib.ClassScope

    @property
    def name(self) -> str:
        return normalize_class_name(self._cls)

    @property
    def attrs(self) -> Attributes:
        return self._cls.class_decl.attrs

    def entry(self) -> ClassEntry:
        props = [prop.ref() for prop in self.fields()]
        name, description = name_and_description_from_attrs(self.attrs)
        return ClassEntry(
            header=self.header,
            parent=self.parent,
            attrs=self.attrs,
            friendly_name=name,
            description=description,
            cls_name=self.name,
            bases=self.bases(),
            properties=props,
        )

    def classes(self) -> Iterator['ClassContext']:
        for cls in self._cls.classes:
            if cls.class_decl.classkey not in ['struct', 'class']:
                continue
            yield ClassContext(
                parent=self.ref(),
                file=self.file,
                scope=self.scope.add_class(normalize_class_name(cls)),
                _cls=cls,
            )

    def fields(self) -> Iterator[FieldContext]:
        for field in self._cls.fields:
            if not isinstance(field.type, cpptypes.Type):
                continue
            if field.access != 'public':
                continue

            yield FieldContext(
                parent=self.ref(),
                file=self.file,
                scope=self.scope.add_field(field.name),
                _field=field,
            )

    def bases(self) -> list[str]:
        return [base.typename.format() for base in self._cls.class_decl.bases if base.access == 'public']

@dataclass
class ArgumentContext(ScopeContext):
    _argument: cpptypes.Parameter

    @property
    def name(self) -> str:
        return self._argument.name

    @property
    def type(self) -> str:
        arg_type = self._argument.type
        match arg_type:
            case cpptypes.Reference():
                arg_type = arg_type.ref_to.typename
            case cpptypes.Type:
                arg_type = arg_type.typename
        return arg_type.format()

    @property
    def category(self) -> ArgumentCategory:
        arg_type = self._argument.type
        match arg_type:
            case cpptypes.Type():
                if arg_type.const:
                    return ArgumentCategory.CONST
                return ArgumentCategory.VALUE
            case cpptypes.Reference():
                arg_type = arg_type.ref_to
                if arg_type.const:
                    return ArgumentCategory.INPUT
                return ArgumentCategory.OUTPUT
            case _:
                return ArgumentCategory.INVALID

    def entry(self) -> ArgumentEntry:
        return ArgumentEntry(
            header=self.header,
            parent=self.parent,
            attrs=None,
            arg_name=self.name,
            category=self.category,
            type=self.type,
        )

@dataclass
class FunctionContext(ScopeContext):
    _fn: cpplib.Function

    @property
    def name(self) -> str:
        return self._fn.name.format()

    @property
    def attrs(self) -> Attributes:
        return self._fn.attrs

    def entry(self) -> FunctionEntry:
        arguments = [arg.ref() for arg in self.arguments()]
        name, description = name_and_description_from_attrs(self.attrs)
        return FunctionEntry(
            header=self.header,
            parent=self.parent,
            attrs=self.attrs,
            friendly_name=name,
            description=description,
            func_name=self.name,
            arguments=arguments,
            return_type=self.result,
        )

    def arguments(self) -> Iterator[ArgumentContext]:
        for argument in self._fn.parameters:
            yield ArgumentContext(
                parent=self.ref(),
                file=self.file,
                scope=self.scope.add_argument(argument.name),
                _argument=argument,
            )

    @property
    def result(self) -> str | None:
        _type = self._fn.return_type.format()
        return _type if _type != 'void' else None


@dataclass
class NamespaceContext(ScopeContext):
    _ns: cpplib.NamespaceScope

    @property
    def name(self) -> str:
        return self._ns.name

    def entry(self) -> NamespaceEntry:
        return NamespaceEntry(
            header=self.header,
            parent=self.parent,
            attrs=None,
            namespace_name=self.name,
        )

    def functions(self) -> Iterator[FunctionContext]:
        for fn in self._ns.functions:
            yield FunctionContext(
                parent=self.ref(),
                file=self.file,
                scope=self.scope.add_function(fn.name.format()),
                _fn=fn,
            )

    def classes(self) -> Iterator[ClassContext]:
        for cls in self._ns.classes:
            if cls.class_decl.classkey not in ['struct', 'class']:
                continue
            yield ClassContext(
                parent=self.ref(),
                file=self.file,
                scope=self.scope.add_class(normalize_class_name(cls)),
                _cls=cls,
            )

    def sub_namespaces(self) -> Iterator['NamespaceContext']:
        for ns in self._ns.namespaces.values():
            yield NamespaceContext(
                parent=self.ref(),
                file=self.file,
                scope=self.scope.add_namespace(ns.name),
                _ns=ns,
            )

    @staticmethod
    def root(file: Path, root_namespace: cpplib.NamespaceScope) -> 'NamespaceContext':
        return NamespaceContext(
            parent=-1,
            file=file,
            scope=ScopePath.root(),
            _ns=root_namespace,
        )

__all__ = [
    'is_anon_struct_field',
    'ref_for_file',
    'ScopeContext',
    'FieldContext',
    'ClassContext',
    'ArgumentContext',
    'FunctionContext',
    'NamespaceContext'
]