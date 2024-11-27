import { $channelPresets, type Channel } from "../stores/pluginsStore";
import { useStore } from "@nanostores/preact";

interface LoadChannelPresetProps {
  channel: Channel;
}

export default function LoadChannelPreset({ channel }: LoadChannelPresetProps) {
  const { activePresetNumber, presets } =
    useStore($channelPresets)[channel] ?? {};

  const handleClick = (presetNumber: number) => {
    if (!presets) {
      return;
    }

    fetch(`/api/v1/loadPreset/${channel}?number=${presetNumber}`).then((r) => {
      if (r.ok) {
        $channelPresets.setKey(channel, {
          presets,
          activePresetNumber: presetNumber,
        });
      }
    });
  };

  return (
    <ul class="menu bg-base-200 rounded-box">
      {presets?.map((preset) => (
        <li
          key={preset.number}
          onClick={() => handleClick(preset.number)}
          class={preset.number === activePresetNumber ? "font-bold" : ""}
        >
          <a>
            {preset.number}: {preset.name}
          </a>
        </li>
      ))}
    </ul>
  );
}
