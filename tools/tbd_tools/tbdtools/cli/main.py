
from tbdtools.cmd_utils import greeting_header
from .docs_cmd import docs_group
from .plugins_cmd import plugins_group
from .firmware_cmd import firmware_group
from .config_cmd import config_group
from .project_cmd import project_group
from tbdtools.cmd_utils import get_main

app = get_main()

app.add_typer(docs_group, name='docs')
app.add_typer(plugins_group, name='plugins')
app.add_typer(firmware_group, name='firmware')
app.add_typer(config_group, name='config')
app.add_typer(project_group, name='project')

@app.command()
def no_args():
    print(greeting_header)

tbd_cli = app

__all__ = ['tbd_cli']