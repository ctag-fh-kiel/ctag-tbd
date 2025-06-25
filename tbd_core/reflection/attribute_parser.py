import cxxheaderparser.lexer as lexer

from .attributes import Attribute, Attributes, AttributeArgTypes

Token = lexer.LexToken
Tokens = list[lexer.LexToken]

def get_enclosed(head: Token, tokens: Tokens) -> tuple[Tokens, Tokens]:
    pairs = {'(': ')', '{': '}', '[': ']', '<': '>'}
    opening = list(pairs.keys())
    closing = list(pairs.values())

    stack = []
    if head.type not in opening:
        raise ValueError(f'expected one of {opening}, got {head.type}')
    stack.append(head.value)

    if not tokens:
        raise ValueError(f'unbalanced expression, expected closing {head.type}')

    enclosed = []
    tail = tokens
    while tail:
        token, *tail = tail
        token_type = token.type
        if token.type in opening:
            stack.append(token.type)
            continue

        if token_type in closing:
            top = stack.pop()
            expected = pairs[top]
            if token_type != expected:
                raise ValueError(f'expected {expected}, got {token_type}')
            if not stack:
                return enclosed, tail
            continue

        enclosed.append(token)

    raise ValueError(f'unbalanced expression, {stack} not closed')


def feed_attr_name_head(tokens: Tokens) -> tuple[str, Tokens]:
    if not tokens:
        raise ValueError('expected attribute name')

    name_segment, *tail = tokens
    if name_segment.type != 'NAME':
        raise ValueError('expected attribute name segment')
    return name_segment.value, tail


def feed_attr_name_tail(tokens: Tokens) -> tuple[list[str], Tokens] | None:
    name_segments = []
    tail = tokens

    while tail:
        delim, *tail = tail
        if delim.type in ['(', ',']:
            return name_segments, [delim, *tail]
        elif delim.type != 'DBL_COLON':
            raise ValueError('expected "(" or "::" in attribute name')

        if not tail:
            raise ValueError('expected name segment')

        name_segment, *tail = tail
        if name_segment.type != 'NAME':
            raise ValueError('expected attribute name segment')

        name_segments.append(name_segment.value)

    return name_segments, []


def feed_attribute(tokens: Tokens) -> tuple[list[str], Tokens, Tokens]:
    name_head, tail = feed_attr_name_head(tokens)
    name_tail, tail = feed_attr_name_tail(tail)
    name = [name_head, *name_tail]

    arg_list = []
    if not tail:
        return name, arg_list, []

    comma_or_opening, *tail = tail
    if comma_or_opening.type == '(':
        arg_list, tail = get_enclosed(comma_or_opening, tail)
        if not tail:
            return name, arg_list, []
        comma_or_opening, *tail = tail

    if comma_or_opening.type != ',':
        raise ValueError('expected ,')

    return name, arg_list, tail

def parse_arg_list(tokens: Tokens) -> dict[str, AttributeArgTypes]:
    tail = tokens

    params = {}
    while tail:
        # get argument name
        key, *tail = tail
        if key.type != 'NAME':
            raise ValueError(f'expected argument name, got {key}')
        key = key.value

        # last arg is a flag
        if not tail:
            params[key] = True
            break

        op, *tail = tail
        if op.type == ',':
            params[key] = True
            continue

        # expecting key value pair
        if op.type != '=':
            raise ValueError(f'expected "=" got {op.value}')

        if not tail:
            raise ValueError(f'unexpected end of argument, missing value')

        value, *tail = tail

        sign = 1
        if value.type == '-':
            value, *tail = tail
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

        if not tail:
            break

        comma, *tail = tail
        if comma.type != ',':
            raise ValueError('expected comma after key-value pair')
    return params

def parse_attributes(tokens: Tokens) -> Attributes:
    attributes = []
    tail = tokens
    while tail:
        name, arg_list, tail = feed_attribute(tail)
        if name[0] != 'tbd':
            continue
        args = parse_arg_list(arg_list)
        attributes.append(Attribute(name_segments=name, params=args))

    return attributes

__all__ = [
    'parse_attributes',
]