from dataclasses import dataclass
from pathlib import Path
from typing import Iterator

import cxxheaderparser.types as cpptypes
import cxxheaderparser.simple as cpplib

from tbd_core.reflection.reflectables import *


def is_anon_struct_field(type_name: UnknownType) -> bool:
    return type_name.type.startswith(ANONYMOUS_STRUCT_PREFIX)


def alias_anonymous_class_name(cls: cpplib.ClassScope) -> str:
    segments = cls.class_decl.typename.segments
    if len(segments) != 1:
        raise ValueError('expected class name to consist of a single segment')
    segment = segments[0]
    if isinstance(segment, cpptypes.AnonymousName):
        anon_id = segment.id
        return f'{ANONYMOUS_STRUCT_PREFIX}{anon_id}'
    return segment.name


def alias_anonymous_field_type(field_type: cpptypes.DecoratedType) -> UnknownType | None:
    if isinstance(field_type, cpptypes.Type):
        segments = field_type.typename.segments
        if len(segments) == 1 and isinstance(segments[0], cpptypes.AnonymousName):
            anon_id = segments[0].id
            return UnknownType(type=f'{ANONYMOUS_STRUCT_PREFIX}{anon_id}')
    return None


def convert_parameter_types(type_name:  cpptypes.DecoratedType) -> Param | None:
    if not isinstance(type_name, cpptypes.Type):
        return None

    segments = type_name.typename.segments
    num_segments = len(segments)

    ns = 'tbd'
    if num_segments == 1:
        name = segments[0].name
    elif num_segments == 2:
        ns = segments[0].name
        name = segments[1].name
    else:
        return None

    if ns != PARAM_NAMESPACE:
        return None

    param_type = ALL_PARAM_TYPES.get(name)
    if param_type is not None:
        return Param(
            param_type=param_type,
            is_mappable=False,
        )
    param_type = MAPPABLE_PARAM_TYPES.get(name)
    if param_type is not None:
        return Param(
            param_type=PARAM_TYPE_FROM_MAPPABLE[param_type],
            is_mappable=True,
        )
    return None


def name_and_description_from_attrs(attrs: Attributes | None) -> tuple[str | None, str | None]:
    """ Given a list of attributes, return the first 'name' and 'description' field values if present. """

    name = None
    description = None

    if attrs is None:
        return name, description

    for attr in attrs.values():
        attr_options = attr.options
        if 'name' in attr_options:
            name = attr_options['name']
        if 'description' in attr_options:
            description = attr_options['description']
    return name, description



@dataclass
class FileContext:
    component: str
    file: Path

    def entry(self) -> FileEntry:
        return FileEntry(
            component=self.component_ref(),
            file=str(self.file),
        )

    def ref(self):
        return file_ref(self.component, str(self.file))

    def component_ref(self):
        return component_ref(self.component)


@dataclass
class ScopeContext:
    parent: int
    file: FileContext
    scope: ScopePath

    @property
    def full_name(self) -> str:
        return self.scope.path

    def ref(self) -> PropertyID | ClassID | ArgumentID | FunctionID | NamespaceID:
        if self.scope.is_root():
            return NO_PARENT_ID
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
    def type(self) -> Param | UnknownType:
        field_type = self._field.type
        if (_type := alias_anonymous_field_type(field_type)) is not None:
            return _type
        if (_type := convert_parameter_types(field_type)) is not None:
            return _type
        return UnknownType(type=field_type.format())


    def entry(self) -> PropertyEntry:
        name, description = name_and_description_from_attrs(self.attrs)
        return PropertyEntry(
                files=[self.file.ref()],
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
        return alias_anonymous_class_name(self._cls)

    @property
    def attrs(self) -> Attributes:
        return self._cls.class_decl.attrs

    def entry(self) -> ClassEntry:
        print('creating', self.name, '>>', self.file.file)
        props = [prop.ref() for prop in self.fields()]
        name, description = name_and_description_from_attrs(self.attrs)
        return ClassEntry(
            files=[self.file.ref()],
            parent=self.parent,
            attrs=self.attrs,
            friendly_name=name,
            description=description,
            cls_name=self.name,
            bases=self.bases(),
            properties=props,
            generated=False,
        )

    def classes(self) -> Iterator['ClassContext']:
        for cls in self._cls.classes:
            # FIXME: unions are not handled here
            if cls.class_decl.classkey not in ['struct', 'class']:
                continue
            yield ClassContext(
                parent=self.ref(),
                file=self.file,
                scope=self.scope.add_class(alias_anonymous_class_name(cls)),
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

    def bases(self) -> list[UnknownType]:
        return [UnknownType(type=base.typename.format()) for base in self._cls.class_decl.bases
                if base.access == 'public']

@dataclass
class ArgumentContext(ScopeContext):
    _argument: cpptypes.Parameter

    @property
    def name(self) -> str:
        return self._argument.name

    @property
    def type(self) -> Param | UnknownType:
        arg_type = self._argument.type
        match arg_type:
            case cpptypes.Reference():
                arg_type = arg_type.ref_to
                if (_type := convert_parameter_types(arg_type)) is not None:
                    return _type
                return UnknownType(type=arg_type.typename.format())
            case cpptypes.Type():
                if (_type := convert_parameter_types(arg_type)) is not None:
                    return _type
                return UnknownType(type=arg_type.typename.format())
        return UnknownType(type=arg_type.format())

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
            files=[self.file.ref()],
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
            files=[self.file.ref()],
            parent=self.parent,
            attrs=self.attrs,
            friendly_name=name,
            description=description,
            func_name=self.name,
            arguments=arguments,
            return_type=self.result,
            generated=False,
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
    def result(self) -> UnknownType | None:
        _type = self._fn.return_type.format()
        return UnknownType(type=_type) if _type != 'void' else None


@dataclass
class NamespaceContext(ScopeContext):
    _ns: cpplib.NamespaceScope

    @property
    def name(self) -> str:
        return self._ns.name

    def entry(self) -> NamespaceEntry:
        return NamespaceEntry(
            parent=self.parent,
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
                scope=self.scope.add_class(alias_anonymous_class_name(cls)),
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
    def root(file: FileContext, root_namespace: cpplib.NamespaceScope) -> 'NamespaceContext':
        return NamespaceContext(
            parent=-1,
            file=file,
            scope=ScopePath.root(),
            _ns=root_namespace,
        )


__all__ = [
    'is_anon_struct_field',
    'FileContext',
    'ScopeContext',
    'FieldContext',
    'ClassContext',
    'ArgumentContext',
    'FunctionContext',
    'NamespaceContext'
]
