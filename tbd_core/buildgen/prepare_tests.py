from .files import (
    get_components_build_path,
    get_build_path,
    update_build_tree_if_outdated,
    get_tests_build_path,
    get_generated_include_path,
    get_messages_path,
)
from .build_generator import add_platformio_block
from .component_info import ComponentInfo, get_tbd_components, ExternalDependency


INDENT_ONE = ' ' * 4
GOOGLE_TEST_DEP = ExternalDependency(ref='google/googletest', version='^1.15.2', repository=None)


def lib_dep_line(lib_dep: ExternalDependency, *, indent: str = INDENT_ONE) -> str:
    if lib_dep.version:
        return f'{indent}{lib_dep.ref}@{lib_dep.version}'
    return f'f{indent}{lib_dep.ref}'


def prepare_tests():
    indent = INDENT_ONE
    unittest_path = get_tests_build_path()
    all_tests_header_dir = get_build_path() / get_generated_include_path() / 'tbd' / 'unittests'

    test_env_block = [
        '[env:tests]',
        'platform = native',
        'test_framework = googletest',
        'build_flags =',
        f'{indent}; core flags',
        f'{indent}-DTBD_UNITTEST_BUILD',
        f'{indent}-DUSE_HOST',
        f'{indent}-I{get_generated_include_path()}',
        f'{indent}-I{get_messages_path()}',
        f'{indent}-I{unittest_path}',
        ''
    ]
    test_sources = []

    external_dependencies: list[ExternalDependency] = []

    for component in get_tbd_components().values():
        if not isinstance(component, ComponentInfo):
            raise ValueError(f'bad component type in TBD modules list {type(component)}')

        if component.defines:
            test_env_block.append(f'{indent}; set by {component.name}')

        for key, value in component.defines.items():
            test_env_block.append(f'{indent}-D{key}={value}')

        component_build_dir = get_components_build_path() / component.full_name
        for path in component.include_dirs:
            test_env_block.append(f'{indent}-I{component_build_dir / path}')

        if component.tests_dir:
            test_sources += update_build_tree_if_outdated(component.tests_dir, unittest_path)

        external_dependencies += component.external_dependencies

    absolute_test_build_path = get_build_path() / unittest_path
    test_sources += [source.relative_to(absolute_test_build_path) for source in test_sources]

    all_tests_header_dir.mkdir(parents=True, exist_ok=True)
    with open(all_tests_header_dir / 'all_unittests.hpp', 'w') as f:
        f.write('#pragma once\n\n\n')
        for test_source in test_sources:
            f.write(f'#include <{test_source}>\n')

    test_env_block.append(f'{indent}-std=c++20')

    lib_deps = [lib_dep_line(dep) for dep in [GOOGLE_TEST_DEP, *external_dependencies]]
    add_platformio_block([
        *test_env_block,
        'lib_deps = ',
        *lib_deps,
        'build_src_filter = ',
        f'{indent}+<../{unittest_path}/**/test_*.cpp>',
        # f'{indent}test_dir = {unittest_path}',
    ])


__all__ = ['prepare_tests']
