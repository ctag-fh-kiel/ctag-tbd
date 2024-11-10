import { atom } from "nanostores";

export interface FavoritePreset {
  name: string;
  plug_0: string;
  pre_0: number;
  plug_1: string;
  pre_1: number;
  ustring: string;
}

type Favorites = FavoritePreset[];

export const favorites = atom<Favorites>([]);
export const currentFavorite = atom<number>(0);
