from pathlib import Path
import typer
from subprocess import run

from project import ProjectStructure
from preprocessor import search_for_plugins, write_plugin_factory_header, write_meta_class, update_plugin_config

plugins_group = typer.Typer()


@plugins_group.command('find')
def find_plugins_cmd(
    ctx: typer.Context,
    strict: bool = typer.Option(False)
):
    """ list all available plugins """

    dirs: ProjectStructure = ctx.obj

    headers = dirs.plugins.glob('*.hpp')
    _, plugins = search_for_plugins(headers, strict)
    for plugin in plugins:
        print(plugin.name)


@plugins_group.command('update-config')
def update_config_cmd(
    ctx: typer.Context,
    strict: bool = typer.Option(False)
):
    """ update the plugin config to match plugins detected"""

    dirs: ProjectStructure = ctx.obj

    headers = dirs.plugins.glob('*.hpp')
    _, plugins = search_for_plugins(headers, strict)
    config_file = dirs.plugins_config
    update_plugin_config(plugins, config_file)



@plugins_group.command('create-factory')
def find_plugins_cmd(
    ctx: typer.Context,
    strict: bool = typer.Option(False)
):
    """ create plugin factory """

    dirs: ProjectStructure = ctx.obj

    headers = dirs.plugins.glob('*.hpp')
    headers, plugins = search_for_plugins(headers, strict)
    out_file = dirs.plugin_registry / 'ctagSoundProcessorFactory.hpp'
    write_plugin_factory_header(headers, plugins, out_file)


@plugins_group.command('create-meta')
def create_meta_cmd(
    ctx: typer.Context,
    strict: bool = typer.Option(False)
):
    """ create reflection information for plugins """

    dirs: ProjectStructure = ctx.obj

    headers = dirs.plugins.glob('*.hpp')
    headers, plugins = search_for_plugins(headers, strict)
    out_file = dirs.plugin_registry / 'ctagSoundProcessorsMeta'
    write_meta_class(headers, plugins, out_file)


__all__ = ['plugins_group']