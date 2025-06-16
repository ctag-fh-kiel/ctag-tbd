from pathlib import Path
import jinja2 as ji
from collections import OrderedDict
import re
import logging

from tbd_core.buildgen import get_generated_sources_path, ComponentInfo, get_tbd_domain, COMPONENTS_DOMAIN, add_define
from tbd_core.buildgen.prepare_build import SOURCE_PATTERNS

_LOGGER = logging.getLogger(__name__)


ERR_EXPR = re.compile(r'(#define\s+)?(TBD_ERR)\s*\(\s*(?P<error_name>\w+)\s*\)')
ERRMSG_EXPR = re.compile(r'(#define\s+)?(TBD_NEW_ERR|TBD_ERRMSG)\s*\(\s*(?P<error_name>\w+)\s*,\s*"(?P<error_msg>.*)"\s*\)')

errors = OrderedDict()

def write_error_info():
    template_path = Path(__file__).parent / 'templates'
    env = ji.Environment(loader=ji.FileSystemLoader(template_path), autoescape=ji.select_autoescape())

    sorted_errors = [('SUCCESS', 'no error'), ('FAILURE', 'unknown error'), *errors.items()]
    indexed_errors = [(index, name, description) for index, (name, description) in enumerate(sorted_errors)]

    source = env.get_template('all_errors.cpp.j2').render(errors=indexed_errors)
    with open(get_generated_sources_path() / 'all_errors.cpp', 'w') as f:
        f.write(source)

    source = env.get_template('all_errors.hpp.j2').render(errors=indexed_errors)
    header_path = get_generated_sources_path() / 'include' / 'tbd' / 'errors'
    header_path.mkdir(parents=True, exist_ok=True)
    with open(header_path /  'all_errors.hpp', 'w') as f:
        f.write(source)

def add_error(name: str, message: str | None) -> None:
    if name not in errors:
        errors[name] = message
        return
    if not message:
        return
    if current_message := errors[name]:
        _LOGGER.warning(f'error message redefined {current_message} -> {message}')
    errors[name] = message


def collect_errors_for_file(source_file: Path):
    with open(source_file, 'r') as f:
        code = f.read()
        if found_errors := [*re.findall(ERR_EXPR, code), *re.findall(ERRMSG_EXPR, code)]:
            for define, macro, name, *args in found_errors:
                if define:
                    continue

                if not args:
                    add_error(name, None)
                elif len(args) == 1:
                    add_error(name, args[0])
                else:
                    raise ValueError(f'error macro with invalid argument number {len(args)}')


def collect_errors_for_dir(source_dir: Path):
    for pattern in SOURCE_PATTERNS:
        for source_file in source_dir.rglob(pattern):
            collect_errors_for_file(source_file)


def collect_errors_for_component(component: ComponentInfo):
    sources = {*component.include_dirs, *component.sources}
    for path in sources:
        source_path = component.path / path
        if source_path.is_file():
            collect_errors_for_file(source_path)
        elif source_path.is_dir():
            collect_errors_for_dir(source_path)
        else:
            raise ValueError(f'unknown source path type {path}')


def collect_errors():
    for component in get_tbd_domain(COMPONENTS_DOMAIN).values():
        if not isinstance(component, ComponentInfo):
            raise ValueError(f'bad module type in TBD modules list {type(component)}')
        collect_errors_for_component(component)

    write_error_info()
    add_define('USE_TBD_ERR', True)
