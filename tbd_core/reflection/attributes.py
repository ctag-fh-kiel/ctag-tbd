from dataclasses import dataclass


AttributeArgTypes = int | float | bool | str

@dataclass
class Attribute:
    """ C++ attribute declaration.

        Attribute van be plain or have arguments as key value list:

        `[[ tbd::some_attr(key1="value", key2=12, key3=3.141) ]]`
    """

    name_segments: list[str]
    params: dict[str, AttributeArgTypes]

    @property
    def name(self) -> str:
        return '::'.join(self.name_segments)

    def __str__(self):
        return self.name

    def __repr__(self):
        return f'Attribute({self.name})'


Attributes = list[Attribute]


__all__ = [
    'AttributeArgTypes',
    'Attribute',
    'Attributes',
]