import os
from pathlib import Path
from typing import Tuple

import git


def _get_project_repo() -> git.Repo:
    cwd = Path(os.getcwd()) 
    return git.Repo(cwd, search_parent_directories=True)


def find_root_from_cwd() -> str:
    git_repo = _get_project_repo()
    git_root = git_repo.git.rev_parse("--show-toplevel")
    return git_root


def get_project_info() -> Tuple[str, str]:
    git_repo = _get_project_repo()
    git_cmd = git.Git()
    if (description := git_cmd.describe('--tags')):
        tag, *_, sha = description.split('-')
    return tag, sha


__all__ = ['find_root_from_cwd', 'get_project_info']
