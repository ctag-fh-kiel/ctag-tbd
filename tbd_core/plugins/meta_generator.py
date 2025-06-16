from functools import lru_cache
import logging
from pathlib import Path
import jinja2 as ji

from tbd_core.reflection import Headers, ReflectableFinder, Reflectables, ReflectableDescription

from .plugin_entry import ParamEntry
from .plugins import Plugins


__LOGGER = logging.getLogger(__file__)


def setter_name(param: ParamEntry):
    return f'set_{param.snake_name}'

@lru_cache
def templates() -> ji.Environment:
    template_path = Path(__file__).parent / 'templates'
    env = ji.Environment(loader=ji.FileSystemLoader(template_path), autoescape=ji.select_autoescape())
    env.filters['setter'] = setter_name
    return env
 

def get_template(name: str) -> ji.Template:
    return templates().get_template(name)


def search_for_plugins(headers: Headers, strict: bool) -> list[ReflectableDescription]:
    collector = ReflectableFinder()

    for header in headers:
        if strict:
            collector.add_from_file(header)
        else:
            try:
                collector.add_from_file(header)
            except Exception as e:
                __LOGGER.error(f'error parsing {header}: {e}')
    return collector.reflectables

def write_plugin_reflection_info(plugins: Plugins, out_folder: Path):
    template = get_template('all_sound_processors.cpp.j2')
    source = template.render(plugins=plugins.plugin_list, params=plugins.param_list)

    out_folder.mkdir(exist_ok=True, parents=True)
    out_file = out_folder / 'plugin_info.cpp'
    with open(out_file, 'w') as f:
        f.write(source)   

def write_plugin_factory_header(headers: Headers, plugins: Plugins, out_folder: Path) -> None:
    template = get_template('factory.cpp.j2')
    header = template.render(plugins=plugins.plugin_list, headers=headers)

    out_folder.parent.mkdir(parents=True, exist_ok=True)
    with open(out_folder / 'factory.cpp', 'w') as f:
        f.write(header)    


def write_meta_classes(plugins: Plugins, out_folder: Path) -> None:
    header_template = get_template('sound_plugin_meta.hpp.j2')
    source_template = get_template('sound_plugin_meta.cpp.j2')

    out_folder.parent.mkdir(parents=True, exist_ok=True)

    for plugin_id, plugin in enumerate(plugins.plugin_list):
        plugin_params = [(param_index, param) for param_index, param in enumerate(plugin.param_list())]
        meta_name = plugin.name + 'Meta'

        header = header_template.render(
            plugin_id=plugin_id, 
            meta_name=meta_name, 
            plugin=plugin, 
            params=plugin_params, 
            plugin_header=plugin.header.name,
        )
        header_name = f'{plugin.name}_meta.hpp'
        header_path = out_folder / header_name
        with open(header_path, 'w') as f:
            f.write(header)    

        source = source_template.render(
            meta_header=header_name, 
            meta_name=meta_name, 
            plugin=plugin,
        )
        source_path = out_folder / f'{plugin.name}_meta.cpp'
        with open(source_path, 'w') as f:
            f.write(source)

    __all__ = [
        'search_for_plugins', 
        'write_plugin_reflection_info', 
        'write_plugin_factory_header', 
        'write_meta_classes',
    ]
    