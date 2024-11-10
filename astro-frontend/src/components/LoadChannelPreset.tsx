import { useEffect, useState } from "preact/hooks";

type Preset = {
	name: string;
	number: number;
};

interface GetPresetsResponse {
	activePresetNumber: number;
	presets: Preset[];
}

interface LoadChannelPresetProps {
	channel: string;
}

export default function LoadChannelPreset({ channel }: LoadChannelPresetProps) {
	const [fetched, setFetched] = useState(false);
	const [activePresetNumber, setActivePresetNumber] = useState<number>();
	const [presets, setPresets] = useState<Preset[]>([]);

	useEffect(() => {
		if (!fetched) {
			setFetched(true);

			fetch(`/api/v1/getPresets/${channel}`)
				.then((r) => r.json())
				.then((r: GetPresetsResponse) => {
					setPresets(r.presets.toSorted((a, b) => a.number - b.number));
					setActivePresetNumber(r.activePresetNumber);
				});
		}
	}, [fetched]);

	const handleClick = (presetNumber: number) => {
		fetch(`/api/v1/loadPreset/${channel}?number=${presetNumber}`).then((r) => {
			if (r.ok) {
				setActivePresetNumber(presetNumber);
			}
		});
	};

	return (
		<ul class="menu bg-base-200 rounded-box">
			{presets.map((preset) => (
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
