from pathlib import Path

from .plugin_entry import ParamEntry
from .plugins import Plugins

import tbd_core.buildgen as tbd
from tbd_core.generators import jilter, GeneratorBase
from ..reflection import Headers


class PluginFilters:
    def __init__(self, plugins: Plugins):
        self._plugins = plugins

    @jilter
    def setter(self, param: ParamEntry):
        return f'set_{param.snake_name}'


class PluginGenerator(GeneratorBase):
    def __init__(self, plugins: Plugins):
        super().__init__(tbd.get_tbd_components_root() / 'core' / 'tbd_sound_registry' / 'src', PluginFilters(plugins))

    def write_plugin_reflection_info(self, plugins: Plugins, out_folder: Path):
        source = self.render('all_sound_processors.cpp.j2',
                             plugins=plugins.plugin_list, params=plugins.param_list)

        out_folder.mkdir(exist_ok=True, parents=True)
        out_file = out_folder / 'plugin_info.cpp'
        with open(out_file, 'w') as f:
            f.write(source)

    def write_plugin_factory_header(self, headers: Headers, plugins: Plugins, out_folder: Path) -> None:
        source = self.render('factory.cpp.j2', plugins=plugins.plugin_list, headers=headers)

        out_folder.parent.mkdir(parents=True, exist_ok=True)
        with open(out_folder / 'factory.cpp', 'w') as f:
            f.write(source)

    def write_meta_classes(self, plugins: Plugins, out_folder: Path) -> None:
        out_folder.parent.mkdir(parents=True, exist_ok=True)

        for plugin_id, plugin in enumerate(plugins.plugin_list):
            plugin_params = [(param_index, param) for param_index, param in enumerate(plugin.param_list())]
            meta_name = plugin.name + 'Meta'

            header = self.render(
                'sound_plugin_meta.hpp.j2',
                plugin_id=plugin_id,
                meta_name=meta_name,
                plugin=plugin,
                params=plugin_params,
                plugin_header=plugin.header,
            )
            header_name = f'{plugin.name}_meta.hpp'
            header_path = out_folder / header_name
            with open(header_path, 'w') as f:
                f.write(header)

            source = self.render(
                'sound_plugin_meta.cpp.j2',
                meta_header=header_name,
                meta_name=meta_name,
                plugin=plugin,
            )
            source_path = out_folder / f'{plugin.name}_meta.cpp'
            with open(source_path, 'w') as f:
                f.write(source)


__all__ = ['PluginGenerator']