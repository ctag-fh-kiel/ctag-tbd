from pathlib import Path
from typing import List, Tuple
import jinja2 as ji

from .reflectables import Headers
from .plugin_parser import SoundPluginFinder, SoundPlugins


def search_for_plugins(headers: Headers, strict: bool) -> Tuple[Headers, SoundPlugins]:
    collector = SoundPluginFinder()

    for header in headers:
        if strict:
            collector.add_from_file(header)
        else:
            try:
                collector.add_from_file(header)
            except Exception as e:
                print(f'error parsing {header}: {e}')
    return collector.headers, collector.reflectables


def write_plugin_factory_header(headers: Headers, sound_plugins: SoundPlugins, factory_header_path: Path) -> None:    
    template_path = Path(__file__).parent / 'templates'
    env = ji.Environment(loader=ji.FileSystemLoader(template_path), autoescape=ji.select_autoescape())    

    template = env.get_template('sound_plugin_factory.jinja.hpp')
    header = template.render(sound_processors=sound_plugins, headers=headers)

    factory_header_path.parent.mkdir(parents=True, exist_ok=True)
    with open(factory_header_path, 'w') as f:
        f.write(header)    


def write_meta_class(headers: Headers, sound_plugins, meta_path: Path) -> None:    
    template_path = Path(__file__).parent / 'templates'
    env = ji.Environment(loader=ji.FileSystemLoader(template_path), autoescape=ji.select_autoescape())    
    meta_path.parent.mkdir(parents=True, exist_ok=True)

    template = env.get_template('sound_plugin_meta.jinja.hpp')
    header = template.render(sound_processors=sound_plugins, headers=headers)
    header_name = f'{meta_path}.hpp'
    with open(header_name, 'w') as f:
        f.write(header)    

    template = env.get_template('sound_plugin_meta.jinja.cpp')
    header_name = Path(header_name).name
    header = template.render(sound_processors=sound_plugins, headers=headers, header_name=header_name)
    with open(f'{meta_path}.cpp', 'w') as f:
        f.write(header)  


    __all__ = ['search_for_plugins', 'write_plugin_factory_header', 'write_meta_class']
    