import { useStore } from "@nanostores/preact";
import { $currentFavoriteNum } from "../stores/favoritesStore";
import { $activePlugins, type Plugin } from "../stores/pluginsStore";

export default function FavoritesButtons() {
  const currentFavorite = useStore($currentFavoriteNum);

  const handleRecall = async () => {
    const response = await fetch(
      `/api/v1/favorites/recall/${currentFavorite}`,
      { method: "POST" },
    );

    if (!response.ok) {
      return;
    }

    const channelOneFetch = fetch("/api/v1/getActivePlugin/0")
      .then((r) => r.json())
      .then(({ id }: Pick<Plugin, "id">) => $activePlugins.setKey("0", { id }));
    const channelTwoFetch = fetch("/api/v1/getActivePlugin/1")
      .then((r) => r.json())
      .then(({ id }: Pick<Plugin, "id">) => $activePlugins.setKey("1", { id }));

    await Promise.all([channelOneFetch, channelTwoFetch]);
  };

  return (
    <div>
      <button className="btn" onClick={handleRecall}>
        Recall
      </button>
      <a role="button" className="btn" href={`/fav/${currentFavorite}/snap`}>
        Snap
      </a>
      <a role="button" className="btn" href={`/fav/${currentFavorite}/edit`}>
        Edit
      </a>
      <button className="btn" onClick={handleRecall}>
        Export
      </button>
      <button className="btn" onClick={handleRecall}>
        Import
      </button>
    </div>
  );
}
