import { atom, onMount, task } from "nanostores";

export interface FavoritePreset {
  name: string;
  plug_0: string;
  pre_0: number;
  plug_1: string;
  pre_1: number;
  ustring: string;
}

type Favorites = FavoritePreset[];

export const $favorites = atom<Favorites>([]);
export const $currentFavoriteNum = atom<number>(0);

onMount($favorites, () => {
  task(async () => {
    if (!globalThis.window) {
      return;
    }

    const r = await fetch("/api/v1/favorites/getAll", { method: "POST" });
    const json = await r.json();
    $favorites.set(json);
  });
});
