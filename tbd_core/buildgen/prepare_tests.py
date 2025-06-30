from .files import (
    get_components_build_path,
    get_build_path,
    update_build_tree_if_outdated,
    get_tests_build_path,
    get_generated_include_path,
    get_messages_path,
)
from .build_generator import add_platformio_block
from .component_info import ComponentInfo, get_tbd_components


def prepare_tests():
    indent = ' ' * 4
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

    absolute_test_build_path = get_build_path() / unittest_path
    test_sources += [source.relative_to(absolute_test_build_path) for source in test_sources]

    all_tests_header_dir.mkdir(parents=True, exist_ok=True)
    with open(all_tests_header_dir / 'all_unittests.hpp', 'w') as f:
        f.write('#pragma once\n\n\n')
        for test_source in test_sources:
            f.write(f'#include <{test_source}>\n')

    test_env_block.append(f'{indent}-std=c++20')

    add_platformio_block([
        *test_env_block,
        'lib_deps = ',
        f'{indent}google/googletest@^1.15.2',
        'build_src_filter = ',
        f'{indent}+<../{unittest_path}/**/test_*.cpp>',
        # f'{indent}test_dir = {unittest_path}',
    ])


__all__ = ['prepare_tests']
