from collections import deque
from dataclasses import dataclass
from typing import Deque, Dict, List, Tuple
from cxxheaderparser.lexer import LexToken, LexerTokenStream
from cxxheaderparser.simple import SClassBlockState, SimpleCxxVisitor, Field

EXPRESSION_DELIMITERS = set([';', '{', '}'])

ArgType = bool | int | float | str
PositionalArgs = List[ArgType]
KeywordArgs = Dict[str, ArgType]
Attribute = Tuple[str, PositionalArgs, KeywordArgs]


def parse_attr_lists(attr_lists: Deque[Deque[LexToken]]) -> List[Attribute]:
    """ Parses and combines multiple attribute lists 
    
        .. note:: 
            Attributes outside the ``tbd`` namespace will be ignored.
    """
    attrs = []
    for attr_list in attr_lists:
        attrs.extend(parse_attr_list(attr_list))
    return attrs

def parse_attr_list(attr_list: Deque[LexToken]) -> List[Attribute]:
    """ Parses a simple attribute list 
    
        .. note:: 
            Attributes outside the ``tbd`` namespace will be ignored.
    """
    raw_attrs = []

    while len(attr_list) > 0:
        name, args = feed_attr(attr_list)
        if name[0] == 'tbd':
            raw_attrs.append((name, args))

    attrs = []
    for name, args in raw_attrs:
        name = '.'.join(name)
        if args is None:
            attrs.append((name, [], {}))
        else:
            attrs.append((name, *parse_args(args)))
    return attrs

def parse_args(arg_list: List[List[LexToken]]) -> Tuple[PositionalArgs, KeywordArgs]:
    """Parse an attributes arguments and return a tuple containing positional and kwargs """

    args = []
    kwargs = {}

    pos_args_done = False
    for arg in arg_list:
        if arg[0].type == 'NAME':
            pos_args_done = True

        if pos_args_done:
            if len(arg) != 3 or arg[1].type != '=':
                raise ValueError('expected keyword arg')
            name = arg[0]
            if name.type != 'NAME':
                raise ValueError('expected keyword arg name')
            kwargs[name.value] = parse_value(arg[2])
            
        else:
            if len(arg) != 1:
                raise ValueError('invalid positional arg')
            args.append(parse_value(arg[0]))

    return args, kwargs

def parse_value(value: LexToken) -> ArgType:
    """Turn attribute argument c++ literal into python value"""

    if value.type == 'true':
        return True
    elif value.type == 'false':
        return False
    elif value.type in ['INT_CONST_DEC', 'INT_CONST_HEX', 'INT_CONST_OCT', 'INT_CONST_BIN']:
        return int(value.value)
    elif value.type == 'FLOAT_CONST':
        return float(value.value)
    elif value.type == 'STRING_LITERAL':
        return value.value[1:-1]
    else:
        raise ValueError(f'unsupported attribute argument type {value.type}')

def feed_attr(attr_list: Deque[LexToken]):
    name = []
    args = None
    
    expecting_name = True
    while len(attr_list) > 0:
        name_part = attr_list.popleft()
        if name_part.type == '(':
            if expecting_name:
                raise ValueError('expected name, got (')
            args = feed_arguments(attr_list)
            if len(attr_list) > 0 and attr_list.popleft().type != ',':
                raise ValueError('expected , between attributes')
            break
        elif name_part.type == ',':
            if expecting_name:
                raise ValueError('expected name, got ,')
            break
        elif name_part.type == 'DBL_COLON':
            expecting_name = True
        else:
            if name_part.type != 'NAME':
                raise ValueError(f'expected name, got {name_part.type}')
            name.append(name_part.value)
            expecting_name = False

    return name, args
    

def feed_arguments(attr_list: Deque[LexToken]):
    OPENING_PARS = '({['
    CLOSING_PARS = ')}]'

    args = []
    current_arg = []
    
    par_stack = ['(']
    while len(attr_list) > 0:
        arg_part = attr_list.popleft()

        if arg_part.type in OPENING_PARS:
            par_stack.append(arg_part)
        elif arg_part.type in CLOSING_PARS:
            par = par_stack.pop()
            if OPENING_PARS[CLOSING_PARS.index(arg_part.type)] != par:
                raise ValueError(f'unbalanced parenthesis, stray closing {par_stack}')
            if len(par_stack) == 0:
                break
        elif arg_part.type == ',' and len(par_stack) == 1:
            args.append(current_arg)
            current_arg = []
        else:
            current_arg.append(arg_part)
    if len(current_arg) > 0:
        args.append(current_arg)
    return args


# 
#
#
class AttributeParser(SimpleCxxVisitor):
    """ Search for TBD attribute annotations on classes and class fields

    All TBD attributes are located in the ``tbd`` namespace or any namespace therein.
    They can be simple flag arguments such as

    .. code-block:: cpp

        [[ tbd::a_flag ]] float example_field;

    or have multiple python like arguments, with positional followed by keyword arguments:

    .. code-block:: 

        [[ tbd::some_prop("hello", 23.4, some_other_arg = 12) ]]

    Supported argument types are c++ literal that translate to python value:

    - ``bool``:   ``true`` and ``false`` bool literal
    - ``int``:    all different c++ int literals such as ``123``, ``0xa3``
    - ``float``:  floating point literals, without type specifier ``12.3`` not ``12.3f`` 
    - ``str``:    c++ string literals (only "..." allowed)
    """

    def __init__(self, lines) -> None:
        self._lines = lines
        super().__init__()

    def on_class_start(self, state: SClassBlockState) -> None:
        try:
            attributes = self._get_attrs(state.location.lineno)
            state.class_decl.attributes = attributes
        except Exception as e:
            print(e)
        
        return super().on_class_start(state)

    def on_class_field(self, state: SClassBlockState, f: Field) -> None:
        try:
            attributes = self._get_attrs(state.location.lineno)
            f.attributes = attributes
        except Exception as e:
            print(e)
        f.is_writable_property = None
        return super().on_class_field(state, f)

    def _get_attrs(self, line_number: int):
        attr_lists = self._get_attr_lists(line_number)
        return parse_attr_lists(attr_lists)

    def _get_attr_lists(self, line_number: int) -> Dict[str, str]:
        """ search backwards to get the entire struct or field declaration

            The the full declaration includes all attributes

            - current expression is presumed to end at the previous ``{``, ``}`` or ``;``
            - ``{``, ``}``, ``;`` are ignored if they are enclosed in ``[[ ... ]]``
        """

        attr_tokens = deque()
        attr_lists = deque()
        feeding_attrs = False
        line_index = line_number - 1
        ignore_terminator = True

        while line_index >= 0:
            for token in self._get_line_tokens(line_index, reversed=True):
                if token.type == 'DBL_RBRACKET': 
                    if feeding_attrs:
                        raise ValueError(f'nested attribute delimiters ]]: {line_number}')
                    feeding_attrs = True
                elif token.type == 'DBL_LBRACKET':
                    if not feeding_attrs:
                        raise ValueError(f'missing opening ]] attribute delimiter: {line_number}')
                    feeding_attrs = False
                    attr_lists.appendleft(attr_tokens)
                    attr_tokens = deque()
                elif token.type in EXPRESSION_DELIMITERS: 
                    if not ignore_terminator:
                        if feeding_attrs:
                            raise ValueError(f'missing closing [[ attribute delimiter: {line_number}')
                        line_index = -1
                        break
                elif feeding_attrs:
                    attr_tokens.appendleft(token)

            ignore_terminator = False
            line_index -= 1
        return attr_lists
        
    def _get_line_tokens(self, line_index: int, *, reversed: bool = False) -> List[LexToken]:
        stream = LexerTokenStream(None, self._lines[line_index])
        retval = []
        while token := stream.token_eof_ok():
            retval.append(token)
        if reversed:
            retval.reverse()
        return retval


__all__ = ['AttributeParser']
