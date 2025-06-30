import logging

import cxxheaderparser.lexer as lexer
import cxxheaderparser.simple as cpplib

from tbd_core.reflection.reflectables.attributes import Attributes

from .attribute_parser import parse_attributes


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
            elif tok.type == "alignas":
                next_tok = self._next_token_must_be("(")
                tokens = self._consume_balanced_tokens(next_tok)
                # attrs.append(AlignasAttribute(tokens))
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

        return [attr for tokens in self._attrs for attr in parse_attributes(tokens)]

    def _reset_attrs(self) -> None:
        self._attrs = []


__all__ = ['Parser', 'AnnotationParser']
