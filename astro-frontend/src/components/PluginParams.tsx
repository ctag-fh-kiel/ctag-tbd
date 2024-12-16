import { useEffect, useState } from "preact/hooks";
import type { Plugin } from "../stores/pluginsStore";
import useFetchQueue from "../hooks/useFetchQueue";

type ParameterUi = {
  id: string;
  name: string;
} & (
    | {
      type: "group";
      params: ParameterUi[];
    }
    | {
      type: "bool";
    }
    | {
      type: "int";
      min: number;
      max: number;
      step: number;
    }
  );

type ParameterCurrent = {
  id: string;
  current: number;
  trig?: number;
  cv?: number;
};

interface PluginWithParams extends Plugin {
  params: ParameterUi[];
}

interface PatchWithParams {
  activePatch: number;
  patches: {
    name: string;
    params: ParameterCurrent[];
  }[];
}

interface PluginParamProps {
  param: ParameterUi;
  paramsCurrent: ParameterCurrent[];
  onChange: (
    param: ParameterUi,
    value: number,
    type: "current" | "trig" | "cv",
  ) => void;
}

function PluginParam({ param, paramsCurrent, onChange }: PluginParamProps) {
  const paramCurrent = paramsCurrent.find((it) => it.id === param.id);
  const [current, setCurrent] = useState<number>(paramCurrent?.current ?? 0);

  switch (param.type) {
    case "group":
      return (
        <>
          <div class="divider" />
          <div>{param.name}</div>
          <div class="divider" />
          <div>
            {param.params.map((it) => (
              <PluginParam
                param={it}
                paramsCurrent={paramsCurrent}
                onChange={onChange}
              />
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
              checked={current !== 0}
              onChange={(event) =>
                onChange(param, event.currentTarget.checked ? 1 : 0, "current")
              }
            />
          </div>
          <div class="flex-none w-1/12">
            <select
              class="select w-full max-w-xs"
              value={paramCurrent?.trig}
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
              value={paramCurrent?.cv}
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
  const [paramsUi, setParamsUi] = useState<ParameterUi[]>();
  const [paramsCurrent, setParamsCurrent] = useState<ParameterCurrent[]>();
  const { addToQueue } = useFetchQueue();

  useEffect(() => {
    if (!fetched) {
      setFetched(true);

      fetch(`/api/v1/getPluginParamsUI/${channel}`)
        .then((r) => r.json())
        .then((plugin: PluginWithParams) => setParamsUi(plugin.params));

      fetch(`/api/v1/getPluginParamsP/${channel}`)
        .then((r) => r.json())
        .then((plugin: PatchWithParams) =>
          setParamsCurrent(plugin.patches[0]?.params),
        );
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

    addToQueue({
      url: `/api/v1/setPluginParam${routeSuffix}/${channel}?id=${changedParam.id}&${type}=${newValue}`,
    });
  };

  if (paramsUi === undefined || paramsCurrent === undefined) {
    return <></>;
  }

  return (
    <>
      {paramsUi.map((param) => (
        <PluginParam
          param={param}
          paramsCurrent={paramsCurrent}
          onChange={handleChange}
        />
      ))}
    </>
  );
}
