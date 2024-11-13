from dataclasses import dataclass
from pathlib import Path
from typing import List
from kconfiglib import Kconfig
import jinja2 as ji
import humps

from .plugin_source_parser import SoundPluginDescription


class PluginConfig:
    def __init__(self, config_file: Path):
        self._config = Kconfig(config_file)


    def get_plugins(self):
        item = self._config.menus[0].list
        while item:
            yield item
            item = item.next

@dataclass
class PluginConfig:
    name: str
    config_name: str


def write_plugin_config(plugins: List[SoundPluginDescription], config_file: Path):
    template_path = Path(__file__).parent / 'templates'
    env = ji.Environment(loader=ji.FileSystemLoader(template_path), autoescape=ji.select_autoescape())    
    
    def entry_from_plugin_name(name):
        name = humps.decamelize(name)
        return PluginConfig(
            name=' '.join(name.split('_')),
            config_name = name.upper(),
        )

    config_entries = [entry_from_plugin_name(plugin.name) for plugin in plugins]

    template = env.get_template('plugin_choice.jinja.projbuild')
    config = template.render(plugins=config_entries)
    config_file.parent.mkdir(parents=True, exist_ok=True)
    with open(config_file, 'w') as f:
        f.write(config)    


def update_plugin_config(plugins: List[SoundPluginDescription], config_file: Path):
    # config = PluginConfig(config_file)
    # for plugin in config.get_plugins():
    #     print(plugin.defaults)
    #     print(plugin.prompt)
    #     print(plugin.help)

    write_plugin_config(plugins, config_file)


__all__ = ['PluginConfig', 'update_plugin_config']



