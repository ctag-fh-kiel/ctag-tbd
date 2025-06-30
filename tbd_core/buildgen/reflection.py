from .component_info import get_tbd_components, AutoReflection
from .registry import generated_tbd_global

from tbd_core.reflection.registry import ReflectableFinder


APIS_GLOBAL = 'reflection'


@generated_tbd_global(APIS_GLOBAL)
def get_reflection_registry() -> ReflectableFinder:
    return ReflectableFinder()


def auto_reflect_component():
    components = get_tbd_components()

    for component in components.values():
        if component.reflect in [AutoReflection.HEADERS, AutoReflection.ALL]:
            for include_dir in component.include_dirs:
                include_dir = component.path / include_dir
                for header in include_dir.rglob('*.hpp'):
                    get_reflection_registry().add_from_file(component.name, header, include_base=include_dir)

        if component.reflect in [AutoReflection.SOURCES, AutoReflection.ALL]:
            for source_dir in component.sources:
                source_dir = component.path / source_dir
                for source in source_dir.rglob('*.[ch]pp'):
                    get_reflection_registry().add_from_file(component.name, source, include_base=source_dir)


__all__ = ['get_reflection_registry', 'auto_reflect_component']
