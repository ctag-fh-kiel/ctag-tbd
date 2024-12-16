import { useStore } from "@nanostores/preact";
import { $favorites, type FavoritePreset } from "../stores/favoritesStore";
import { useEffect, useState } from "preact/hooks";

export interface EditFavoriteProps {
  num: number;
  favoriteData?: FavoritePreset;
}

export default function EditCurrrentFavorite({
  num,
  favoriteData,
}: EditFavoriteProps) {
  const favorites = useStore($favorites);
  const currentFavorite = favoriteData ?? favorites.at(num);
  const [name, setName] = useState<string>("");
  const [ustring, setUstring] = useState<string>("");

  useEffect(() => {
    if (currentFavorite) {
      setName(currentFavorite.name);
      setUstring(currentFavorite.ustring);
    }
  }, [currentFavorite]);

  const handleSave = async () => {
    if (!currentFavorite) {
      return;
    }

    const data: FavoritePreset = {
      ...currentFavorite,
      name,
      ustring,
    };

    const response = await fetch(`/api/v1/favorites/store/${num}`, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(data),
    });

    if (response.ok) {
      favorites[num] = data;
      $favorites.set(favorites);
    }
  };

  return (
    <div>
      <label className="form-control w-full max-w-xs">
        <div className="label">
          <span className="label-text">Name</span>
        </div>
        <input
          type="text"
          className="input input-bordered w-full max-w-xs"
          value={name}
          onChange={(e) => setName(e.currentTarget.value)}
        />
      </label>

      <label className="form-control w-full max-w-xs">
        <div className="label">
          <span className="label-text">User String</span>
        </div>
        <input
          type="text"
          className="input input-bordered w-full max-w-xs"
          value={ustring}
          onChange={(e) => setUstring(e.currentTarget.value)}
        />
      </label>

      <button className="btn" onClick={handleSave}>
        Save
      </button>
    </div>
  );
}
