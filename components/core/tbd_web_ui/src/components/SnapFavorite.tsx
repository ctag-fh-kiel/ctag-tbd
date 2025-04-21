import EditFavorite from "./EditFavorite";
import {
  $activePlugins,
  $channelPresets,
  $plugins,
} from "../stores/pluginsStore";
import { useStore } from "@nanostores/preact";
import { $favorites, type FavoritePreset } from "../stores/favoritesStore";

interface SnapFavoriteProps {
  num: number;
}

export default function SnapFavorite({ num }: SnapFavoriteProps) {
  const favorites = useStore($favorites);
  const plugins = useStore($plugins);
  const activePlugins = useStore($activePlugins);
  const currentFavorite = favorites.at(num);
  const channelPresets = useStore($channelPresets);

  const pre_0 = channelPresets["0"]?.activePresetNumber;
  const pre_1 = channelPresets["1"]?.activePresetNumber;
  const plug_0 = plugins[activePlugins["0"]?.id ?? ""];
  const plug_1 = plugins[activePlugins["1"]?.id ?? ""];

  if (
    !currentFavorite ||
    pre_0 === undefined ||
    pre_1 === undefined ||
    !plug_0 ||
    !plug_1
  ) {
    return <EditFavorite num={num} />;
  }

  const favoriteData: FavoritePreset = {
    ...currentFavorite,
    pre_0,
    pre_1,
    plug_0: plug_0.id,
    plug_1: plug_0.isStereo ? plug_1.id : "",
  };

  return <EditFavorite num={num} favoriteData={favoriteData} />;
}
