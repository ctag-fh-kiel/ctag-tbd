from .generation_stages import GenerationStages
from .component_info import get_tbd_components, AutoReflection
from .registry import generated_tbd_global

from tbd_core.reflection.registry import ReflectableFinder
from tbd_core.reflection.db import ReflectableDB


REFLECTION_REGISTRY_GLOBAL = 'reflection_registry'
REFLECTION_GLOBAL = 'reflection'


@generated_tbd_global(REFLECTION_REGISTRY_GLOBAL)
def get_reflection_registry() -> ReflectableFinder:
    """ Get reflection registry pre finalization. """

    return ReflectableFinder()


@generated_tbd_global(REFLECTION_GLOBAL, after_stage=GenerationStages.REFLECTION)
def get_reflectables() -> ReflectableDB:
    """ Get reflectables once the reflection has been parsed. """

    return get_reflection_registry().get_reflectables()


def auto_reflect_component():
    components = get_tbd_components()
    reflection_registry = get_reflection_registry()

    for component in components.values():
        if component.reflect in [AutoReflection.HEADERS, AutoReflection.ALL]:
            for include_dir in component.include_dirs:
                include_dir = component.path / include_dir
                for header in include_dir.rglob('*.hpp'):
                    reflection_registry.add_from_file(component.name, header, include_base=include_dir)

        if component.reflect in [AutoReflection.SOURCES, AutoReflection.ALL]:
            for source_dir in component.sources:
                source_dir = component.path / source_dir
                for source in source_dir.rglob('*.[ch]pp'):
                    reflection_registry.add_from_file(component.name, source, include_base=source_dir)

__all__ = [
    'get_reflection_registry',
    'get_reflectables',
    'auto_reflect_component',
]
