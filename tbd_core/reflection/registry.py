import dataclasses
import logging
from pathlib import Path
from typing import OrderedDict

import cxxheaderparser.types as cpptypes

from .attributes import Attributes
from .reflectable_parser import AnnotationParser, Parser
from .reflectables import (
    NamespaceDescription,
    PropertyDescription,
    ClassDescription,
    FunctionDescription,
    Reflectables, PropertyDescriptions,
)
from .scopes import ScopeDescription, normalize_typename


_LOGGER = logging.getLogger(__file__)


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


class ReflectableFinder:
    def __init__(self):
        self._headers: list[str] = []
        self._reflectables: Reflectables = Reflectables()

    def get_reflectables(self) -> Reflectables:
        self._amend_property_types()
        return self._reflectables

    def add_from_file(self, file_name: Path, *, include_base: Path | None = None) -> Reflectables:
        lines = self._read_code_from_file(file_name)
        visitor = AnnotationParser(lines)
        code = ''.join(lines)
        Parser(str(file_name), code, visitor).parse()
        parsed = visitor.data

        scope = ScopeDescription.from_root(parsed.namespace)
        relative_header = file_name.relative_to(include_base) if include_base else file_name

        added = Reflectables()
        self._find_entities_in_namespace(scope, relative_header, added)
        return added

    # private

    def _find_entities_in_namespace(self, scope: ScopeDescription, header: Path, added: Reflectables) -> None:
        self._add_namespace(scope, added)
        self._collect_functions_in_namespace(scope, header, added)
        self._collect_classes_in_namespace(scope, header, added)

        for ns in scope.namespace().namespaces.values():
            sub_namespace_scope = scope.add_namespace(ns)

            self._find_entities_in_namespace(sub_namespace_scope, header, added)

    def _add_namespace(self, scope: ScopeDescription, added: Reflectables) -> None:
        raw = scope.namespace()
        namespace = NamespaceDescription(scope, raw)
        namespaces = self._reflectables.namespaces
        if namespace.ref() not in namespaces:
            namespaces[namespace.ref()] = namespace
            added.namespaces[namespace.ref()] = namespace

    def _collect_functions_in_namespace(self, scope: ScopeDescription, header: Path, added: Reflectables) -> None:
        for raw_func in scope.namespace().functions:
            func_scope = scope.add_function(raw_func)
            self._collect_function(func_scope, header, added)

    def _collect_function(self, scope: ScopeDescription, header: Path, added: Reflectables) -> None:
        attrs = scope.attrs()
        name, description = name_and_description_from_attrs(attrs)

        func = FunctionDescription(
            raw=scope.func(),
            scope=scope,
            header=header,
            friendly_name=name,
            description=description,
            attrs=attrs,
        )
        self._add_function(func, added)

    def _add_function(self, func: FunctionDescription, added: Reflectables) -> None:
        funcs = self._reflectables.funcs
        if func.ref() in funcs:
            _LOGGER.warning(f'already encountered function {func.full_name}')
        funcs[func.ref()] = func
        added.funcs[func.ref()] = func

    def _collect_classes_in_namespace(self, scope: ScopeDescription, header: Path, added: Reflectables) -> None:
        for cls in scope.namespace().classes:
            cls_scope = scope.add_class(cls)
            self._collect_class_and_nested_classes(cls_scope, header, added)

    def _collect_class_and_nested_classes(self, scope: ScopeDescription, header: Path, added: Reflectables) -> None:
        attrs = scope.attrs()
        name, description = name_and_description_from_attrs(attrs)

        type_id = scope.hash
        properties = self._extract_properties(scope, type_id, added)

        raw_cls = scope.cls()
        cls = ClassDescription(
            raw=raw_cls,
            scope=scope,
            header=header,
            friendly_name=name,
            description=description,
            properties=properties
        )
        self._add_class(cls, added)

        for nested_cls in raw_cls.classes:
            nested_cls_scope = scope.add_class(nested_cls)
            self._collect_class_and_nested_classes(nested_cls_scope, header, added)

    def _add_class(self, cls: ClassDescription, added: Reflectables) -> None:
        classes = self._reflectables.classes
        if cls.ref() in classes:
            _LOGGER.warning(f'already encountered function {cls.full_name}')
        classes[cls.ref()] = cls
        added.classes[cls.ref()] = cls

    def _extract_properties(self, scope: ScopeDescription, cls_id: int, added: Reflectables) -> list[int]:
        cls = scope.cls()
        properties = []
        for field in cls.fields:
            field_scope = scope.add_field(field)

            # attributes can not be pointers, references, arrays etc
            if not isinstance(field.type, cpptypes.Type):
                continue

            # TODO: should private fields be reflected?
            if field.access == 'private':
                continue

            # type_name = self._full_find_field_type(field_scope)
            attrs = field_scope.attrs()
            name, description = name_and_description_from_attrs(attrs)

            prop = PropertyDescription(
                scope=field_scope,
                raw=field,
                cls_id=cls_id,
                type=-1,
                friendly_name=name,
                description=description,
                attrs=attrs,
            )
            self._add_property(prop, added)
            properties.append(prop.ref())

        return properties

    def _add_property(self, property: PropertyDescription, added: Reflectables) -> None:
        properties = self._reflectables.properties
        if property.ref() in properties:
            _LOGGER.warning(f'already encountered function {property.full_name}')
        properties[property.ref()] = property
        added.properties[property.ref()] = property

    def _amend_property_types(self) -> None:
        for prop in self._reflectables.properties.values():
            if prop.type == -1:
                prop.type = self._find_property_type(prop.scope)

    def _find_property_type(self, scope: ScopeDescription) -> str | int:
        field = scope.field()
        field_type, is_anonymous = normalize_typename(field.type.typename)
        if is_anonymous:
            return f'{scope.parent}::{field_type}'

        for cls in self._reflectables.class_list:
            cls_name = cls.full_name
            encl_scope = scope.parent
            while encl_scope:
                full_name = f'{encl_scope}::{field_type}'
                if cls_name == full_name:
                    return cls.ref()
                encl_scope = encl_scope.parent

        return field_type

    @staticmethod
    def _read_code_from_file(header_path: Path) -> list[str]:
        code = []
        with open(header_path) as f:
            for line in f:
                if not line.strip().startswith('#'):
                    code.append(line)
        return code


__all__ = ['ReflectableFinder']
