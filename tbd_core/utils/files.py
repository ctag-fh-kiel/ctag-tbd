import fnmatch
import logging
import os
from pathlib import Path
import shutil
from typing import Iterable

_LOGGER = logging.getLogger(__file__)


def update_file_if_outdated(
        source_file: Path | str,
        dest_dir: Path | str,
        dest_file: Path | str,
        *,
        symlink: bool = False,
) -> Path:
    """ Copy or symlink a file.

    Copying only occurs if the file is out of date.

    :param source_file: file to update, has to be absolute path
    :param dest_dir: target base directory (additional subpaths in dest_file path will be appended)
    :param dest_file: absolute or relative path to target file (if absolute, has to start with dest_dir)
    :param symlink: if set, create symlink instead of copying file
    :return: absolute path to new file
    """

    source_file = Path(source_file)
    if not source_file.is_absolute():
        raise RuntimeError(f'source_file has to be absolute path, got {source_file}')
    
    if not dest_dir.is_absolute():
        raise RuntimeError(f'target dir must be absolute path, got {dest_dir}')
    
    dest_file = Path(dest_dir) / dest_file if not dest_file.is_absolute() else dest_file
    if not dest_file.is_relative_to(dest_dir):
        raise RuntimeError(f'absolute target file paths have to be relative to target dir, got {dest_file}')

    dest_file.parent.mkdir(parents=True, exist_ok=True)

    if symlink:
        if dest_file.exists():
            if not dest_file.is_symlink():
                raise RuntimeError(f'destination file exists and is not a symlink {dest_file}')
            return dest_file

        _LOGGER.debug(f'creating symlink: {source_file} -> {dest_file}')
        os.symlink(source_file, dest_file)
    else:
        if dest_file.exists():
            if not dest_file.is_file():
                raise RuntimeError(f'destination file exists and is not a file {dest_file}')
            if dest_file.is_symlink():
                raise RuntimeError(f'destination file exists and is a symlink {dest_file}')
            if source_file.stat().st_mtime <= dest_file.stat().st_mtime:
                return dest_file

        _LOGGER.debug(f'copy to sources: {source_file} -> {dest_file}')
        shutil.copyfile(source_file, dest_file)

    return dest_file


def update_files_if_outdated_with_filter(
    source_dir: Path,
    dest_dir: Path,
    files: Iterable[Path],
    *,
    flatten: bool = False,
    symlink: bool = False,
    ignore: list[str] | None = None
) -> list[Path]:
    """ Copy or symlink list of files.

    Copying only occurs if the file is out of date.

    :note: Operates on a per-file basis, thus subdirectories in the target tree are not symlinks.

    :param source_dir: base path of file list (has to be absolute)
    :param dest_dir: target base directory (additional subpaths in file paths will be appended)
    :param files: list of files (paths have to be absolute and relative to source_dir)
    :param flatten: if set, all files are copied or symlinked into root of dest_dir
    :param symlink: if set, create symlink instead of copying files
    :param ignore: patterns of files to ignore
    :return: absolute paths to new files
    """

    if not source_dir.is_absolute():
        raise RuntimeError(f'source_dir has to be absolute path, got {source_dir}')

    if not dest_dir.is_absolute():
        raise RuntimeError(f'target dir must be absolute path, got {dest_dir}')

    retval = []
    for source_file in files:
        if not source_file.is_file():
            continue
        if ignore and any(fnmatch.fnmatch(source_file.name, pattern) for pattern in ignore):
            continue

        relative_source_file = dest_dir / source_file.name if flatten else source_file.relative_to(source_dir)
        dest_file = dest_dir / relative_source_file
        if update_file_if_outdated(source_file, dest_dir, dest_file, symlink=symlink):
            retval.append(dest_file)
    return retval


def update_tree_if_outdated(
    source_dir: Path | str,
    dest_dir: Path | str,
    *,
    patterns: list[str] | None = None,
    flatten: bool = False,
    symlink: bool = False,
    ignore: list[str] | None = None,
) -> list[Path]:
    """ Copy or symlink files in paths.

    Copying only occurs if the file is out of date. Either all files are copied/symlinked or specific file patterns
    can be specified. Performs deep copy of files.

    :note: Operates on a per-file basis, thus subdirectories in the target tree are not symlinks.

    :param source_dir: base path of file list (has to be absolute)
    :param dest_dir: target base directory (additional subpaths in file paths will be appended)
    :param patterns: file name glob patterns of files to copy/symlink (defaults to *)
    :param flatten: if set, all files are copied or symlinked into root of dest_dir
    :param symlink: if set, create symlink instead of copying files
    :param ignore: patterns of files to ignore
    :return: absolute paths to new files
    """

    if patterns is None:
        patterns = ['*']

    source_dir = Path(source_dir)
    dest_dir = Path(dest_dir)

    retval = []
    for pattern in patterns:
        retval += update_files_if_outdated_with_filter(source_dir, dest_dir, source_dir.rglob(pattern),
                                                       flatten=flatten, symlink=symlink, ignore=ignore)

    return retval


def batch_update_files_if_outdated(
        source_dir: Path | str,
        dest_dir: Path | str,
        files: list[Path | str],
        *,
        sub_dir: Path | str | None = None,
        flatten: bool = False,
        symlink: bool = False,
        ignore: list[str] | None = None,
) -> list[Path]:
    """ Copy or symlink files from patterns.

    Copying only occurs if the file is out of date. File list can be a combination of regular glob patterns  (recursive
    globs need to be explicitly specified) or simple file paths.

    :note: Operates on a per-file basis, thus subdirectories in the target tree are not symlinks.

    :param source_dir: base path of file list (has to be absolute)
    :param dest_dir: target base directory (additional subpaths in file paths will be appended)
    :param files: list of file paths/glob patterns
    :param sub_dir: interpret glob patterns relative to subdirectory of source-dir
    :param flatten: if set, all files are copied or symlinked into root of dest_dir
    :param symlink: if set, create symlink instead of copying files
    :param ignore: patterns of files to ignore
    :return: absolute paths to new files
    """

    source_dir = Path(source_dir)
    dest_dir = Path(dest_dir)

    if not source_dir.is_absolute():
        raise RuntimeError(f'source_dir has to be absolute path, got {source_dir}')

    search_dir = source_dir
    if sub_dir is not None:
        sub_dir = Path(sub_dir)
        if sub_dir.is_absolute():
            raise RuntimeError(f'sub_dir paths have to be relative, got {sub_dir}')
        search_dir = source_dir / sub_dir

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

        retval += update_files_if_outdated_with_filter(source_dir, dest_dir, source_files,
                                                       flatten=flatten, symlink=symlink, ignore=ignore)

    return retval


__all__ = [
    'update_file_if_outdated',
    'update_tree_if_outdated',
    'batch_update_files_if_outdated',
]
