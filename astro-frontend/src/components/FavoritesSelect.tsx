import { useStore } from "@nanostores/preact";
import { currentFavorite, favorites } from "../stores/favoritesStore";

export default function FavoritesSelect() {
  const $favorites = useStore(favorites);
  const $currentFavorite = useStore(currentFavorite);

  return (
    <select
      class="select w-full max-w-xs"
      value={$currentFavorite}
      onChange={(event) => {
        const newFavorite = Number.parseInt(event.currentTarget.value, 10);

        if (!Number.isNaN(newFavorite)) {
          currentFavorite.set(newFavorite);
        }
      }}
    >
      {$favorites.map((favorite, i) => (
        <option value={i}>
          {i}: {favorite.name}
        </option>
      ))}
    </select>
  );
}
