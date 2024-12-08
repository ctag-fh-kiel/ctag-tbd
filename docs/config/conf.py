import os
from pathlib import Path
import sys
from sphinx.util import logging

logger = logging.getLogger(__name__)


def find_project_path() -> Path:
    logger.verbose('determining project source root')

    logger.verbose('checking for TBD_PROJECT_DIR')
    # try env var
    if (project_path := os.getenv('TBD_PROJECT_DIR')) is not None:
        logger.info(f'TBD_PROJECT_DIR project root set: {project_path}')            
        return Path(project_path).resolve()
    

    logger.verbose('checking for git repo root')
    # try to find git root
    try:
        import git
        git_repo = git.Repo(os.getcwd(), search_parent_directories=True)
        git_root = git_repo.git.rev_parse("--show-toplevel")
        logger.info(f'git repo project root found: {project_path}')  
        return Path(git_root).resolve()
    except:
        pass

    # try relative to current file
    project_path = Path(__file__).parent.parent
    logger.verbose(f'falling back to relative location: {project_path}')
    return project_path  

def check_tbd_tools_import():
    logger.info('checking if tbd_sphinx is in module path')
    try:
        import tbdtools.tbd_sphinx
        logger.info('loaded tbd_sphinx')
        return
    except:
        pass

    logger.info('trying to find tbd_sphinx in project files')
    try:
        tbd_python_path = find_project_path() / 'tools'
        logger.info(f'adding project folder {tbd_python_path} to module search path')
        sys.path.append(str(tbd_python_path))

        import tbdtools.tbd_sphinx
        logger.info('loaded tbd_sphinx')
        return
    except Exception as e:
        print(e)

    raise ImportError('failed to import TBD sphinx plugin tbdtools.tbd_sphinx')

def get_build_dir() -> Path:
    return find_project_path() / 'build' / 'docs'


def get_doxygen_xml_dir() -> Path:
    return get_build_dir() / 'code_xml'


# check_tbd_tools_import()

doxygen_xml_dir = get_doxygen_xml_dir()


# -- Project information -----------------------------------------------------

project = 'TBD'
copyright = '2024 CTAG'
author = 'CTAG'

# -- General configuration ---------------------------------------------------

extensions = [
    'sphinxcontrib.youtube',
    'breathe',
    # 'tbdtools.tbd_sphinx',
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
