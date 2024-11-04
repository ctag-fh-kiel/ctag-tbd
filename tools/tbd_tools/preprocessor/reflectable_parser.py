from pathlib import Path
from typing import Generic, List, TypeVar
import abc
import re

from cxxheaderparser.simple import SClassBlockState
import cxxheaderparser.simple as cpplib
import cxxheaderparser.types as cpptypes

from .reflectables import Properties, PropertyDescription, ReflectableDescription, get_property_type, Headers


class AnnotationParser(cpplib.SimpleCxxVisitor):
    def __init__(self, lines) -> None:
        self._get_properties_exp = re.compile(r'^\s*\[\s*\[(?P<attributes>.*)\]\s*\]')
        self._ctag_attr_expr = re.compile(r'(?P<attr>\w+)(\((?P<args>[^\)]*)\))?')
        self._lines = lines
        super().__init__()

    def on_class_field(self, state: SClassBlockState, f: cpplib.Field) -> None:
        line = self._lines[state.location.lineno - 1]
        f.is_writable_property = self._is_writable_property(line)
        super().on_class_field(state, f)

    def _is_writable_property(self, line: str) -> bool:
        res = self._get_properties_exp.match(line)
        if res is None:
            return None
        
        atrributes = ''.join(res.group('attributes').split())
        ctag_attrs = [match.group('args') for match in self._ctag_attr_expr.finditer(atrributes) if (attr := match.group('attr')) == 'ctag_prop']
        if not ctag_attrs:
            return None
        
        if len(ctag_attrs) > 1:
            raise ValueError('multiple occurences of ctag_prop attribute')
        
        prop_attr = ctag_attrs[0]
        if prop_attr is None or prop_attr == '':
            return False
        
        if prop_attr != 'readonly':
            raise ValueError(f'invalid ctag_prop argument {prop_attr}')
        return True


ReflectableT = TypeVar('ReflectableT', bound=ReflectableDescription)
class ReflectableFinder(abc.ABC, Generic[ReflectableT]):
    def __init__(self):
        self._cls: List[ReflectableDescription] = []
        self._headers: Headers = set()

    @property
    def reflectables(self) -> List[ReflectableT]:
        return self._cls

    @property
    def headers(self) -> Headers:
        return self._headers


    def add_from_file(self, file_name: Path) -> None:
        lines = self._read_code_from_file(file_name)
        visitor = AnnotationParser(lines)
        code = ''.join(lines)
        cpplib.CxxParser(file_name, code, visitor).parse()
        # parsed = cp.parse_string(code)
        parsed = visitor.data
        self._find_classes(parsed.namespace, file_name)

    # abstract

    @staticmethod
    @abc.abstractmethod
    def _is_collected_class(cls: cpplib.ClassScope) -> bool:
        """ prefilter classes by raw description
        
            override in derived classes to narrow down selection
        """

    @staticmethod
    @abc.abstractmethod
    def _postprocess_reflectable(reflectable: ReflectableDescription) -> ReflectableT:
        """ transforms raw parsed class to description type 
        
            override in derived classes to extract more detailed class information and
            filter out undesired classes from detailed class description

            returns: descrition type or None; if None, the class is dropped from set
        """

    # private

    def _find_classes(self, namespace: cpplib.NamespaceScope, file_name: Path) -> None:
        for ns_name, ns in namespace.namespaces.items():
            for cls in ns.classes:
                if not self._is_collected_class(cls):
                    continue

                name = cls.class_decl.typename.segments[-1].format()

                # print(f'found sound processor {cls.class_decl.typename.format()}')
                self._headers.add(file_name.name)

                properties = self._extract_properties(cls)
                if (description := self._postprocess_reflectable(ReflectableDescription(name, properties))) is not None:
                    self._cls.append(description)

            self._find_classes(ns, file_name)

    def _extract_properties(self, cls: cpplib.ClassScope) -> Properties:
        properties = []
        for field in cls.fields:
            property_type = field.is_writable_property
            if property_type is None:
                continue

            field_name = field.name
            field_type = field.type
            
            if not isinstance(field_type, cpptypes.Type):
                raise ValueError(f'invalid field type {field_type.format()}')
            field_type = field_type.format()

            properties.append(PropertyDescription(field_name, get_property_type(field_type), property_type))

        return properties

    @DeprecationWarning
    def _extract_wrapped_properties(self, cls: cpplib.ClassScope) -> List[PropertyDescription]:
        properties = []
        for field in cls.fields:
            field_type = field.type
            if not isinstance(field_type, cpptypes.Type):
                continue

            field_type = field_type.typename.segments[-1]

            if not isinstance(field_type, cpptypes.NameSpecifier):
                continue

            if field_type.name not in ['Property']:
                continue

            # print(field_type.specialization)
            field_name = field.name.format()
            nested_type = field_type.specialization.args[0].arg.format()
            type_parts = nested_type.split(maxsplit=1)
            if type_parts[0] == 'const':
                properties.append(PropertyDescription(field_name, get_property_type[type_parts[1]], True))
            else:
                properties.append(PropertyDescription(field_name, get_property_type[nested_type], False))

        return properties

    @staticmethod
    def _read_code_from_file(header_path: Path) -> str:
        code = []
        with open(header_path) as f:
            for line in f:
                if not line.strip().startswith('#'):
                    code.append(line)
        return code


__all__ = ['Headers', 'ReflectableFinder']
