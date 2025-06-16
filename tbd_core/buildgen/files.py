import fnmatch
import logging
import os
from pathlib import Path
import shutil

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
    # CORE.add_platformio_option('build_src_filter', [f'+<{PATH_RELATIVE_TO_BUILD_DIR}>'])
    return source_path


@generated_tbd_global(PATHS_DOMAIN)
def get_generated_include_path() -> Path:
    relative_to_build_dir = Path() / 'src' / 'generated' / 'include'
    include_path = get_generated_sources_path() / 'include'
    include_path.mkdir(parents=True, exist_ok=True)
    add_include_dir(relative_to_build_dir)
    return include_path


def copy_file_if_outdated(source_file: Path | str, dest_file: Path | str, *, symlink=True) -> list[Path]:
    source_file = Path(source_file)
    build_path = get_build_path()
    dest_file = build_path / dest_file
    dest_file.parent.mkdir(parents=True, exist_ok=True)
    if dest_file.exists() and  source_file.stat().st_mtime <= dest_file.stat().st_mtime:
        return [dest_file]
    _LOGGER.debug(f'copy to sources: {source_file}')
    if symlink:
        os.symlink(source_file, dest_file)
    else:
        shutil.copyfile(source_file, dest_file)

    return [dest_file]


def copy_tree_if_outdated(
    source_dir: Path | str,
    dest_dir: Path | str,
    *,
    patterns: list[str] | None = None,
    flatten: bool = False,
    symlink: bool = True,
    ignore: list[str] | None = None,
) -> list[Path]:

    if patterns is None:
        patterns = ['*']

    source_dir = Path(source_dir)
    dest_dir = Path(dest_dir)

    retval = []
    for pattern in patterns:
        for source_file in source_dir.rglob(pattern):
            if not source_file.is_file():
                continue
            if ignore and any(fnmatch.fnmatch(source_file.name, pattern) for pattern in ignore):
                continue

            relative_source_file = dest_dir / source_file.name if flatten else source_file.relative_to(source_dir)
            dest_file = dest_dir / relative_source_file
            if copy_file_if_outdated(source_file, dest_file, symlink=symlink):
                retval.append(dest_file)

    return retval


def copy_batch(
        source_dir: Path | str,
        dest_dir: Path | str,
        files: list[Path | str],
        *,
        sub_dir: Path | str | None = None,
        flatten: bool = False,
        symlink: bool = True,
        ignore: list[str] | None = None,
) -> list[Path]:

    search_dir = Path(source_dir) / sub_dir if sub_dir else source_dir

    retval = []
    for file in files:
        source_path = Path(search_dir) / file

        source_path_str = str(file)
        if '*' in source_path_str:
            source_files = []
            for source_file in search_dir.glob(source_path_str):
                source_files.append(source_file)
        else:
            source_files = [source_path]

        for source_file in source_files:
            if not source_file.is_file():
                _LOGGER.error(f'{source_file} is not a file')
                continue
            if ignore and any(fnmatch.fnmatch(source_file.name, pattern) for pattern in ignore):
                continue
            relative_source_file = dest_dir / source_file.name if flatten else source_file.relative_to(source_dir)
            dest_file = dest_dir / relative_source_file
            if copy_file_if_outdated(source_file, dest_file, symlink=symlink):
                retval.append(dest_file)

    return retval



__all__ = [
    'get_build_path', 
    'get_components_build_path',
    'get_tbd_components_root', 
    'get_tbd_source_root', 
    'get_vendor_root', 
    'get_generated_sources_path', 
    'get_generated_include_path',
    'copy_file_if_outdated',
    'copy_tree_if_outdated',
    'copy_batch',
]
