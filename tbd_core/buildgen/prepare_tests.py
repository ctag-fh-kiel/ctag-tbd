from pathlib import Path

from . import get_generated_include_path
from .files import get_components_build_path, copy_tree_if_outdated, get_tests_build_path
from .build_generator import add_platformio_block
from .component_info import COMPONENTS_DOMAIN, ComponentInfo
from .registry import get_tbd_domain


def prepare_tests():
    indent = ' ' * 4
    unittest_path = get_tests_build_path()
    all_tests_header_dir = get_generated_include_path() / 'tbd' / 'unittests'

    test_env_block = [
        '[env:tests]',
        'platform = native',
        'test_framework = googletest',
        'build_flags =',
        f'{indent}; core flags',
        f'{indent}-DTBD_UNITTEST_BUILD',
        f'{indent}-DUSE_HOST',
        f'{indent}-Isrc/generated/include',
        f'{indent}-I{unittest_path}',
        ''
    ]
    test_sources = []

    for module in get_tbd_domain(COMPONENTS_DOMAIN).values():
        if not isinstance(module, ComponentInfo):
            raise ValueError(f'bad module type in TBD modules list {type(module)}')

        if module.defines:
            test_env_block.append(f'{indent}; set by {module.name}')

        for key, value in module.defines.items():
            test_env_block.append(f'{indent}-D{key}={value}')

        component_build_dir = get_components_build_path() / module.full_name
        for path in module.include_dirs:
            test_env_block.append(f'{indent}-I{component_build_dir / path}')


        if module.tests_dir:
            test_sources.extend(copy_tree_if_outdated(module.tests_dir, unittest_path))

    test_sources = [source_file.relative_to(unittest_path) for source_file in test_sources]

    all_tests_header_dir.mkdir(parents=True, exist_ok=True)
    with open(all_tests_header_dir / 'all_unittests.hpp', 'w') as f:
        f.write('#pragma once\n\n\n')
        for test_source in test_sources:
            f.write(f'#include <{test_source}>\n')

    test_env_block.append(f'{indent}-std=c++20')

    add_platformio_block([
        *test_env_block,
        'build_src_filter = ',
        f'{indent}+<../{unittest_path}/**/test_*.cpp>',
        f'test_dir = {unittest_path}',
    ])


__all__ = ['prepare_tests']
