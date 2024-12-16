import { useEffect, useState } from "preact/hooks";
import type { Channel } from "../stores/pluginsStore";

type Preset = {
  name: string;
  number: number;
};

interface GetPresetsResponse {
  activePresetNumber: number;
  presets: Preset[];
}

interface SaveChannelPresetProps {
  channel: Channel;
}

export default function SaveChannelPreset({ channel }: SaveChannelPresetProps) {
  const [fetched, setFetched] = useState(false);
  const [activePresetNumber, setActivePresetNumber] = useState<number>(-1);
  const [presets, setPresets] = useState(new Map<number, Preset>());
  const [newPreset, setNewPreset] = useState<Preset>({ number: -1, name: "" });
  const isOverwritingPreset = presets.has(newPreset.number);

  useEffect(() => {
    if (!fetched) {
      setFetched(true);

      fetch(`/api/v1/getPresets/${channel}`)
        .then((r) => r.json())
        .then((r: GetPresetsResponse) => {
          const loadedPresets = new Map<number, Preset>(
            r.presets.map((p) => [p.number, p]),
          );
          setPresets(new Map<number, Preset>(loadedPresets));
          setActivePresetNumber(r.activePresetNumber);
          setNewPreset({
            number: loadedPresets.size,
            name: "",
          });
        });
    }
  }, [fetched]);

  const handleSave = (event: SubmitEvent) => {
    event.preventDefault();

    if (newPreset === undefined) {
      return;
    }

    fetch(
      `/api/v1/savePreset/${channel}?number=${newPreset.number}&name=${newPreset.name}`,
    ).then((r) => {
      if (r.ok) {
        setActivePresetNumber(newPreset.number);
        setPresets((prev) => new Map(prev).set(newPreset.number, newPreset));
      }
    });
  };

  if (activePresetNumber === -1) {
    return <></>;
  }

  return (
    <>
      <h4>
        Current preset {activePresetNumber}:{" "}
        {presets.get(activePresetNumber)?.name}
      </h4>
      <form onSubmit={handleSave}>
        <label className="form-control w-full max-w-xs">
          <div className="label">
            <span className="label-text">Patch Number</span>
          </div>
          <input
            type="number"
            className="input input-bordered w-full max-w-xs"
            value={newPreset.number}
            onChange={(e) =>
              setNewPreset({
                number: e.currentTarget.valueAsNumber,
                name: presets.get(e.currentTarget.valueAsNumber)?.name ?? "",
              })
            }
            min={0}
            max={presets.size}
            required
          />
        </label>
        <label className="form-control w-full max-w-xs">
          <div className="label">
            <span className="label-text">Patch Name</span>
          </div>
          <input
            type="text"
            className="input input-bordered w-full max-w-xs"
            value={newPreset.name}
            onChange={(e) =>
              setNewPreset((prev) => ({
                ...prev,
                name: e.currentTarget.value,
              }))
            }
            minLength={1}
            required
          />
        </label>
        <button
          type="submit"
          className={`btn ${isOverwritingPreset ? "btn-warning" : ""}`}
        >
          {isOverwritingPreset ? "Overwrite ?!" : "Save"}
        </button>
      </form>
    </>
  );
}
