import { $ } from "bun";
import { readdir } from "node:fs/promises";

const MILLISECOND = 1000;

const config = {
	url: "http://ctag-tbd.local/",
	// localPath: "../spiffs_image/www",
	localPath: "../astro-frontend/dist",
	fileTypes: ["html", "js", "css", "ico"],
	compressionTypes: ["none", "gzip"], // ["none", "gzip", "brotli", "zstd"]
	runsPerPath: 1,
	loadTimeScale: MILLISECOND,
	enableFileSizeBenchmark: true,
	enableLoadTimeBenchmark: false,
	none: {
		command: "",
		fileEnding: "",
		headerType: "",
	},
	gzip: {
		command: "gzip -9 -k -f",
		fileEnding: ".gz",
		headerType: "gzip",
	},
	brotli: {
		command: "brotli -q 11 -f",
		fileEnding: ".br",
		headerType: "br",
	},
	zstd: {
		command: "zstd -19 -f",
		fileEnding: ".zst",
		headerType: "zstd",
	},
} as const;

const curlFormat = (await Bun.file("curl-format-json.txt").text()).trim();

type CurlResult = {
	namelookupTime: number;
	connectTime: number;
	appconnectTime: number;
	pretransferTime: number;
	redirectTime: number;
	starttransferTime: number;
	totalTime: number;
};

type CompressionType = (typeof config)["compressionTypes"][number];
type Sizes = Record<CompressionType, number>;
type LoadTimes = Record<
	CompressionType,
	{ min: number; avg: number; max: number }
>;

type Benchmark = {
	path: string;
	size: Sizes;
	loadTime: LoadTimes;
};

function fileSize(path: string) {
	return Bun.file(path).size;
}

async function performRequest(pathname: string, compression: CompressionType) {
	const fullUrl = config.url + pathname;
	const encodingHeader = `-H "Accept-encoding: ${config[compression].headerType}" `;

	const result =
		await $`curl -w "${curlFormat}" -o /dev/null -s -4 ${{ raw: encodingHeader }}"${fullUrl}"`
			.nothrow()
			.text();

	const json: CurlResult = JSON.parse(result);

	for (const key of Object.keys(json)) {
		const jsonKey = key as keyof CurlResult;
		json[jsonKey] *= config.loadTimeScale;
	}

	const loadTime = json.totalTime - json.pretransferTime;

	return { ...json, loadTime };
}

async function benchmarkLoadTime(
	pathname: string,
	compression: CompressionType,
) {
	const loadTimes: number[] = [];

	for (let i = 0; i < config.runsPerPath; i++) {
		console.log(
			`${pathname} - ${compression} (${i + 1}/${config.runsPerPath})`,
		);
		const result = await performRequest(pathname, compression);
		loadTimes.push(result.loadTime);
	}

	loadTimes.sort((a, b) => a - b);

	const min = loadTimes[0];
	const max = loadTimes[loadTimes.length - 1];
	const avg =
		loadTimes.reduce((prev, current) => prev + current, 0) / config.runsPerPath;

	return { min, avg, max };
}

const benchmarks: Benchmark[] = [];
const paths = await readdir(config.localPath, {
	recursive: true,
	withFileTypes: true,
});

for (const path of paths) {
	if (
		!path.isFile() ||
		config.fileTypes.every((fileType) => !path.name.endsWith(`.${fileType}`))
	) {
		continue;
	}

	let relativePath = path.name;

	if (path.parentPath !== config.localPath) {
		relativePath =
			path.parentPath.replace(config.localPath + "/", "") + "/" + path.name;
	}

	const fullPath = `${path.parentPath}/${path.name}`;
	const sizeEntries: [CompressionType, number][] = [];
	const loadTimeEntries: [CompressionType, LoadTimes[CompressionType]][] = [];

	for (const compression of config.compressionTypes) {
		const { command, fileEnding } = config[compression];
		if (command.length > 0) {
			await $`${{ raw: command }} ${fullPath}`.quiet();
		}

		if (config.enableFileSizeBenchmark) {
			sizeEntries.push([compression, fileSize(`${fullPath}${fileEnding}`)]);
		}

		if (config.enableLoadTimeBenchmark) {
			loadTimeEntries.push([
				compression,
				await benchmarkLoadTime(relativePath, compression),
			]);
		}
	}

	const benchmark: Benchmark = {
		path: relativePath,
		size: Object.fromEntries(sizeEntries) as Sizes,
		loadTime: Object.fromEntries(loadTimeEntries) as LoadTimes,
	};
	benchmarks.push(benchmark);
}

const formattedBenchmarks: any[] = [];

for (const benchmark of benchmarks) {
	for (const compression of config.compressionTypes) {
		const formattedBenchmark: any = {
			path: benchmark.path,
			compression,
		};

		if (config.enableFileSizeBenchmark) {
			formattedBenchmark.size = benchmark.size[compression];
		}

		if (config.enableLoadTimeBenchmark) {
			formattedBenchmark["load min"] = parseFloat(
				benchmark.loadTime[compression].min.toFixed(2),
			);
			formattedBenchmark["load avg"] = parseFloat(
				benchmark.loadTime[compression].avg.toFixed(2),
			);
			formattedBenchmark["load max"] = parseFloat(
				benchmark.loadTime[compression].max.toFixed(2),
			);
		}

		formattedBenchmarks.push(formattedBenchmark);
	}
}

console.table(formattedBenchmarks);
