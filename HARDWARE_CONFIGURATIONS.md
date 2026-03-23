# Hardware Configurations

The TBD firmware supports four hardware configurations, controlled by two
Kconfig flags. Each configuration compiles a different subset of the firmware
to match the capabilities of the target board.

## Configurations at a Glance

| Config | Name | SD Card | RP2350 | Use Case |
|--------|------|---------|--------|----------|
| **A** | TBD Simple | — | — | Minimal board: flash-only storage, USB-MIDI, no sequencer |
| **B** | TBD + RP2350 | — | yes | RP2350 bridge for sequencer/OLED/MIDI, flash-only storage |
| **C** | TBD + SD | yes | — | SD card storage, USB-MIDI, no sequencer |
| **D** | **TBD-16** (default) | yes | yes | Full platform: SD card, RP2350 bridge, OTA, macro system |

Config D (TBD-16) is the default and the configuration used for all releases.

## Kconfig Flags

Both flags live in `main/Kconfig.projbuild` under **CTAG TBD Configuration → TBD Hardware Configuration**.

| Flag | Default | Effect when disabled |
|------|---------|---------------------|
| `CONFIG_TBD_USE_SD_CARD` | `y` | Storage falls back to LittleFS flash. The macro system, PicoSeqRack plugin, and SD-dependent REST/SPI endpoints are excluded at compile time. Partition table switches to `partitions_no_sd.csv`. |
| `CONFIG_TBD_USE_RP2350` | `y` | SPI bridge to the RP2350 (sequencer, OLED, MIDI routing) is disabled. MIDI is received via ESP32-P4 native USB-MIDI (TinyUSB). Frees 10 GPIO pins on SPI2/SPI3 buses. |

## Feature Matrix

| Feature | A | B | C | D |
|---------|---|---|---|---|
| 50+ DSP plugins | yes | yes | yes | yes |
| Web UI (presets, parameters) | yes | yes | yes | yes |
| Sample ROM (flash) | yes | yes | yes | yes |
| USB-MIDI (TinyUSB) | yes | — | yes | — |
| RP2350 SPI bridge | — | yes | — | yes |
| Hardware sequencer + OLED | — | yes | — | yes |
| MIDI via RP2350 | — | yes | — | yes |
| SD card storage | — | — | yes | yes |
| Macro system | — | — | yes | yes |
| PicoSeqRack plugin | — | — | yes | yes |
| OTA updates | — | — | yes | yes |
| Sample drag-and-drop (SD) | — | — | yes | yes |

## Partition Tables

### Config D / Config C — SD card (`partitions_example.csv`)

| Name | Type | SubType | Offset | Size |
|------|------|---------|--------|------|
| nvs | data | nvs | 0x9000 | 16 KB |
| otadata | data | ota | 0xd000 | 8 KB |
| phy_init | data | phy | 0xf000 | 4 KB |
| ota_0 | app | ota_0 | 0x10000 | 5 MB |
| ota_1 | app | ota_1 | — | 1 MB |

### Config A / Config B — Flash-only (`partitions_no_sd.csv`)

| Name | Type | SubType | Offset | Size |
|------|------|---------|--------|------|
| nvs | data | nvs | 0x9000 | 16 KB |
| otadata | data | ota | 0xd000 | 8 KB |
| phy_init | data | phy | 0xf000 | 4 KB |
| ota_0 | app | ota_0 | 0x10000 | 3.125 MB |
| storage | data | spiffs | 0x330000 | 2 MB |
| sample_rom | data | 0x40 | 0x530000 | 5 MB |

## Building a Specific Configuration

### Config D — TBD-16 (default)

No extra flags needed — both Kconfig defaults are `y`:

```bash
idf.py build
```

### Config A — TBD Simple

```bash
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.tbd-simple" build
```

### Config B — TBD + RP2350

```bash
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.tbd-rp2350-only" build
```

### Config C — TBD + SD

```bash
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.tbd-sd-only" build
```

> **Note:** When switching between configurations, run `idf.py fullclean` first to
> ensure the build directory is reset. The Kconfig cache and partition table
> are not automatically regenerated otherwise.

## sdkconfig Overlay Files

Each alternate configuration has a small overlay file that is merged on top of
`sdkconfig.defaults`. Only the flags that differ from the default TBD-16
config are set:

| File | CONFIG_TBD_USE_SD_CARD | CONFIG_TBD_USE_RP2350 | Partition Table |
|------|------------------------|-----------------------|-----------------|
| *(none — base defaults)* | y | y | `partitions_example.csv` |
| `sdkconfig.defaults.tbd-simple` | n | n | `partitions_no_sd.csv` |
| `sdkconfig.defaults.tbd-rp2350-only` | n | y | `partitions_no_sd.csv` |
| `sdkconfig.defaults.tbd-sd-only` | y | n | `partitions_example.csv` |

## CI

The CI pipeline (`ci.yml`) verifies that **all four configurations** compile
on every pull request and push to `dada-tbd-master`. The full TBD-16 build
(with SD archive and unified image) runs as the primary job; the three
alternate configs run as lightweight compilation checks in a matrix.

Release workflows (`create-release.yml`, `staging-release.yml`,
`feature-test-release.yml`) always build Config D (TBD-16).

## Architecture Notes

### SPManager Split

To avoid heavy `#if` guards throughout `SPManager.cpp`, the macro system
implementation is factored out into `MacroSPManager.cpp`. This file contains
all macro-related method bodies, static member definitions, and the
`InitMacroSystem()` helper. CMake's `Macro.*` source filter automatically
excludes it when `CONFIG_TBD_USE_SD_CARD` is disabled.

### Conditional CMake Sources

Component `CMakeLists.txt` files use Kconfig flags to include/exclude source
files rather than relying solely on preprocessor guards:

- **`main/CMakeLists.txt`** — Filters out `Macro*`, `SynthDefinition*`,
  `TrackDefinition*` when SD is disabled
- **`components/ctagSoundProcessor/CMakeLists.txt`** — Excludes PicoSeqRack
  and the `rack/` subdirectory when SD is disabled
- **`components/drivers/CMakeLists.txt`** — Excludes SD/SDMMC sources when SD
  is disabled; excludes RP2350 SPI sources when RP2350 is disabled

### Storage Abstraction

`components/drivers/fs.cpp` mounts either the SD card (via SDMMC) or a
LittleFS flash partition, depending on `CONFIG_TBD_USE_SD_CARD`. The storage
root path (`/sdcard` or `/littlefs`) is available as
`CTAG::RESOURCES::sdcardRoot` for runtime use.
