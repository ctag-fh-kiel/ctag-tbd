import { useEffect, useState } from "preact/hooks";
import type { Plugin } from "../stores/pluginsStore";

type Parameter = {
  id: string;
  name: string;
} & (
  | {
      type: "group";
      params: Parameter[];
    }
  | {
      type: "bool";
      current: 0 | 1;
      trig: number;
    }
  | {
      type: "int";
      current: number;
      min: number;
      max: number;
      step: number;
      cv: number;
    }
);

interface PluginWithParams extends Plugin {
  params: Parameter[];
}

interface PluginParamProps {
  param: Parameter;
  onChange: (
    param: Parameter,
    value: number,
    type: "current" | "trig" | "cv",
  ) => void;
}

function PluginParam({ param, onChange }: PluginParamProps) {
  const [current, setCurrent] = useState<number>(
    "current" in param ? param.current : 0,
  );

  switch (param.type) {
    case "group":
      return (
        <>
          <div class="divider" />
          <div>{param.name}</div>
          <div class="divider" />
          <div>
            {param.params.map((it) => (
              <PluginParam param={it} onChange={onChange} />
            ))}
          </div>
        </>
      );
    case "bool":
      return (
        <div class="flex h-12 items-center">
          <div class="flex-none w-5/12">{param.name}</div>
          <div class="flex-none w-6/12">
            <input
              type="checkbox"
              class="toggle"
              checked={param.current !== 0}
              onChange={(event) =>
                onChange(param, event.currentTarget.checked ? 1 : 0, "current")
              }
            />
          </div>
          <div class="flex-none w-1/12">
            <select
              class="select w-full max-w-xs"
              value={param.trig}
              onChange={(event) => {
                const newValue = Number.parseInt(event.currentTarget.value, 10);

                if (!Number.isNaN(newValue)) {
                  onChange(param, newValue, "trig");
                }
              }}
            >
              <option value={-1}>None</option>
              <option value={0}>TRIG0</option>
              <option value={1}>TRIG1</option>
            </select>
          </div>
        </div>
      );
    case "int":
      return (
        <div class="flex h-12 items-center">
          <div class="flex-none w-4/12">{param.name}</div>
          <div class="flex-none w-1/12">
            <input
              type="number"
              value={current}
              class="input input-ghost w-full max-w-xs"
              onChange={(event) => {
                const newValue = Number.parseInt(event.currentTarget.value, 10);

                if (!Number.isNaN(newValue)) {
                  setCurrent(newValue);
                  onChange(param, newValue, "current");
                }
              }}
            />
          </div>
          <div class="flex-none w-6/12">
            <input
              type="range"
              min={param.min}
              max={param.max}
              value={current}
              class="range"
              onInput={(event) => {
                const newValue = Number.parseInt(event.currentTarget.value, 10);

                if (!Number.isNaN(newValue)) {
                  setCurrent(newValue);
                  onChange(param, newValue, "current");
                }
              }}
            />
          </div>
          <div class="flex-none w-1/12">
            <select
              class="select w-full max-w-xs"
              value={param.cv}
              onChange={(event) => {
                const newValue = Number.parseInt(event.currentTarget.value, 10);

                if (!Number.isNaN(newValue)) {
                  onChange(param, newValue, "cv");
                }
              }}
            >
              <option value={-1}>None</option>
              <option value={0}>CV0</option>
              <option value={1}>CV1</option>
              <option value={2}>POT0</option>
              <option value={3}>POT1</option>
            </select>
          </div>
        </div>
      );
    default:
      return null;
  }
}

interface PluginParamsProps {
  channel: string;
}

export default function PluginParams({ channel }: PluginParamsProps) {
  const [fetched, setFetched] = useState(false);
  const [params, setParams] = useState<Parameter[]>([]);

  useEffect(() => {
    if (!fetched) {
      setFetched(true);

      fetch(`/api/v1/getPluginParams/${channel}`)
        .then((r) => r.json())
        .then((plugin: PluginWithParams) => setParams(plugin.params));
    }
  }, [fetched]);

  const handleChange: PluginParamProps["onChange"] = (
    changedParam,
    newValue,
    type,
  ) => {
    let routeSuffix = "";

    if (type === "trig") {
      routeSuffix = "TRIG";
    } else if (type === "cv") {
      routeSuffix = "CV";
    }

    fetch(
      `/api/v1/setPluginParam${routeSuffix}/${channel}?id=${changedParam.id}&${type}=${newValue}`,
    );
  };

  return (
    <>
      {params.map((param) => (
        <PluginParam param={param} onChange={handleChange} />
      ))}
    </>
  );
}
