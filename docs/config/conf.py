import os
from pathlib import Path


if (doxygen_xml_dir := os.getenv('TBD_DOCS_BUILD_DI')) is None:
    print('THIS')
    doxygen_xml_dir = (Path(__file__).parent.parent.parent) / 'build' / 'docs'
else:
    print('THAT')
    doxygen_xml_dir = Path(doxygen_xml_dir)
doxygen_xml_dir = doxygen_xml_dir / 'code_xml'

# -- Project information -----------------------------------------------------

project = 'TBD'
copyright = '2024 CTAG'
author = 'CTAG'

# -- General configuration ---------------------------------------------------

extensions = [
    'sphinxcontrib.youtube',
    'breathe'
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
