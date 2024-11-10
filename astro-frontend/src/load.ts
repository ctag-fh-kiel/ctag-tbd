import { favorites } from "./stores/favoritesStore";
import { activePlugins, plugins, type Plugin } from "./stores/pluginsStore";

window.addEventListener("DOMContentLoaded", () => {
  fetch("/api/v1/getActivePlugin/0")
    .then((r) => r.json())
    .then(({ id }: Pick<Plugin, "id">) =>
      activePlugins.set({ ...activePlugins.get(), 0: { id } }),
    );

  fetch("/api/v1/getActivePlugin/1")
    .then((r) => r.json())
    .then(({ id }: Pick<Plugin, "id">) =>
      activePlugins.set({ ...activePlugins.get(), 1: { id } }),
    );

  fetch("/api/v1/getPlugins")
    .then((r) => r.json())
    .then((fetchedPlugins: Plugin[]) => {
      for (const plugin of fetchedPlugins) {
        plugins.setKey(plugin.id, plugin);
      }
    });

  fetch("/api/v1/favorites/getAll", { method: "POST" })
    .then((r) => r.json())
    .then(favorites.set);
});
