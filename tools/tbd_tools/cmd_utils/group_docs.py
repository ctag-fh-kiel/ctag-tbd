from pathlib import Path
from typing import Dict, Set
from git import List, Tuple
import typer
from typer.core import TyperGroup, TyperCommand

# hierarchy of rst heading chars
# tuples of (char, with_overline)
rst_headings = [
    ('*', True),
    ('\'', True),
    ('=', False),
    ('-', False),
    ('^', False),
    ('"', False),
]

class CliDocsBuilder:
    def __init__(self, base_heading: int = 1):
        self._docs = []
        self._base_heading = base_heading

    @property
    def docs(self):
        return '\n\n'.join(self._docs)

    def add_no_args_hint(docs):
        pass

    def start_group(self, name, docs, parents):
        depth = len(parents)

        body = docs if docs is not None else ''

        if depth == 0:
        # root command
            
            pass
        else:
        # first level subcommands

            heading = self._rst_heading(name, self._base_heading + depth)
            part = f'{heading}\n\n{docs}'
            self._docs.append(part)

    def add_command(self, name, docs, group, parents):
        parents_title = ' '.join(parents)
        title = f'``{parents_title}`` **{name}**'
        if docs is None:
            part = f'{title}\n\n'
        else:
            body = '\n'.join([f'   {line}' for line in docs.split('\n')])
            part = f'{title}\n\n{body}\n\n'
        self._docs.append(part)

    @staticmethod
    def _rst_heading(title, level):
        heading_char, needs_overline = rst_headings[level]
        heading_line = heading_char * len(title)
        if needs_overline:
            return f'{heading_line}\n{title}\n{heading_line}'
        else:
            return f'{title}\n{heading_line}'
        

TyperCommands = Dict[str, TyperCommand]
TyperGroups = Dict[str, TyperGroup]


def _build_group_doc_tree(group: TyperGroup, builder: CliDocsBuilder, route: List[str], seen: Set[TyperGroup]):
    commands, groups = _sort_groups_and_command(group)

    # loop detection, just to be safe
    if group in seen:
        return
    seen.add(group)

    group_name = group.name
    builder.start_group(group_name, group.help, route)

    for cmd_name, cmd in commands.items():
        if len(route) == 1 and cmd_name == 'no-args':
            builder.add_no_args_hint(cmd.help)
        else:
            builder.add_command(cmd_name, cmd.help, group_name, route)

    new_route = [*route, group.name]
    for group in groups.values():
        _build_group_doc_tree(group, builder, new_route, seen)        


def _sort_groups_and_command(group: TyperGroup) -> Tuple[TyperCommands, TyperGroups]:
    child_list = group.commands
    commands = {child_name: child for child_name, child in child_list.items() if isinstance(child, typer.core.TyperCommand)}
    groups = {child_name: child for child_name, child in child_list.items() if isinstance(child, typer.core.TyperGroup)}
    return commands, groups


def generate_docs_for_app(app: typer.Typer):
    builder = CliDocsBuilder()
    group = typer.main.get_group(app)
    _build_group_doc_tree(group, builder, [], set())
    return builder.docs
    

def write_docs_for_cli(general: str, cli: typer.Typer, out_file: Path):
    docs = generate_docs_for_app(cli)

    file_contents = f'{general}\n{docs}'

    out_file.parent.mkdir(parents=True, exist_ok=True)
    with open(out_file, 'w') as f:
        f.write(file_contents)


__all__ = ['generate_docs_for_app', "write_docs_for_cli"]