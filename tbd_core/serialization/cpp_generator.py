from pathlib import Path
from typing import Final, Iterator

from tbd_core.buildgen import get_tbd_components_root
from tbd_core.generators import GeneratorBase, jilter
from tbd_core.reflection.db import ReflectableDB, ClassPtr, flattened_properties, PropertyPtr


class CppFilters:
    def __init__(self, reflectable_db: ReflectableDB):
        self._reflectable_db: Final[ReflectableDB] = reflectable_db

    @jilter
    def field_name(self, property_id: int) -> str:
        return self._reflectable_db.get_property(property_id).field_name

    @jilter
    def snake_name(self, path: str) -> str:
        return path.replace('.', '_')

    @jilter
    def flattened_properties(self, serializable: ClassPtr) -> Iterator[tuple[str, PropertyPtr]]:
        for path, prop in flattened_properties(serializable):
            yield '.'.join(path), prop


class CppGenerator(GeneratorBase[CppFilters]):
    def __init__(self, reflectable_db: ReflectableDB):
        super().__init__(get_tbd_components_root() / 'core' / 'tbd_presets' / 'src', CppFilters(reflectable_db))
        self._reflectable_db: Final[ReflectableDB] = reflectable_db

    def render(self, template_file: Path | str, **args) -> str:
        return super().render(template_file, reflectables=self._reflectable_db, **args)
