import { useStore } from "@nanostores/preact";
import { $currentFavoriteNum, $favorites } from "../stores/favoritesStore";

export default function FavoritesSelect() {
  const favorites = useStore($favorites);
  const currentFavorite = useStore($currentFavoriteNum);

  return (
    <select
      class="select w-full max-w-xs"
      value={currentFavorite}
      onChange={(event) => {
        const newFavorite = Number.parseInt(event.currentTarget.value, 10);

        if (!Number.isNaN(newFavorite)) {
          $currentFavoriteNum.set(newFavorite);
        }
      }}
    >
      {favorites.map((favorite, i) => (
        <option value={i}>
          {i}: {favorite.name}
        </option>
      ))}
    </select>
  );
}
