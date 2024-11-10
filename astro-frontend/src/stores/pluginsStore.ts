import { map } from "nanostores";

export interface Plugin {
  id: string;
  name: string;
  isStereo: boolean;
  hint: string;
}

export type PluginsMap = Record<string, Plugin>;

type ActivePlugins = Record<string, Pick<Plugin, "id">>;

export const plugins = map<PluginsMap>({});
export const activePlugins = map<ActivePlugins>({});
