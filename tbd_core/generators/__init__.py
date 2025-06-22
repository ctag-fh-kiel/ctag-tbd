import inspect
import subprocess
from pathlib import Path
from typing import Callable, Any, Final, TypeVar, Generic

import jinja2 as ji

def jilter(f: Callable[[Any], Any]) -> Callable[[Any], Any]:
    """ Mark method as jinja filter. """
    f._is_jilter = True
    return f


def generate_protos(out_dir: Path | str, input_dir: Path | str, input_files: list[Path | str]) -> None:
    out_dir.mkdir(parents=True, exist_ok=True)
    import tbd_core.buildgen.build_deps as build_deps
    build_deps.python_dependencies('nanopb')
    subprocess.run([
        'python', '-m', 'nanopb.generator.nanopb_generator',
        '--output-dir', out_dir,
        '--proto-path', input_dir,
        *input_files,
    ])


FiltersT = TypeVar("FiltersT")

class GeneratorBase(Generic[FiltersT]):
    def __init__(self, templates_path: Path, filters: FiltersT | None = None) -> None:
        self._templates: Final[Path] = templates_path
        self._env = ji.Environment(loader=ji.FileSystemLoader(templates_path), autoescape=ji.select_autoescape())

        self._filters: FiltersT | None = None
        if filters:
            self._filters = self._add_filters(filters)
            self._env.filters |= self._filters


    def render(self, template_file: Path | str, **args) -> str:
        template = self._env.get_template(str(template_file))
        return template.render(**args)

    @staticmethod
    def _add_filters(obj):
        filters = {}
        for name, _ in inspect.getmembers(obj, predicate=inspect.isroutine):
            if name.startswith('_'):
                continue
            method = obj.__getattribute__(name)
            if not hasattr(method, '_is_jilter'):
                continue

            # sig = inspect.signature(method)
            # if len(sig.parameters) != 1:
            #     raise RuntimeError(f'filter {name} must have exactly 1 parameter')
            filters[name] = method
        return filters


__all__ = [
    'jilter',
    'generate_protos',
    'GeneratorBase',
]