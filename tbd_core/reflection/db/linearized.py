from typing import Iterator

from .pointers import ClassPtr, PropertyPtr


def flattened_properties(cls: ClassPtr, path: list[str] | None = None) -> Iterator[tuple[list[str], PropertyPtr]]:
    if path is None:
        path = []

    for prop in cls.properties:
        this_path = [*path, prop.field_name]
        prop_type = prop.type
        match prop_type:
            case str():
                yield this_path, prop
            case ClassPtr():
                for sub_path, sub_prop in flattened_properties(prop_type, this_path):
                    yield path + sub_path, sub_prop
            case _:
                raise ValueError(f'invalid property type: {prop_type}')


__all__ = ['flattened_properties']