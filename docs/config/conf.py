import os
from pathlib import Path
import git
import sys


def find_project_path() -> Path:
    # try env var
    if (project_path := os.getenv('TBD_PROJECT_DIR')) is not None:
        return Path(project_path).resolve()
    
    # try to find git root
    try:
        git_repo = git.Repo(os.getcwd(), search_parent_directories=True)
        git_root = git_repo.git.rev_parse("--show-toplevel")
        return Path(git_root).resolve()
    except:
        pass

    # try relative to current file
    return Path(__file__).parent.parent

try:
    import tbdtools.sphinx
except:
    sys.path.append(find_project_path() / 'tools' / 'tbd_tool')


def get_build_dir() -> Path:
    return find_project_path() / 'build' / 'docs'


def get_doxygen_xml_dir() -> Path:
    return get_build_dir() / 'code_xml'


doxygen_xml_dir = get_doxygen_xml_dir()


# -- Project information -----------------------------------------------------

project = 'TBD'
copyright = '2024 CTAG'
author = 'CTAG'

# -- General configuration ---------------------------------------------------

extensions = [
    'sphinxcontrib.youtube',
    'breathe',
    'tbdtools.sphinx',
]

exclude_patterns = []


# -- Options for HTML output -------------------------------------------------


html_theme = 'pydata_sphinx_theme'
html_sidebars = {
    '**': ['globaltoc.html', 'localtoc.html', 'relations.html', 'searchbox.html']
}

html_context = {
    'default_mode': 'auto'
}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named 'default.css' will overwrite the builtin 'default.css'.
#html_static_path = ['_static']
#
# source_suffix = {
#     '.rst': 'restructuredtext',
#     '.txt': 'markdown',
#     '.md': 'markdown',
# }
#

breathe_projects = {'ctag_tbd': doxygen_xml_dir}
breathe_default_project = 'ctag_tbd'
breathe_domain_by_extension = {
    'h' : 'cpp',
    'hpp': 'cpp',
    'c' : 'cpp',
    'cpp': 'cpp',
}
# breathe_projects_source = {
#     "auto" : (
#         "../components/ctagSoundProcessor", ["ctagSoundProcessor.hpp"]
#     )
# }
# breathe_build_directory = build_dir / 'autodocs'

# def setup(app):
#     pass
    # app.add_config_value('recommonmark_config', {
    #     'enable_eval_rst': True
    # }, True)
    # app.add_transform(AutoStructify)
