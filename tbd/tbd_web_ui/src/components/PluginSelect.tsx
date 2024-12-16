import { useStore } from "@nanostores/preact";
import {
  $activePlugins,
  $plugins,
  type Channel,
  type Plugin,
} from "../stores/pluginsStore";

interface PluginSelectProps {
  channel: Channel;
}

export default function PluginSelect({ channel }: PluginSelectProps) {
  const plugins = useStore($plugins);
  const activePlugins = useStore($activePlugins);
  const activePlugin = activePlugins[channel] ?? undefined;
  const isOtherPluginStereo = Object.entries(activePlugins).some(
    ([key, p]) => p !== undefined && key !== channel && plugins[p.id]?.isStereo,
  );

  return (
    <>
      <select
        class="select w-full max-w-xs"
        value={activePlugin?.id}
        onChange={(event) => {
          const newId = event.currentTarget.value;

          fetch(`/api/v1/setActivePlugin/${channel}?id=${newId}`).then((r) => {
            if (r.ok) {
              $activePlugins.setKey(channel, { id: newId });
            }
          });

          if (
            channel === "0" &&
            !plugins[newId]?.isStereo &&
            activePlugins["1"]?.id !== undefined
          ) {
            fetch(`/api/v1/setActivePlugin/1?id=${activePlugins["1"].id}`).then(
              (r) => {
                if (r.ok) {
                  $activePlugins.setKey(channel, { id: newId });
                }
              },
            );
          }
        }}
        disabled={isOtherPluginStereo}
      >
        {Object.values(plugins)
          .filter((plugin) => (channel === "1" ? !plugin.isStereo : true))
          .toSorted((a: Plugin, b: Plugin) =>
            a.name.localeCompare(b.name, undefined, { numeric: true }),
          )
          .map((plugin: Plugin) => (
            <option value={plugin.id}>
              {plugin.name} ({plugin.isStereo ? "ST" : "M"})
            </option>
          ))}
      </select>
      <div>
        <a
          role="button"
          class={`btn ${isOtherPluginStereo ? "btn-disabled" : ""}`}
          href={`/${channel}/edit/`}
        >
          Edit channel 0
        </a>
        <a
          role="button"
          class={`btn ${isOtherPluginStereo ? "btn-disabled" : ""}`}
          href={`/${channel}/load/`}
        >
          Load preset
        </a>
        <a
          role="button"
          class={`btn ${isOtherPluginStereo ? "btn-disabled" : ""}`}
          href={`/${channel}/save/`}
        >
          Save preset
        </a>
      </div>
    </>
  );
}
