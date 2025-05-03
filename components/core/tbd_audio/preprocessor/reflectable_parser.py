from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Final, List, Optional
import logging

import cxxheaderparser.lexer as lexer
import cxxheaderparser.simple as cpplib
import cxxheaderparser.types as cpptypes

from .reflectables import (
    normalize_typename,
    Properties, 
    PropertyDescription, 
    ReflectableDescription, 
    ScopeDescription,
    Headers,
    Attributes
)

_LOGGER = logging.getLogger(__file__)


class Parser(cpplib.CxxParser):
    def _consume_attribute_specifier_seq(
        self, tok: lexer.LexToken, doxygen: Optional[str] = None
    ) -> None:
        while True:
            if tok.type == "DBL_LBRACKET":
                tokens = self._consume_balanced_tokens(tok)
                self.visitor.on_attributes(tokens[1:-1])
            else:
                self.lex.return_token(tok)
                break

            maybe_tok = self.lex.token_if(*self._attribute_specifier_seq_start_types)
            if maybe_tok is None:
                break

            tok = maybe_tok


class AnnotationParser(cpplib.SimpleCxxVisitor):
    def __init__(self, lines) -> None:
        self._attrs: List[lexer.LexToken] | None = None
        super().__init__()

    def on_class_start(self, state:cpplib.SClassBlockState):
        super().on_class_start(state)
        state.class_decl.attrs = self._parse_attrs()
        self._attrs = None


    def on_class_field(self, state:cpplib.SClassBlockState, f: cpplib.Field) -> None:
        super().on_class_field(state, f)

        try:
            f.attrs = self._parse_attrs()
        except Exception as e:
            _LOGGER.error(f'{state.location.filename}{state.location.lineno}: {e}')
            f.attrs = None

        self._attrs = None
        

    def _parse_attrs(self) -> Attributes:
        tokens = self._attrs
        if tokens is None or len(tokens) < 3:
            return None
        fst, snd, *args, last = tokens

        # ignore non tbd attributes
        if fst.value != 'tbd':
            return None
         
        if snd.type != '(' or last.type != ')':
            raise ValueError('encountered invalid attribute')

        attrs = {}
        while len(args) > 0:
            # get argument name
            key, *args = args
            if key.type != 'NAME':
                raise ValueError(f'expected argument name, got {key}')  
            key = key.value

            # last arg is a flag
            if not args:
                attrs[key] = True
                break
            
            op, *args = args
            if op.type == ',':
                attrs[key] = True
                continue
            
            # expecting key value pair
            if op.type != '=':
                raise ValueError(f'expected "=" got {op.value}')
            
            if not args:
                raise ValueError(f'unexpected end of argument, missing value')

            value, *args = args

            if value.type == 'STRING_LITERAL':
                value = value.value[1:-1]
                attrs[key] = value
            elif value.type == 'INT_CONST_DEC':
                attrs[key] = int(value.value)
            elif value.type =="INT_CONST_HEX":
                attrs[key] = int(value.value)
            elif value.type == "INT_CONST_BIN":
                attrs[key] = int(value.value)
            elif value.type ==   "INT_CONST_OCT":
                attrs[key] = int(value.value)
            elif value.type == "FLOAT_CONST":
                attrs[key] = float(value.value)
            elif value.type =="HEX_FLOAT_CONST":
                attrs[key] = float(value.value)
            elif value.type == "true":
                attrs[key] = True
            elif value.type == "false":
                attrs[key] = True
            else:
                raise ValueError(f'unsopported literal type {value.type}') 

            if not args:
                break

            comma, *args = args
            if comma.type != ',':
                raise ValueError('expected comma after key-value pair')
                
        return attrs    


    def on_attributes(self, tokens: List[lexer.LexToken]) -> None:
        self._attrs = tokens

    def on_parse_start(self, state: cpplib.SNamespaceBlockState) -> None:
        self._attrs = None
        return super().on_parse_start(state)

    def on_extern_block_start(self, state: cpplib.SNamespaceBlockState):
        self._attrs = None
        return super().on_extern_block_start(state)
    
    def on_extern_block_end(self, state: cpplib.SExternBlockState):
        self._attrs = None
        return super().on_extern_block_end(state)

    def on_namespace_start(self, state: cpplib.SNamespaceBlockState):
        self._attrs = None
        return super().on_namespace_start(state)
    
    def on_namespace_end(self, state: cpplib.SNamespaceBlockState):
        self._attrs = None
        return super().on_namespace_end(state)

    def on_concept(self, state: cpplib.SNonClassBlockState, concept: cpplib.Concept):
        self._attrs = None
        return super().on_concept(state, concept)

    def on_namespace_alias(
        self, state: cpplib.SNonClassBlockState, alias: cpplib.NamespaceAlias
    ) -> None:
        self._attrs = None
        return super().on_namespace_alias(state, alias)

    def on_forward_decl(self, state: cpplib.SState, fdecl: cpplib.ForwardDecl) -> None:
        self._attrs = None
        return super().on_forward_decl(state, fdecl)

    def on_template_inst(self, state: cpplib.SState, inst: cpplib.TemplateInst) -> None:
        self._attrs = None
        return super().on_template_inst(state, inst)

    def on_variable(self, state: cpplib.SState, v: cpplib.Variable) -> None:
        self._attrs = None
        return super().on_variable(state, v)
    
    def on_function(self, state: cpplib.SNonClassBlockState, fn: cpplib.Function) -> None:
        self._attrs = None
        return super().on_function(state, fn)

    def on_method_impl(self, state: cpplib.SNonClassBlockState, method: cpplib.Method) -> None:
        self._attrs = None
        return super().on_method_impl(state, method)

    def on_typedef(self, state: cpplib.SState, typedef: cpplib.Typedef) -> None:
        self._attrs = None
        return super().on_typedef(state, typedef)

    def on_using_namespace(
        self, state: cpplib.SNonClassBlockState, namespace: List[str]
    ) -> None:
        self._attrs = None
        return super().on_using_namespace(state, namespace)

    def on_using_alias(self, state: cpplib.SState, using: cpplib.UsingAlias) -> None:
        self._attrs = None
        return super().on_using_alias(state, using)

    def on_using_declaration(self, state: cpplib.SState, using: cpplib.UsingDecl) -> None:
        self._attrs = None
        return super().on_using_declaration(state, using)

    def on_enum(self, state: cpplib.SState, enum: cpplib.EnumDecl) -> None:
        self._attrs = None
        return super().on_enum(state, enum)

    def on_class_method(self, state: cpplib.SClassBlockState, method: cpplib.Method) -> None:
        self._attrs = None
        return super().on_class_method(state, method)

    def on_class_friend(self, state: cpplib.SClassBlockState, friend: cpplib.FriendDecl) -> None:
        self._attrs = None
        return super().on_class_friend(state, friend)

    def on_class_end(self, state: cpplib.SClassBlockState) -> None:
        self._attrs = None  
        return super().on_class_end(state)

    def on_deduction_guide(
        self, state: cpplib.SNonClassBlockState, guide: cpplib.DeductionGuide
    ) -> None:
        self._attrs = None
        return super().on_deduction_guide(state, guide)


class LazyHeaderCollector:
    def __init__(self, parser: 'ReflectableFinder', header: str):
        self._parser: Final = parser
        self._header: Final = header
        self._header_id: Optional[str] = None

    def use_id(self) -> int:
        if self._header_id is None:
            self._header_id = len(self._parser._headers)
            self._parser._headers.append(self._header)
            
        return self._header_id
    
class LazyScopeCollector:
    def __init__(self, parser: 'ReflectableFinder', scope: ScopeDescription):
        self._parser: Final = parser
        self._scope: Final = scope
        self._scope_id: Optional[str] = None

    def use_id(self) -> int:
        if self._scope_id is None:
            self._scope_id = len(self._parser._scopes)
            self._parser._scopes.append(self._scope)
            
        return self._scope_id
    
    def add_namespace(self, namespace_name: str) -> 'ReflectableFinder.LazyScopeCollector':
        sub_namespace = self._scope.add_namespace(namespace_name)
        return LazyScopeCollector(self._parser, sub_namespace)
        
    def add_class(self, class_name: str) -> 'ReflectableFinder.LazyScopeCollector':
        sub_class = self._scope.add_class(class_name)
        return LazyScopeCollector(self._parser, sub_class)
    
    def add_field(self, field_name: str) -> 'ReflectableFinder.LazyScopeCollector':
        sub_field = self._scope.add_field(field_name)
        return LazyScopeCollector(self._parser, sub_field)
    
    @property
    def path(self):
        return self._scope.path



ClassFilter = Callable[[cpplib.ClassScope], bool]

class ReflectableFinder:
    def __init__(self):
        self._headers: List[str] = []
        self._fields: Properties = []
        self._classes: list[ReflectableDescription] = []
        # self._scopes: list[ScopeDescription] = []

    @property
    def reflectables(self) -> List[ReflectableDescription]:
        return self._classes

    @property
    def headers(self) -> Headers:
        return self._headers
     
    @property
    def fields(self) -> Properties:
        return self._fields


    def add_from_file(self, file_name: Path) -> None:
        lines = self._read_code_from_file(file_name)
        visitor = AnnotationParser(lines)
        code = ''.join(lines)
        Parser(file_name, code, visitor).parse()
        parsed = visitor.data
        self._find_classes(ScopeDescription.from_root(parsed.namespace), file_name)

    # private

    def _find_classes(self, scope: ScopeDescription, header: Path) -> None:
        self._collect_classes_in_namespace(scope, header)
        for ns in scope.namespace().namespaces.values():
            sub_namespace_scope = scope.add_namespace(ns)
            self._find_classes(sub_namespace_scope, header)

    def _collect_classes_in_namespace(self, scope: ScopeDescription, header: Path):
        for cls in scope.namespace().classes:
            cls_scope = scope.add_class(cls)
            self._collect_class_and_nested_classes(cls_scope, header)


    def _collect_class_and_nested_classes(self, scope: ScopeDescription, header: Path):
        cls = scope.cls()
        attrs = scope.attrs()
        name = None
        description = None

        if attrs is not None:
            if 'name' in attrs:
                name = attrs['name'] 
            if 'description' in attrs:
                description = attrs['description']

        properties = self._extract_properties(scope, header)
        reflectable = ReflectableDescription(
            cls_name=scope.name(), 
            full_name = scope,
            header=header,
            raw=cls,
            friendly_name=name, 
            description=description, 
            properties=properties
        )
        self._classes.append(reflectable)

        for nested_cls in cls.classes:
            nested_cls_scope = scope.add_class(nested_cls)
            self._collect_class_and_nested_classes(nested_cls_scope, header)


    def _collect_nested_classes(self):
        pass
        
    def _extract_properties(self, scope: ScopeDescription, header: Path) -> Properties:
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

            field_name = field.name.format()

            type_name = self._full_find_field_type(field_scope)
            name = None
            description = None

            # only annotated fields are considered properties
            attrs = field.attrs
            if attrs is not None:
                if 'name' in attrs:
                    name = attrs['name']
                if 'description' in attrs:
                    description = attrs['description']

            prop = PropertyDescription(
                field_name=field_name, 
                full_name=field_scope,
                raw=field,
                type=type_name, 
                friendly_name=name, 
                description=description,
                attrs=attrs,
            )
            # self._fields.append(prop)
            properties.append(prop)

        return properties

    def _full_find_field_type(self, scope: ScopeDescription):
        field = scope.field()
        field_type, is_anonymous = normalize_typename(field.type.typename)
        if is_anonymous:
            return f'{scope.parent}::{field_type}'
        
        for cls in self._classes:
            cls_name = cls.full_name.path
            encl_scope = scope.parent
            while encl_scope:
                full_name = f'{encl_scope}::{field_type}'
                if cls_name == full_name:
                    return cls_name
                encl_scope = encl_scope.parent

        return field_type        

    @staticmethod
    def _read_code_from_file(header_path: Path) -> str:
        code = []
        with open(header_path) as f:
            for line in f:
                if not line.strip().startswith('#'):
                    code.append(line)
        return code


__all__ = ['ReflectableFinder']
