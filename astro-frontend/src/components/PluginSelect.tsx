import { useStore } from "@nanostores/preact";
import { activePlugins, plugins, type Plugin } from "../stores/pluginsStore";

interface PluginSelectProps {
  channel: string;
}

export default function PluginSelect({ channel }: PluginSelectProps) {
  const $plugins = useStore(plugins);
  const $activePlugins = useStore(activePlugins);
  const activePlugin = $activePlugins[channel] ?? undefined;
  const isOtherPluginStereo = Object.entries($activePlugins).some(
    ([key, { id }]) => key !== channel && $plugins[id]?.isStereo,
  );

  return (
    <select
      class="select w-full max-w-xs"
      value={activePlugin?.id}
      onChange={(event) => {
        const newId = event.currentTarget.value;

        fetch(`/api/v1/setActivePlugin/${channel}?id=${newId}`).then((r) => {
          if (r.ok) {
            activePlugins.setKey(channel, { id: newId });
          }
        });
      }}
      disabled={isOtherPluginStereo}
    >
      {Object.values($plugins)
        .toSorted((a: Plugin, b: Plugin) => a.name.localeCompare(b.name))
        .map((plugin: Plugin) => (
          <option value={plugin.id}>
            {plugin.name} ({plugin.isStereo ? "ST" : "M"})
          </option>
        ))}
    </select>
  );
}
