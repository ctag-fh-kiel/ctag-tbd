import dataclasses

from pathlib import Path
from typing import Final, OrderedDict
import logging

import cxxheaderparser.lexer as lexer
import cxxheaderparser.simple as cpplib
import cxxheaderparser.types as cpptypes

from .reflectables import (
    NamespaceDescription,
    Properties, 
    PropertyDescription, 
    ReflectableDescription,
    FunctionDescription,
)
from .scopes import ScopeDescription, normalize_typename
from .attributes import Attribute, Attributes


_LOGGER = logging.getLogger(__file__)


class Parser(cpplib.CxxParser):
    """ Hack to implement missing attribute parsing/processing of cxxheaderparser lib.

        This hack will intercept any `[[` sequence and forward the enclosed tokens to visitor for processing.
    """

    def _consume_attribute_specifier_seq(
        self, tok: lexer.LexToken, doxygen: str | None = None
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
    """ C++ visitor with additional processing of attributes.

        Attributes will be added to context and picked up/associated with the next declaration. Attributes will be
        added to
    """

    def __init__(self, lines) -> None:
        self._attrs: list[list[lexer.LexToken]] | None = None
        super().__init__()

    def on_class_start(self, state:cpplib.SClassBlockState):
        super().on_class_start(state)
        state.class_decl.attrs = self._parse_attrs()
        self._reset_attrs()


    def on_class_field(self, state:cpplib.SClassBlockState, f: cpplib.Field) -> None:
        super().on_class_field(state, f)

        try:
            f.attrs = self._parse_attrs()
        except Exception as e:
            _LOGGER.error(f'{state.location.filename}{state.location.lineno}: {e}')
            f.attrs = None

        self._reset_attrs()

    def on_parse_start(self, state: cpplib.SNamespaceBlockState) -> None:
        self._reset_attrs()
        return super().on_parse_start(state)

    def on_extern_block_start(self, state: cpplib.SNamespaceBlockState):
        self._reset_attrs()
        return super().on_extern_block_start(state)
    
    def on_extern_block_end(self, state: cpplib.SExternBlockState):
        self._reset_attrs()
        return super().on_extern_block_end(state)

    def on_namespace_start(self, state: cpplib.SNamespaceBlockState):
        self._reset_attrs()
        return super().on_namespace_start(state)
    
    def on_namespace_end(self, state: cpplib.SNamespaceBlockState):
        self._reset_attrs()
        return super().on_namespace_end(state)

    def on_concept(self, state: cpplib.SNonClassBlockState, concept: cpplib.Concept):
        self._reset_attrs()
        return super().on_concept(state, concept)

    def on_namespace_alias(
        self, state: cpplib.SNonClassBlockState, alias: cpplib.NamespaceAlias
    ) -> None:
        self._reset_attrs()
        return super().on_namespace_alias(state, alias)

    def on_forward_decl(self, state: cpplib.SState, fdecl: cpplib.ForwardDecl) -> None:
        self._reset_attrs()
        return super().on_forward_decl(state, fdecl)

    def on_template_inst(self, state: cpplib.SState, inst: cpplib.TemplateInst) -> None:
        self._reset_attrs()
        return super().on_template_inst(state, inst)

    def on_variable(self, state: cpplib.SState, v: cpplib.Variable) -> None:
        self._reset_attrs()
        return super().on_variable(state, v)
    
    def on_function(self, state: cpplib.SNonClassBlockState, fn: cpplib.Function) -> None:
        fn.attrs = self._parse_attrs()
        self._reset_attrs()  
        return super().on_function(state, fn)

    def on_method_impl(self, state: cpplib.SNonClassBlockState, method: cpplib.Method) -> None:
        self._reset_attrs()
        return super().on_method_impl(state, method)

    def on_typedef(self, state: cpplib.SState, typedef: cpplib.Typedef) -> None:
        self._reset_attrs()
        return super().on_typedef(state, typedef)

    def on_using_namespace(
        self, state: cpplib.SNonClassBlockState, namespace: list[str]
    ) -> None:
        self._reset_attrs()
        return super().on_using_namespace(state, namespace)

    def on_using_alias(self, state: cpplib.SState, using: cpplib.UsingAlias) -> None:
        self._reset_attrs()
        return super().on_using_alias(state, using)

    def on_using_declaration(self, state: cpplib.SState, using: cpplib.UsingDecl) -> None:
        self._reset_attrs()
        return super().on_using_declaration(state, using)

    def on_enum(self, state: cpplib.SState, enum: cpplib.EnumDecl) -> None:
        self._reset_attrs()
        return super().on_enum(state, enum)

    def on_class_method(self, state: cpplib.SClassBlockState, method: cpplib.Method) -> None:
        self._reset_attrs()
        return super().on_class_method(state, method)

    def on_class_friend(self, state: cpplib.SClassBlockState, friend: cpplib.FriendDecl) -> None:
        self._reset_attrs()
        return super().on_class_friend(state, friend)

    def on_class_end(self, state: cpplib.SClassBlockState) -> None:
        self._reset_attrs()  
        return super().on_class_end(state)

    def on_deduction_guide(
        self, state: cpplib.SNonClassBlockState, guide: cpplib.DeductionGuide
    ) -> None:
        self._reset_attrs()
        return super().on_deduction_guide(state, guide)

    def on_attributes(self, tokens: list[lexer.LexToken]) -> None:
        self._attrs.append(tokens)

    # private

    def _parse_attrs(self) -> Attributes:
        if not self._attrs:
            return []
        return [attr for tokens in self._attrs if (attr := self._parse_attr(tokens)) is not None]

    @staticmethod
    def _parse_attr(tokens) -> Attribute | None:
        args = tokens
        name_segments = []

        while True:
            if not args:
                raise ValueError('expected attribute name')

            name, *args = args
            if name.type != 'NAME':
                raise ValueError('expected attribute name segment')
            # ignore non tbd attributes
            if not name_segments and name.value != 'tbd':
                return None

            name_segments.append(name.value)
            if not args:
                return Attribute(name_segments=name_segments, params={})

            delim, *args = args
            if delim.type == '(':
                break
            elif delim.type == 'DBL_COLON':
                continue
            else:
                raise ValueError('expected "(" or "::" in attribute name')

        if not args:
            raise ValueError('missing closing ")" for arg list')

        *args, last = args
        if last.type != ')':
            raise ValueError('encountered invalid attribute')

        params = {}
        while len(args) > 0:
            # get argument name
            key, *args = args
            if key.type != 'NAME':
                raise ValueError(f'expected argument name, got {key}')
            key = key.value

            # last arg is a flag
            if not args:
                params[key] = True
                break

            op, *args = args
            if op.type == ',':
                params[key] = True
                continue

            # expecting key value pair
            if op.type != '=':
                raise ValueError(f'expected "=" got {op.value}')

            if not args:
                raise ValueError(f'unexpected end of argument, missing value')

            value, *args = args

            sign = 1
            if value.type == '-':
                value, *args = args
                sign = -1

            if value.type == 'STRING_LITERAL':
                value = value.value[1:-1]
                params[key] = value
            elif value.type == 'INT_CONST_DEC':
                params[key] = sign * int(value.value)
            elif value.type == "INT_CONST_HEX":
                params[key] = sign * int(value.value)
            elif value.type == "INT_CONST_BIN":
                params[key] = sign * int(value.value)
            elif value.type == "INT_CONST_OCT":
                params[key] = sign * int(value.value)
            elif value.type == "FLOAT_CONST":
                params[key] = sign * float(value.value)
            elif value.type == "HEX_FLOAT_CONST":
                params[key] = sign * float(value.value)
            elif value.type == "true":
                params[key] = True
            elif value.type == "false":
                params[key] = True
            else:
                raise ValueError(f'unsopported literal type {value.type}')

            if not args:
                break

            comma, *args = args
            if comma.type != ',':
                raise ValueError('expected comma after key-value pair')

        return Attribute(name_segments=name_segments, params=params)

    def _reset_attrs(self) -> None:
        self._attrs = []


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


@dataclasses.dataclass
class ReflectableSubset:
    namespaces: list[NamespaceDescription] = dataclasses.field(default_factory=list)
    classes: list[ReflectableDescription] = dataclasses.field(default_factory=list)
    funcs: list[FunctionDescription] = dataclasses.field(default_factory=list)


class ReflectableFinder:
    def __init__(self):
        self._headers: list[str] = []
        self._namespaces: OrderedDict[int, NamespaceDescription] = OrderedDict()
        self._fields: OrderedDict[int, PropertyDescription] = OrderedDict()
        self._classes: OrderedDict[int, ReflectableDescription] = OrderedDict()
        self._funcs: OrderedDict[int, FunctionDescription] = OrderedDict()

    @property
    def reflectables(self) -> list[ReflectableDescription]:
        return [v for v in self._classes.values()]

    @property
    def headers(self) -> list[str]:
        return self._headers
     
    @property
    def fields(self) -> Properties:
        return [v for v in self._fields.values()]
    
    @property
    def funcs(self) -> list[FunctionDescription]:
        return [v for v in self._funcs.values()]

    def add_from_file(self, file_name: Path, *, include_base: Path | None = None) -> ReflectableSubset:
        lines = self._read_code_from_file(file_name)
        visitor = AnnotationParser(lines)
        code = ''.join(lines)
        Parser(str(file_name), code, visitor).parse()
        parsed = visitor.data

        scope = ScopeDescription.from_root(parsed.namespace)
        relative_header = file_name.relative_to(include_base) if include_base else file_name

        added = ReflectableSubset()
        self._find_entities_in_namespace(scope, relative_header, added)
        return added

    # private

    def _find_entities_in_namespace(self, scope: ScopeDescription, header: Path, added: ReflectableSubset) -> None:
        self._add_namespace(scope, added)
        self._collect_functions_in_namespace(scope, header, added)
        self._collect_classes_in_namespace(scope, header, added)

        for ns in scope.namespace().namespaces.values():
            sub_namespace_scope = scope.add_namespace(ns)

            self._find_entities_in_namespace(sub_namespace_scope, header, added)

    def _add_namespace(self, scope: ScopeDescription, added: ReflectableSubset) -> None:
        raw = scope.namespace()
        ns_hash = scope.hash
        if ns_hash not in self._namespaces:
            namespace = NamespaceDescription(scope, raw)
            self._namespaces[ns_hash] = namespace
            added.namespaces.append(namespace)

    def _collect_functions_in_namespace(self, scope: ScopeDescription, header: Path, added: ReflectableSubset) -> None:
        for raw_func in scope.namespace().functions:
            func_scope = scope.add_function(raw_func)
            self._collect_function(func_scope, header, added)

    def _collect_function(self, scope: ScopeDescription, header: Path, added: ReflectableSubset) -> None:
        func_hash = scope.hash
        if func_hash not in self._funcs:
            _LOGGER.warning(f'already encountered function {scope.path}')

        attrs = scope.attrs()
        name, description = name_and_description_from_attrs(attrs)

        func = FunctionDescription(
            raw=scope.func(),
            scope = scope,
            header=header,
            friendly_name=name, 
            description=description,
            attrs=attrs,
        )

        self._funcs[func_hash] = func
        added.funcs.append(func)

    def _collect_classes_in_namespace(self, scope: ScopeDescription, header: Path, added: ReflectableSubset) -> None:
        for cls in scope.namespace().classes:
            cls_scope = scope.add_class(cls)
            self._collect_class_and_nested_classes(cls_scope, header, added)

    def _collect_class_and_nested_classes(self, scope: ScopeDescription, header: Path, added: ReflectableSubset) -> None:
        cls_hash = scope.hash
        if cls_hash not in self._classes:
            _LOGGER.warning(f'already encountered class {scope.path}')

        cls = scope.cls()
        attrs = scope.attrs()
        name, description = name_and_description_from_attrs(attrs)

        type_id = scope.hash
        properties = self._extract_properties(scope, type_id)
        reflectable = ReflectableDescription(
            raw=cls,
            scope = scope,
            header=header,
            friendly_name=name, 
            description=description, 
            properties=properties
        )
        self._classes[scope.hash] = reflectable
        added.classes.append(reflectable)

        for nested_cls in cls.classes:
            nested_cls_scope = scope.add_class(nested_cls)
            self._collect_class_and_nested_classes(nested_cls_scope, header, added)

        
    def _extract_properties(self, scope: ScopeDescription, cls_id: int) -> Properties:
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

            type_name = self._full_find_field_type(field_scope)
            attrs = field_scope.attrs()
            name, description = name_and_description_from_attrs(attrs)

            prop = PropertyDescription(
                scope=field_scope,
                raw=field,
                cls_id=cls_id,
                type=type_name, 
                friendly_name=name, 
                description=description,
                attrs=attrs,
            )
            properties.append(prop)

        return properties

    def _full_find_field_type(self, scope: ScopeDescription):
        field = scope.field()
        field_type, is_anonymous = normalize_typename(field.type.typename)
        if is_anonymous:
            return f'{scope.parent}::{field_type}'
        
        for cls in self._classes.values():
            cls_name = cls.full_name
            encl_scope = scope.parent
            while encl_scope:
                full_name = f'{encl_scope}::{field_type}'
                if cls_name == full_name:
                    return cls_name
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


__all__ = ['Parser', 'AnnotationParser', 'ReflectableFinder']
