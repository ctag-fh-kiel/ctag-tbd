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

project = 'dada tbd/docs'
html_title = 'dada tbd/docs'
copyright = '2014-2026 dadamachines / CTAG'
author = 'dadamachines'

rst_prolog = """.. attention::
    This documentation is a work in progress and may be incomplete or subject to change.
"""

# -- General configuration ---------------------------------------------------

extensions = [
    'sphinxcontrib.youtube',
    'breathe',
    'sphinx.ext.githubpages',
    'sphinx_copybutton',
    'sphinx_design',
    'ablog',  # Blog plugin
    'myst_parser',  # Markdown support
    # 'tbdtools.tbd_sphinx',
]

# -- ABlog Configuration -----------------------------------------------------
blog_baseurl = "https://dadamachines.github.io/ctag-tbd/"
blog_title = "dada tbd blog"
blog_path = "blog"
blog_post_pattern = "blog/posts/*"
blog_feed_fulltext = True
blog_feed_subtitle = "Latest posts from the dadamachines TBD project"
blog_authors = {
    "dadamachines": ("dadamachines", "https://dadamachines.com"),
}
blog_default_author = "dadamachines"
post_date_format = "%B %d, %Y"
post_show_prev_next = True
post_auto_orphan = True

# Disable ABlog sidebar widgets (we handle navigation via Furo)
blog_sidebars = []

# -- MyST Configuration ------------------------------------------------------
myst_enable_extensions = [
    "colon_fence",
    "deflist",
    "html_image",
]

exclude_patterns = []

templates_path = ['../_templates']

# -- Options for HTML output -------------------------------------------------


html_theme = 'furo'

# Furo theme options
html_theme_options = {
    "navigation_with_keys": True,
    "sidebar_hide_name": False,
    "light_css_variables": {
        "color-code-background": "#1e1e2e",
        "color-code-foreground": "#cdd6f4",
    },
}

# Dark code blocks in both light and dark mode
pygments_style = "monokai"
pygments_dark_style = "monokai"

# html_sidebars = {
#     '**': ['globaltoc.html', 'localtoc.html', 'relations.html', 'searchbox.html']
# }

html_context = {
    'default_mode': 'auto'
}

html_static_path = ['../_static']
html_css_files = ['blog.css', 'plugins.css']
html_favicon = 'assets/favicon.png'
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
