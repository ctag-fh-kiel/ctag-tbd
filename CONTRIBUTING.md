# Contributing to dadamachines TBD

Thank you for helping improve the TBD platform! This guide covers how to
set up your fork, build firmware, test your changes with CI, and submit
pull requests.

---

## Repository Layout

| Directory | What's there |
|-----------|-------------|
| `main/` | ESP32-P4 firmware entry point and system management |
| `components/` | DSP plugins, drivers, audio engine |
| `sdcard_image/` | Web UI (HTML/JS/CSS) and device configuration data |
| `docs/` | Sphinx documentation (RST) + flash pages |
| `sample_rom/` | Stock audio samples (Git LFS) |
| `.github/workflows/` | CI / release pipelines |

---

## Branch Model

```
dada-tbd-master          ← production branch
│                           docs deploy on every push
│                           CI build check on push (firmware-relevant files only)
│                           firmware release only on v* tag push (opt-in)
│
├── staging              ← pre-release testing
│                           full build + pre-release on every push
│                           firmware auto-deploys to CDN staging channel
│
└── feature-test/*       ← ad-hoc feature builds (ephemeral)
                            full build + pre-release on every push
                            firmware auto-deploys to per-feature CDN channel
```

### What triggers what

| Event | Workflow | What happens |
|-------|----------|-------------|
| Push to `dada-tbd-master` | `deploy-docs.yml` | Docs rebuild + deploy to GitHub Pages (every push) |
| Push to `dada-tbd-master` (firmware files changed) | `ci.yml` | Build check only — validates firmware compiles. **No release.** |
| PR against `dada-tbd-master` (firmware files changed) | `ci.yml` | Build check on the PR — same as above |
| Push `v*` tag on `dada-tbd-master` | `create-release.yml` | Full build → GitHub Release → CDN stable channel |
| Push to `staging` | `staging-release.yml` | Full build → GitHub pre-release → CDN staging channel |
| Push to `feature-test/*` | `feature-test-release.yml` | Full build → GitHub pre-release → CDN per-feature channel |

**Key distinction:** Pushing code to `dada-tbd-master` deploys docs and
validates the build, but it does **not** create a firmware release. Releases
happen only when a maintainer pushes a `v*` tag. This lets the team land
docs changes, code improvements, and bug fixes incrementally without
triggering firmware releases on every commit.

---

## Getting Started — Fork Setup

### If you already have a fork (e.g. `possan/ctag-tbd`)

```bash
cd ctag-tbd

# Make sure your remote tracks the dadamachines repo
git remote add upstream https://github.com/dadamachines/ctag-tbd.git
# (skip if you already have an 'upstream' remote)

# Fetch and sync with the production branch
git fetch upstream
git checkout -b dada-tbd-master upstream/dada-tbd-master
# or if you already have the branch:
git checkout dada-tbd-master
git pull upstream dada-tbd-master
```

### Fresh clone

```bash
git clone --recursive https://github.com/dadamachines/ctag-tbd.git
cd ctag-tbd
```

If you plan to submit PRs, fork the repo on GitHub first, then:

```bash
git clone --recursive https://github.com/YOUR_USERNAME/ctag-tbd.git
cd ctag-tbd
git remote add upstream https://github.com/dadamachines/ctag-tbd.git
```

---

## Building Firmware

### Prerequisites

- **ESP-IDF v5.5.3** — [install guide](https://docs.espressif.com/projects/esp-idf/en/v5.5.3/esp32p4/get-started/)
- **xxhash** — `brew install xxhash` (macOS) or `apt install xxhash` (Linux)
- ESP32-P4 target (not ESP32)

### Build

```bash
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

---

## Submitting Changes (Pull Request Workflow)

This is the standard workflow for all contributors:

```
1. Fork dadamachines/ctag-tbd on GitHub (if not already done)
2. Create a feature branch from dada-tbd-master
3. Make changes, commit, push to your fork
4. Open a PR against dadamachines/ctag-tbd → dada-tbd-master
5. CI builds your PR automatically — fix any failures
6. Maintainer reviews and merges
```

### Step by step

```bash
# Sync your fork with upstream
git fetch upstream
git checkout dada-tbd-master
git pull upstream dada-tbd-master

# Create your feature branch
git checkout -b feature/my-improvement

# ... make changes ...
git add -A && git commit -m "feat: describe what you changed"

# Push to YOUR fork (not upstream)
git push origin feature/my-improvement
```

Then open a PR on GitHub: `your-fork:feature/my-improvement` → `dadamachines/ctag-tbd:dada-tbd-master`.

CI will automatically build and validate your PR. You'll see the result
as a check on the PR page.

---

## Testing Firmware Before Merging — Staging & Feature-Test Branches

The staging and feature-test branches give you CI-built firmware that you
can flash directly from the browser — no local build environment needed.
These branches live on the **upstream repo** (`dadamachines/ctag-tbd`),
not on your fork.

### Option A: Staging Branch (pre-release testing)

Use this when changes are merged to `dada-tbd-master` and you want to
build a testable firmware before tagging a release.

```bash
# As a maintainer with push access:
git checkout staging
git merge dada-tbd-master
git push origin staging
```

This triggers a full CI build. The resulting firmware is:
- Published as a GitHub **pre-release**
- Deployed to the CDN staging channel
- Flashable from the [Beta Channel page](https://dadamachines.github.io/ctag-tbd/flash/20_staging_channel.html) (select "Staging" in the dropdown)

### Option B: Feature-Test Branch (per-feature testing)

Use this to get a CI-built firmware for a specific feature branch **before
merging it to dada-tbd-master**. This is useful for hardware testing or
sharing a build with other team members.

```bash
# Push your feature branch with the feature-test/ prefix:
git checkout -b feature-test/my-cool-feature
# ... make changes, or cherry-pick from your fork's branch ...
git push origin feature-test/my-cool-feature
```

This triggers a full CI build. The resulting firmware is:
- Published as a GitHub **pre-release** named after the branch
- Deployed to a per-feature CDN channel (`feature-test-my-cool-feature`)
- Flashable from the [Beta Channel page](https://dadamachines.github.io/ctag-tbd/flash/20_staging_channel.html) (select "Feature: my-cool-feature" in the dropdown)

> **Note:** You need push access to `dadamachines/ctag-tbd` to create
> feature-test branches. If you don't have push access, open a PR and ask
> a maintainer to create the feature-test branch for you, or request
> collaborator access.

### Reference example

The branch `feature-test/test-pipeline` is kept as a working reference
of this workflow. It was used to validate the entire CI → CDN → flash page
pipeline end-to-end.

---

## Typical Contributor Workflows

### "I want to fix a bug and submit a PR"

```bash
git fetch upstream && git checkout -b fix/my-bugfix upstream/dada-tbd-master
# fix the bug...
git push origin fix/my-bugfix
# open PR → dada-tbd-master
```

CI checks the build. Maintainer merges. Done.

### "I want to test my changes on real hardware via browser flash"

```bash
# After your PR is merged to dada-tbd-master:
git fetch upstream
git checkout staging
git merge upstream/dada-tbd-master
git push upstream staging
# → CI builds → flash from Beta Channel page → test on device
```

Or, for pre-merge testing with push access:

```bash
git checkout -b feature-test/my-feature upstream/dada-tbd-master
git cherry-pick <your commits>
git push upstream feature-test/my-feature
# → CI builds → flash from Beta Channel page (feature dropdown)
```

### "I want to trigger a stable release"

Only maintainers do this:

```bash
git checkout dada-tbd-master
git tag v0.5.0
git push origin v0.5.0
# → CI builds → GitHub Release → CDN stable channel → Stable Channel flash page
```

---

## CI Pipelines — Detail

### CI Check (`ci.yml`)

Runs on every push to `dada-tbd-master` and on pull requests — but **only
when firmware-relevant files change** (source code, CMake, sdkconfig,
patches, sdcard_image, sample_rom, workflows). Docs-only commits do **not**
trigger a firmware build.

Builds the full firmware in Docker (`espressif/idf:v5.5.3`). Verifies
ESP-IDF patches are applied and logs checksums. **Does not create a release.**

### Docs Deploy (`deploy-docs.yml`)

Runs on **every push** to `dada-tbd-master` (no path filter). Builds and
deploys documentation to GitHub Pages. This is why docs updates are
immediately visible without a firmware release.

### Stable Release (`create-release.yml`)

Triggered **only** by pushing a `v*` tag (or manual dispatch):

```bash
git tag v0.5.0 && git push origin v0.5.0
```

Pipeline: build firmware → create GitHub Release with all artifacts →
dispatch to CDN repo → CDN writes `stable/latest.json` and deploys to
GitHub Pages.

### Staging Release (`staging-release.yml`)

Triggered on every push to the `staging` branch. Pipeline: build firmware →
create GitHub pre-release → dispatch to CDN staging channel.

### Feature Test Release (`feature-test-release.yml`)

Triggered on every push to any `feature-test/*` branch. Pipeline: build
firmware → create GitHub pre-release → dispatch to CDN with a per-feature
channel.

---

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

---

## Artifact Naming Convention

All public-facing artifacts use the **dadamachines** product name, not the
upstream `ctag-tbd` project name.

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

---

## For Plugin Developers

See the [plugin documentation](https://dadamachines.github.io/ctag-tbd/plugins/)
and the generator templates in `generators/`.

## Code Style

- C++20
- Follow existing patterns in the file you're editing
- No `idf.py flash` in any script or documentation

---

## After a History Rewrite (Phase 3b)

After the git history is rewritten (to remove old binary blobs), all
contributors must re-clone:

```bash
# Back up any local branches first
git clone --recursive https://github.com/dadamachines/ctag-tbd.git
cd ctag-tbd
```

If you had a fork, update it:

```bash
# In your existing fork clone:
git remote add upstream https://github.com/dadamachines/ctag-tbd.git
git fetch upstream
git checkout dada-tbd-master
git reset --hard upstream/dada-tbd-master
git push origin dada-tbd-master --force
```

Your local feature branches can be cherry-picked onto the new history.
The rewrite only removes old binary blobs from git history — no source
code is changed.

---

## License

This project is licensed under the terms in [LICENSE](LICENSE). Core platform
code is GPLv3 (inherited from upstream CTAG TBD).
