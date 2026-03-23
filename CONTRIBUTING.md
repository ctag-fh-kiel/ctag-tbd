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
| `dada-tbd-unified.bin` | `build/dada-tbd-unified.bin` | Merged binary for address 0x0 flash |
| `webui-update-v*.zip` | `build/webui-update-v*.zip` | WebUI update package (built by CI) |

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

Builds the full firmware in Docker (`espressif/idf:v5.5.3`). Also builds
WebUI bundles, extracts the WebUI version from `webui-version.json`, and
creates a `webui-update-v*.zip` package. Verifies ESP-IDF patches are
applied and logs checksums. Outputs `webui_version` for downstream
workflows. **Does not create a release.**

### Docs Deploy (`deploy-docs.yml`)

Runs on **every push** to `dada-tbd-master` (no path filter). Builds and
deploys documentation to GitHub Pages. This is why docs updates are
immediately visible without a firmware release.

### Stable Release (`create-release.yml`)

Triggered **only** by pushing a `v*` tag (or manual dispatch):

```bash
git tag v0.5.0 && git push origin v0.5.0
```

Pipeline: build firmware + WebUI bundle → create GitHub Release with all
artifacts → push to CDN repo (including `webui_version`) → CDN updates
`stable/releases.json` (with `webuiVersion`/`webuiUpdate` fields) and
deploys to GitHub Pages.

### Staging Release (`staging-release.yml`)

Triggered on every push to the `staging` branch. Pipeline: build firmware +
WebUI bundle → create GitHub pre-release → push to CDN staging channel
(including `webui_version`).

### Feature Test Release (`feature-test-release.yml`)

Triggered on every push to any `feature-test/*` branch. Pipeline: build
firmware + WebUI bundle → create GitHub pre-release → push to CDN with a
per-feature channel (including `webui_version`).

---

## Firmware CDN (`dada-tbd-firmware`)

All release and pre-release firmware is served from
[`dadamachines.github.io/dada-tbd-firmware/`](https://dadamachines.github.io/dada-tbd-firmware/).

### Channel Structure

```
apps/                                ← RP2350 app registry (catalog-driven)
  ├── bootloader/                    ← BOOT2350 bootloader
  │   └── manifest.json
  ├── groovebox/                     ← Groovebox app (from possan/tbd-pico-seq3)
  │   └── manifest.json
  ├── tusb-msc-pico/                 ← USB MSC helper for Pico (UF2)
  │   └── manifest.json
  ├── debug-probe/                   ← Debug probe app
  │   └── manifest.json
  ├── flash-nuke/                    ← Flash nuke utility
  │   └── manifest.json
  └── game/                          ← Example game app
      └── manifest.json
utilities/
  └── dada-tbd-16-tusb_msc-p4/      ← P4 USB MSC helper (bin, not in catalog)
      └── dada-tbd-16-tusb-msc.bin
stable/
  ├── p4/               ← P4 firmware (versioned, flat)
  ├── pico/             ← RP2350 Groovebox UF2
  └── releases.json     ← channel manifest with version history
staging/
  ├── p4/
  ├── pico/
  └── releases.json
feature-test-<name>/
  ├── p4/
  ├── pico/
  └── releases.json
webui-updates/
  ├── latest.json       ← latest WebUI version metadata
  └── webui-update-v*.zip  ← WebUI update packages (deduplicated)
app-catalog.json         ← auto-generated merged catalog of all apps
bundles/                 ← pre-assembled SD card bundles
```

The CDN repo has 6 GitHub Actions workflows:

| Workflow | Purpose |
|----------|---------|
| `build-catalog.yml` | Merges all `apps/*/manifest.json` into `app-catalog.json` |
| `deploy-pages.yml` | Deploys the CDN to GitHub Pages |
| `receive-firmware.yml` | Receives P4 firmware dispatch from this repo |
| `receive-pico-app.yml` | Receives Pico app dispatch from app repos |
| `validate-pr.yml` | Validates app manifest PRs (schema + SHA-256 check) |
| `build-picosd-bundle.yml` | Manual dispatch — assembles Pico SD bundle |

Each workflow uses its own concurrency group to prevent cross-cancellation.

### Manifest Format (`releases.json`)

```json
{
  "channel": "stable",
  "latest": "v0.5.0",
  "versions": [
    {
      "tag": "v0.5.0",
      "timestamp": "2026-03-22T10:00:00Z",
      "files": {
        "unified": "stable/p4/dada-tbd-16-v0.5.0-unified.bin",
        "sdcard": "stable/p4/dada-tbd-16-v0.5.0-sd.zip",
        "hash": "stable/p4/dada-tbd-16-v0.5.0-sd-hash.txt",
        "pico": "stable/pico/dada-tbd-16-v0.5.0-pico.uf2"
      },
      "webuiVersion": "0.5.0",
      "webuiUpdate": "webui-updates/webui-update-v0.5.0.zip"
    }
  ]
}
```

---

## Artifact Naming Convention

All public-facing artifacts use the **dadamachines** product name with a
device prefix (`dada-tbd-16-`) on the CDN. Build outputs keep their
original names; the CDN receive workflow renames them.

### Tag format per channel

| Channel | Tag format | Example tag | Example CDN filename |
|---------|-----------|-------------|---------------------|
| **Stable** | `v{semver}` | `v0.5.0` | `dada-tbd-16-v0.5.0-unified.bin` |
| **Staging** | `staging-v{base}-{N}` | `staging-v0.4.2-3` | `dada-tbd-16-staging-v0.4.2-3-unified.bin` |
| **Feature** | `feature-test-{name}` | `feature-test-cool-thing` | `dada-tbd-16-feature-test-cool-thing-unified.bin` |

- Staging tags are derived from `git describe`: base = nearest `v*` tag, N = commit distance
- Feature tags equal the channel name (branch `feature-test/cool-thing` → tag `feature-test-cool-thing`)
- The tag prefix (`staging-`, `feature-test-`) ensures binaries are self-identifying even outside their CDN directory

### CDN artifact table

| Artifact | Build Output | CDN Name | Notes |
|----------|-------------|----------|-------|
| Unified image | — | `dada-tbd-16-{tag}-unified.bin` | All partitions merged, flash at `0x0` |
| SD archive | `dada-tbd-sd.zip` | `dada-tbd-16-{tag}-sd.zip` | Versioned in `{channel}/p4/` |
| SD hash | `dada-tbd-sd-hash.txt` | `dada-tbd-16-{tag}-sd-hash.txt` | Versioned in `{channel}/p4/` |
| P4 USB MSC | `tusb_msc.bin` | `dada-tbd-16-tusb-msc.bin` | Fixed at `utilities/dada-tbd-16-tusb_msc-p4/` |
| Pico USB MSC | — | `tusb-msc-pico-{ver}.uf2` | In `apps/tusb-msc-pico/` (catalog-driven) |
| Pico firmware | — | `dada-tbd-16-{tag}-pico.uf2` | RP2350 Groovebox |
| App catalog | — | `app-catalog.json` | Auto-generated from `apps/*/manifest.json` |

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

## Building RP2350 Apps (Pico Side)

The TBD-16 has a second processor — an RP2350 — that runs the user
interface, MIDI I/O, and display. You can build custom apps for it.

### Quick Start

1. **Fork** [`dadamachines/dada-tbd-app-template`](https://github.com/dadamachines/dada-tbd-app-template)
2. Clone your fork and install [PlatformIO](https://platformio.org/)
3. Build: `pio run -e pi2350`
4. Flash: enter BOOTSEL mode (hold right front button on USB-C #2), drag `firmware.uf2` onto the USB drive

The template is the recommended starting point, but you can use **any
RP2350 toolchain** — Pico SDK, Arduino IDE, Rust, etc. Any toolchain that
produces a valid `.uf2` binary works.

### Publishing Your App to the App Catalog

There are two paths depending on your relationship with dadamachines:

#### Path A — Trusted collaborator (direct push)

For internal contributors with `PICO_CDN_TOKEN` access (e.g. possan).
Your CI runs a `publish-cdn` job that clones the CDN repo, commits the
`.uf2`, and pushes directly — on tagged releases:

```bash
# In your app repo, tag a release:
git tag v1.0.0
git push origin v1.0.0
# → CI builds .uf2 → pushes to CDN repo → binary lands on Pages
```

The template repo includes the `publish-cdn` job in its CI workflow.
You need a `PICO_CDN_TOKEN` secret — ask dadamachines for one.

> **Why direct push instead of `repository_dispatch`?** GitHub Actions
> `GITHUB_TOKEN` can't download artifacts across repos from private
> repositories. Direct push with a fine-grained PAT (`PICO_CDN_TOKEN`)
> works for both public and private source repos.

#### Path B — External contributor (manifest PR)

For anyone building RP2350 apps:

1. **Build** your app and **create a GitHub Release** with the `.uf2` attached
2. **Note the SHA-256**: `sha256sum my-app.uf2`
3. **Open a PR** to [`dadamachines/dada-tbd-firmware`](https://github.com/dadamachines/dada-tbd-firmware)
   adding `apps/your-app/manifest.json`:

```json
{
  "id": "your-app",
  "name": "Your App Name",
  "description": "What it does",
  "author": { "name": "Your Name", "github": "your-username" },
  "repo": "https://github.com/your-username/your-app-repo",
  "license": "MIT",
  "tier": "community",
  "category": "instrument",
  "sdFilename": "your-app.uf2",
  "releases": [{
    "version": "1.0.0",
    "firmwareCompat": "0.4",
    "date": "2026-03-22",
    "sourceUrl": "https://github.com/your-username/your-repo/releases/download/v1.0.0/your-app.uf2",
    "sha256": "abc123...",
    "size": 524288,
    "changelog": "Initial release"
  }]
}
```

4. **CI validates** your PR automatically (schema check, downloads binary, verifies SHA-256)
5. **dadamachines reviews and merges** — the binary is committed to the CDN
   repo and served from GitHub Pages (same origin as the App Manager, no
   CORS issues)

> **Why does the binary need to be in the CDN repo?**
> GitHub Release download URLs don't set `Access-Control-Allow-Origin`
> headers. The browser-based App Manager on `dadamachines.github.io` can
> only fetch files from the same origin. CI downloads the binary server-side
> (no CORS) and commits it to the CDN repo so it's served from Pages.

### Sideloading (no catalog needed)

You don't need the catalog to run your app. Just build and flash:

```bash
# Build
pio run -e pi2350

# Flash (standalone mode — replaces current app)
# Enter BOOTSEL mode → drag firmware.uf2 onto USB drive

# Or with PlatformIO upload:
pio run -t upload
```

Sideloaded apps have the same SPI API access as catalog apps — no
capability difference. The catalog is for sharing, not for running.

---

## License

This project is licensed under the terms in [LICENSE](LICENSE). Core platform
code is GPLv3 (inherited from upstream CTAG TBD).

The [RP2350 App Template](https://github.com/dadamachines/dada-tbd-app-template)
is licensed under LGPL 3.0 — your app code is yours under your license.
