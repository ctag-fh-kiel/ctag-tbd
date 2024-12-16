from pathlib import Path
import typer
from typing import Optional

from tbdtools.cmd_utils import get_ctx
from tbdtools.project import ProjectRoot
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
    _ctx: typer.Context,
    strict: bool = typer.Option(False)
):
    """ list all available plugins """

    dirs = get_ctx(_ctx).dirs

    headers = dirs.src.plugins.headers().glob('*.hpp')
    _, plugins = search_for_plugins(headers, strict)
    for plugin in plugins:
        print(plugin.name)


@plugins_group.command('update-config')
def update_config_cmd(
    _ctx: typer.Context,
    strict: bool = typer.Option(False)
):
    """ update the plugin config to match plugins detected"""

    dirs = get_ctx(_ctx).dirs

    headers = dirs.src.sounds.headers().glob('*.hpp')
    _, plugins = search_for_plugins(headers, strict)
    config_file = dirs.resources.plugin_definitions()
    update_plugin_config(plugins, config_file)



@plugins_group.command('create-factory')
def create_factory_cmd(
    _ctx: typer.Context,
    strict: bool = typer.Option(False),
    out_file: Optional[Path] = typer.Option(None, '--out', '-o')
):
    """ create plugin factory """

    dirs = get_ctx(_ctx).dirs

    headers = dirs.src.sounds.headers().glob('*.hpp')
    headers, plugins = search_for_plugins(headers, strict)
    if out_file is None:
        out_file = dirs.src.sound_registry.headers() / 'sound_processor_factory.hpp'
    print(out_file)
    write_plugin_factory_header(headers, plugins, out_file)


@plugins_group.command('create-meta')
def create_meta_cmd(
    _ctx: typer.Context,
    strict: bool = typer.Option(False)
):
    """ create reflection information for plugins """

    dirs = get_ctx(_ctx).dirs

    headers = dirs.src.sounds.headers().glob('*.hpp')
    headers, plugins = search_for_plugins(headers, strict)
    out_file = dirs.src.sound_registry.headers() / 'ctagSoundProcessorsMeta'
    write_meta_class(headers, plugins, out_file)


@plugins_group.command('pretty-configs')
def pretty_configs_cmd(
    _ctx: typer.Context,
    out_dir: Path
):
    """ create reflection information for plugins """

    dirs = get_ctx(_ctx).dirs

    configs_path = dirs.resources.plugin_definitions()

    write_pretty_configs(configs_path, out_dir)


@plugins_group.command('plugin-schema')
def extract_configs_cmd(
    _ctx: typer.Context,
):
    """ create reflection information for plugins """

    print(get_plugin_schema())


@plugins_group.command('preset-schema')
def presets_schema_cmd(
    _ctx: typer.Context,
):
    """ create reflection information for plugins """

    print(get_preset_schema())


__all__ = ['plugins_group']