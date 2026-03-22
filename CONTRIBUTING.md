# Contributing to dadamachines TBD

Thank you for helping improve the TBD platform! This guide covers how to
build, test, and submit changes.

## Repository Layout

| Directory | What's there |
|-----------|-------------|
| `main/` | ESP32-P4 firmware entry point and system management |
| `components/` | DSP plugins, drivers, audio engine |
| `sdcard_image/` | Web UI (HTML/JS/CSS) and device configuration data |
| `docs/` | Sphinx documentation (RST) + flash pages |
| `sample_rom/` | Stock audio samples (Git LFS) |
| `.github/workflows/` | CI / release pipelines |

## Branch Model

```
dada-tbd-master          ← production branch (all merges go here)
  └── staging            ← pre-release testing (auto-builds)
  └── feature-test/*     ← ad-hoc feature builds (auto-builds)
```

| Branch | Trigger | Builds | Creates |
|--------|---------|--------|---------|
| `dada-tbd-master` | push / PR | CI check | — |
| `dada-tbd-master` + `v*` tag | tag push | full build | GitHub Release + CDN deploy |
| `staging` | push | full build | GitHub pre-release + CDN staging channel |
| `feature-test/*` | push | full build | GitHub pre-release + CDN feature channel |

## Quick Start — Building Firmware

### Prerequisites

- **ESP-IDF v5.5.3** — [install guide](https://docs.espressif.com/projects/esp-idf/en/v5.5.3/esp32p4/get-started/)
- **xxhash** — `brew install xxhash` (macOS) or `apt install xxhash` (Linux)
- ESP32-P4 target (not ESP32)

### Build

```bash
git clone --recursive https://github.com/dadamachines/ctag-tbd.git
cd ctag-tbd
. ~/esp/esp-idf/export.sh   # or wherever your ESP-IDF is
idf.py build
```

The build produces:

| Artifact | Path | Description |
|----------|------|-------------|
| `dada-tbd.bin` | `build/dada-tbd.bin` | Main firmware (app partition) |
| `bootloader.bin` | `build/bootloader/bootloader.bin` | ESP32-P4 bootloader |
| `partition-table.bin` | `build/partition_table/partition-table.bin` | Partition layout |
| `ota_data_initial.bin` | `build/ota_data_initial.bin` | OTA boot selector |
| `dada-tbd-sd.zip` | `build/dada-tbd-sd.zip` | SD card archive (WebUI + samples + data) |
| `dada-tbd-sd-hash.txt` | `build/dada-tbd-sd-hash.txt` | SD archive integrity hash |

### Flash to Device

> **Never use `idf.py flash`** — it writes OTA data that causes reboot loops.

Use the provided script:

```bash
bin/flash.sh
```

Or flash manually with esptool:

```bash
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1)
esptool.py --chip esp32p4 -p "$PORT" -b 460800 \
  --before=default_reset --after=hard_reset \
  write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \
  0x2000  build/bootloader/bootloader.bin \
  0x10000 build/dada-tbd.bin \
  0x8000  build/partition_table/partition-table.bin \
  0xd000  build/ota_data_initial.bin
```

### Build WebUI

```bash
cd sdcard_image/www && bash build-webui.sh && cd ../..
```

## CI Pipelines

All pipelines are fully automated. Engineers push code; CI builds, tests,
releases, and deploys firmware to the CDN.

### CI Check (`ci.yml`)

Runs on every push to `dada-tbd-master` and on pull requests. Builds the
full firmware in Docker (`espressif/idf:v5.5.3`) to catch regressions.
Verifies ESP-IDF patches are applied and logs checksums.

### Stable Release (`create-release.yml`)

Triggered by pushing a version tag (`v*`):

```bash
git tag v0.5.0
git push origin v0.5.0
```

**Pipeline:** build firmware → create GitHub Release with all artifacts →
dispatch to CDN repo → CDN writes `stable/latest.json` manifest and deploys
to GitHub Pages.

**Result:** Firmware available at
`https://dadamachines.github.io/dada-tbd-firmware/stable/` and on the
[Stable Channel flash page](https://dadamachines.github.io/ctag-tbd/flash/10_stable_channel.html).

### Staging Release (`staging-release.yml`)

Triggered on every push to the `staging` branch:

```bash
git checkout staging
git merge dada-tbd-master
git push origin staging
```

**Pipeline:** build firmware → create GitHub pre-release → dispatch to CDN
staging channel.

**Result:** Firmware appears in the
[Beta Channel flash page](https://dadamachines.github.io/ctag-tbd/flash/20_staging_channel.html)
under "Staging".

### Feature Test Release (`feature-test-release.yml`)

For ad-hoc testing of feature branches. Push to any `feature-test/*` branch:

```bash
git checkout -b feature-test/my-cool-feature
# ... make changes ...
git push origin feature-test/my-cool-feature
```

**Pipeline:** build firmware → create GitHub pre-release → dispatch to CDN
with a per-feature channel (`feature-test-my-cool-feature`).

**Result:** Channel appears in the Beta Channel flash page dropdown as
"Feature: my-cool-feature". Engineers can flash directly from the browser.

## Firmware CDN (`dada-tbd-firmware`)

All release and pre-release firmware is served from
[`dadamachines.github.io/dada-tbd-firmware/`](https://dadamachines.github.io/dada-tbd-firmware/).

### Channel Structure

```
stable/
  ├── p4/               ← P4 firmware binaries
  ├── pico/             ← RP2350 Groovebox UF2
  └── latest.json       ← channel manifest
staging/
  ├── p4/
  ├── pico/
  └── latest.json
feature-test-<name>/
  ├── p4/
  ├── pico/
  └── latest.json
```

### Manifest Format (`latest.json`)

```json
{
  "tag": "v0.5.0",
  "channel": "stable",
  "timestamp": "2026-03-22T10:00:00Z",
  "files": {
    "unified": "stable/p4/dada-tbd-16-v0.5.0-unified.bin",
    "p4": "stable/p4/dada-tbd.bin",
    "bootloader": "stable/p4/bootloader.bin",
    "partition_table": "stable/p4/partition-table.bin",
    "ota_data": "stable/p4/ota_data_initial.bin",
    "sdcard": "stable/p4/dada-tbd-sd.zip",
    "hash": "stable/p4/dada-tbd-sd-hash.txt",
    "tusb_msc": "stable/p4/tusb_msc.bin",
    "pico": "stable/pico/dada-tbd-16-v0.5.0-pico.uf2"
  }
}
```

## Artifact Naming Convention

All public-facing artifacts use the **dadamachines** product name, not the
upstream `ctag-tbd` project name.

### Build Outputs

| Artifact | Name | Notes |
|----------|------|-------|
| App binary | `dada-tbd.bin` | ESP-IDF output (was `ctag-tbd.bin`) |
| Unified image | `dada-tbd-16-{tag}-unified.bin` | All partitions merged, flash at `0x0` |
| SD archive | `dada-tbd-sd.zip` | WebUI + samples + data |
| SD hash | `dada-tbd-sd-hash.txt` | XXH128 integrity hash |
| Pico firmware | `dada-tbd-16-{tag}-pico.uf2` | RP2350 Groovebox |

### Why `dada-tbd` and not `ctag-tbd`?

This repository is a fork of `ctag-fh-kiel/ctag-tbd` (upstream). The
upstream project targets different hardware. Our artifacts represent
dadamachines hardware products (TBD-16, TBD-Core) and should be clearly
identifiable as dadamachines firmware — not generic `ctag-tbd` builds for
unknown hardware.

## Submitting Changes

1. **Open an issue** describing the bug or feature.
2. **Fork** the repo and create a branch from `dada-tbd-master`.
3. **Make your changes** — keep commits focused and descriptive.
4. **Test locally** — `idf.py build` must succeed; flash and verify on device
   if hardware-related.
5. **Open a pull request** against `dada-tbd-master`.
6. CI will build your PR automatically. Fix any failures before requesting
   review.

### For Plugin Developers

See the [plugin documentation](https://dadamachines.github.io/ctag-tbd/plugins/)
and the generator templates in `generators/`.

### Code Style

- C++20
- Follow existing patterns in the file you're editing
- No `idf.py flash` in any script or documentation

## After a History Rewrite (Phase 3b)

After the git history is rewritten (to remove old binary blobs), all
contributors must re-clone:

```bash
# Back up any local branches first
git clone --recursive https://github.com/dadamachines/ctag-tbd.git
cd ctag-tbd
```

Your local branches can be cherry-picked onto the new history. The rewrite
only removes old binary blobs from git history — no source code is changed.

## License

This project is licensed under the terms in [LICENSE](LICENSE). Core platform
code is GPLv3 (inherited from upstream CTAG TBD).
