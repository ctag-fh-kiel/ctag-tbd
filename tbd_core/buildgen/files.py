import logging
from pathlib import Path

import tbd_core.utils as utils

from .registry import generated_tbd_global
from .build_generator import get_build_generator, add_include_dir

_LOGGER = logging.getLogger(__file__)


PATHS_DOMAIN = 'paths'


@generated_tbd_global(PATHS_DOMAIN)
def get_build_path() -> Path:
    return Path(get_build_generator().build_path)


@generated_tbd_global(PATHS_DOMAIN)
def get_components_build_path():
    return Path() / 'src' / 'tbd'


@generated_tbd_global(PATHS_DOMAIN)
def get_tests_build_path():
    return Path() / 'test'


@generated_tbd_global(PATHS_DOMAIN)
def get_tbd_source_root() -> Path:
    return Path(__file__).parent.parent.parent


@generated_tbd_global(PATHS_DOMAIN)
def get_tbd_components_root():
    return get_tbd_source_root() / 'components'


@generated_tbd_global(PATHS_DOMAIN)
def get_vendor_root():
    return get_tbd_source_root / 'vendor'


@generated_tbd_global(PATHS_DOMAIN)
def get_generated_sources_path() -> Path:
    relative_to_build_path = Path() / 'src' / 'generated'
    source_path = get_build_path() / relative_to_build_path
    source_path.mkdir(parents=True, exist_ok=True)
    return relative_to_build_path


@generated_tbd_global(PATHS_DOMAIN)
def get_generated_include_path() -> Path:
    relative_to_build_dir = get_generated_sources_path() / 'include'
    include_path = get_build_path() / relative_to_build_dir
    include_path.mkdir(parents=True, exist_ok=True)
    return relative_to_build_dir


@generated_tbd_global(PATHS_DOMAIN)
def get_messages_path() -> Path:
    relative_to_build_dir = get_generated_sources_path() / 'messages'
    message_dir = get_build_path() / relative_to_build_dir
    message_dir.mkdir(parents=True, exist_ok=True)
    return relative_to_build_dir


def update_build_file_if_outdated(
        source_file: Path | str,
        dest_file: Path | str,
        *,
        symlink=True
) -> Path:
    """ Copy or symlink source files to build directory.

    Target has to be in project build directory. If dest_file is relative path, it will be appended to build directory
    path. See tbd_core.utils.update_file_if_outdated for details.
    """

    build_path = get_build_path()

    dest_file = Path(dest_file)
    dest_file = dest_file if dest_file.is_absolute() else build_path / dest_file
    if not dest_file.is_relative_to(build_path):
        raise RuntimeError(f'absolute build file paths have to be relative to build dir, got {dest_file}')

    return utils.update_file_if_outdated(source_file, build_path, dest_file, symlink=symlink)


def update_build_tree_if_outdated(
        source_dir: Path | str,
        dest_dir: Path | str,
        *,
        patterns: list[str] | None = None,
        flatten: bool = False,
        symlink: bool = True,
        ignore: list[str] | None = None,
) -> list[Path]:
    """ Copy or symlink sources in path to build directory.

    Target directory dest_dir has to be in project build directory. If dest_dir is relative path, it will be appended
    to build directory path. See tbd_core.utils.update_tree_if_outdated for details.
    """

    build_path = get_build_path()

    dest_dir = Path(dest_dir)
    dest_dir = dest_dir if dest_dir.is_absolute() else build_path / dest_dir
    if not dest_dir.is_relative_to(build_path):
        raise RuntimeError(f'absolute build dir paths have to be relative to build dir, got {dest_dir}')

    return utils.update_tree_if_outdated(source_dir, dest_dir,
                                         patterns=patterns, flatten=flatten, symlink=symlink, ignore=ignore)


def batch_update_build_files_if_outdated(
        source_dir: Path | str,
        dest_dir: Path | str,
        files: list[Path | str],
        *,
        sub_dir: Path | str | None = None,
        flatten: bool = False,
        symlink: bool = True,
        ignore: list[str] | None = None
) -> list[Path]:
    """ Copy or symlink list of sources to build directory.

    Target directory dest_dir has to be in project build directory. If dest_dir is relative path, it will be appended
    to build directory path. See tbd_core.utils.batch_update_files_if_outdated for details.
    """

    build_path = get_build_path()

    dest_dir = Path(dest_dir)
    dest_dir = dest_dir if dest_dir.is_absolute() else build_path / dest_dir
    if not dest_dir.is_relative_to(build_path):
        raise RuntimeError(f'absolute build dir paths have to be relative to build dir, got {dest_dir}')

    return utils.batch_update_files_if_outdated(source_dir, dest_dir, files,
                                                sub_dir=sub_dir, flatten=flatten, symlink=symlink, ignore=ignore)


__all__ = [
    'get_build_path', 
    'get_components_build_path',
    'get_tests_build_path',
    'get_tbd_components_root', 
    'get_tbd_source_root',
    'get_messages_path',
    'get_vendor_root', 
    'get_generated_sources_path', 
    'get_generated_include_path',
    'update_build_file_if_outdated',
    'update_build_tree_if_outdated',
    'batch_update_build_files_if_outdated'
]
