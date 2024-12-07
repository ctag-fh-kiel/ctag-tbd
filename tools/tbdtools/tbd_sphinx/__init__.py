from __future__ import annotations
import importlib
import re

from docutils import nodes
from git import Optional
from sphinx.application import Sphinx
from sphinx.util.docutils import SphinxDirective
from sphinx.util.typing import ExtensionMetadata
from sphinx.util import logging

logger = logging.getLogger(__name__)


from tbdtools.cmd_utils import generate_docs_for_app


_strip_hyphens_exp = re.compile(r'"?(?P<name>\w+)"?')

def strip_hyphens(string: str) -> str:
    if (match := _strip_hyphens_exp.match(string)):
        return match.group('name')
    return None


class CliApi(SphinxDirective):
    """ add directive that inserts typer application docs """

    required_arguments = 2

    def run(self) -> list[nodes.Node]:
        def error(msg, err: Optional[Exception] = None):
            logger.error(f'{msg}')
            if err is not None:
                logger.error(err)
            return [nodes.paragraph(text=f'error: {msg}')]

        first_arg, second_arg, *_ = self.arguments

        cli_module_name = strip_hyphens(first_arg)
        if not cli_module_name:
            return error(f'bad first argument {first_arg}')
        
        cli_name = strip_hyphens(second_arg)
        if not cli_module_name:
            return error(f'bad second argument {second_arg}')

        try:
            cli_module = importlib.import_module(cli_module_name)
            cli = getattr(cli_module, cli_name)

            try:
                docs = generate_docs_for_app(cli)
                try:
                    container = nodes.Element()
                    return self.parse_text_to_nodes(docs)
                except Exception as err:
                    return error('parsing RST failed', err)

            except Exception as err:
                return error(f'failed to generate docs for {cli_module_name}.{cli_name}', err)

        except Exception as err:
            return error(f'failed to load CLI class {cli_module_name}.{cli_name}', err)


def setup(app: Sphinx) -> ExtensionMetadata:
    app.add_directive('cli-api', CliApi)

    return {
        'version': '0.1',
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }
