import { map, onMount, task } from "nanostores";

const channels = ["0", "1"] as const;

export type Channel = (typeof channels)[number];

export interface Plugin {
  id: string;
  name: string;
  isStereo: boolean;
  hint: string;
}

export interface Preset {
  name: string;
  number: number;
}

export interface ChannelPreset {
  activePresetNumber: number;
  presets: Preset[];
}

export type PluginsMap = Record<string, Plugin>;

type ActivePlugins = Record<Channel, Pick<Plugin, "id"> | undefined>;

type ChannelPresets = Record<Channel, ChannelPreset | undefined>;

export const $plugins = map<PluginsMap>({});

onMount($plugins, () => {
  task(async () => {
    if (!globalThis.window) {
      return;
    }

    const r = await fetch("/api/v1/getPlugins");
    const fetchedPlugins: Plugin[] = await r.json();

    for (const plugin of fetchedPlugins) {
      $plugins.setKey(plugin.id, plugin);
    }
  });
});

export const $activePlugins = map<ActivePlugins>({
  0: undefined,
  1: undefined,
});

onMount($activePlugins, () => {
  task(async () => {
    if (!globalThis.window) {
      return;
    }

    await Promise.all(
      channels.map((ch) =>
        fetch(`/api/v1/getActivePlugin/${ch}`)
          .then((r) => r.json())
          .then(({ id }: Pick<Plugin, "id">) =>
            $activePlugins.setKey(ch, { id }),
          ),
      ),
    );
  });
});

export const $channelPresets = map<ChannelPresets>({
  0: undefined,
  1: undefined,
});

onMount($channelPresets, () => {
  task(async () => {
    if (!globalThis.window) {
      return;
    }

    await Promise.all(
      channels.map((ch) =>
        fetch(`/api/v1/getPresets/${ch}`)
          .then((r) => r.json())
          .then((pr: ChannelPreset) => $channelPresets.setKey(ch, pr)),
      ),
    );
  });
});
