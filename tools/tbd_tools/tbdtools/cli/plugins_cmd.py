from pathlib import Path
import typer

from tbdtools.project import ProjectStructure
from tbdtools.preprocessor import (
    search_for_plugins, 
    write_plugin_factory_header, 
    write_meta_class, 
    update_plugin_config,
    write_pretty_configs,
    get_plugin_schema,
    get_preset_schema
)


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


@plugins_group.command('pretty-configs')
def extract_configs(
    ctx: typer.Context,
    out_dir: Path
):
    """ create reflection information for plugins """

    dirs: ProjectStructure = ctx.obj

    configs_path = dirs.plugin_configs

    write_pretty_configs(configs_path, out_dir)


@plugins_group.command('plugin-schema')
def extract_configs(
    ctx: typer.Context,
):
    """ create reflection information for plugins """

    print(get_plugin_schema())


@plugins_group.command('preset-schema')
def extract_configs(
    ctx: typer.Context,
):
    """ create reflection information for plugins """

    print(get_preset_schema())


__all__ = ['plugins_group']