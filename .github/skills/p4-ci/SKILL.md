---
name: p4-ci
description: 'CI/CD pipeline for ESP32-P4 firmware releases. USE FOR: creating stable releases; pushing to staging channel; creating feature-test builds; understanding the CI workflow architecture; checking CDN publish status; re-running CI builds; understanding channel derivation; checking what triggers what; reviewing release artifacts. DO NOT USE FOR: flashing firmware to device (use tbd-deploy skill); editing firmware C++ code; modifying WebUI code; creating WebUI update packages (use webui-update skill). TRIGGER WORDS: release, CI, pipeline, stable release, staging, feature test, CDN, publish, tag, channel, workflow, GitHub Actions, create-release, build firmware.'
argument-hint: 'Specify action: "stable release", "staging push", "feature test", "check status", or "explain pipeline"'
---

# P4 CI/CD Pipeline (ESP32-P4)

Build, release, and publish ESP32-P4 firmware to the CDN.

## Architecture Overview

The pipeline has 7 workflow files:

| Workflow | Trigger | Purpose |
|----------|---------|---------|
| `ci.yml` | PR / push to `dada-tbd-master` | Build-check (no release) |
| `build-firmware.yml` | Called by other workflows | Reusable build job (ESP-IDF Docker) |
| `create-release.yml` | Push `v*` tag | Stable release → GitHub Release → CDN stable channel |
| `staging-release.yml` | Push to `staging` branch | Pre-release → CDN staging channel |
| `feature-test-release.yml` | Push to `feature-test/*` branch | Pre-release → CDN per-feature channel |
| `build-docs.yml` | Push to `dada-tbd-master` | Build documentation |
| `deploy-docs.yml` | Push to `dada-tbd-master` | Deploy docs to GitHub Pages |

### How it flows

```
ctag-tbd repo                          CDN repo (dada-tbd-firmware)
─────────────                          ──────────────────────────────
push v0.5.0 tag
  → create-release.yml
    → build-firmware.yml (ESP-IDF)     
    → GitHub Release with artifacts
    → repository_dispatch ──────────→  receive-firmware.yml
                                         → downloads artifact
                                         → places in stable/p4/
                                         → writes stable/releases.json
                                         → deploys to GitHub Pages
```

## CDN Channel System

| Channel | Trigger | CDN path | Tag format | Example |
|---------|---------|----------|------------|----------|
| `stable` | `v*` tag on `dada-tbd-master` | `stable/p4/` | `v{semver}` | `v0.5.0` |
| `staging` | Push to `staging` branch | `staging/p4/` | `staging-v{base}-{N}` | `staging-v0.4.2-3` |
| `feature-test-<name>` | Push to `feature-test/<name>` branch | `feature-test-<name>/p4/` | `feature-test-<name>` | `feature-test-cool-thing` |

CDN URL base: `https://dadamachines.github.io/dada-tbd-firmware/`

### Separation of concerns

- **P4 repo** writes ONLY to `{channel}/p4/` — never touches `{channel}/pico/`
- **Pico repo** writes ONLY to `{channel}/pico/` — never touches `{channel}/p4/`
- Both patch their own fields into `{channel}/releases.json`
- P4 CI preserves any existing `pico` fields when writing the manifest
- P4 CI also writes the WebUI update package to `webui-updates/` and updates `webui-updates/latest.json`
- Tag naming ensures binaries are self-identifying: `staging-` prefix for staging, `feature-test-` prefix for features

### WebUI versioning in releases.json

Each version entry in `releases.json` includes WebUI metadata:

```json
{
  "tag": "v0.5.0",
  "timestamp": "2026-03-23T12:00:00Z",
  "files": { "unified": "...", "sdcard": "...", "hash": "...", "pico": "..." },
  "webuiVersion": "0.4.0",
  "webuiUpdate": "webui-updates/webui-update-v0.4.0.zip"
}
```

The flash pages use `webuiVersion` and `webuiUpdate` to guide users: **WebUI must be
updated before firmware flash** to avoid compatibility issues (e.g., the `.jsn` → `.json`
rename in v0.4.0 would break older firmware that expects `.jsn` files).

**Firmware-only releases** (e.g., v0.4.5 ships with same WebUI 0.4.2 as v0.4.3): CI still
builds and packages the WebUI — this is expected. The CDN deduplicates automatically
(`latest.json` won't create a new version entry if the version is unchanged). The device's
updater page shows "✓ WebUI is up to date" and the user skips to firmware flash.

## Workflow Files

All at `.github/workflows/`:

- [ci.yml](../../.github/workflows/ci.yml)
- [build-firmware.yml](../../.github/workflows/build-firmware.yml)
- [create-release.yml](../../.github/workflows/create-release.yml)
- [staging-release.yml](../../.github/workflows/staging-release.yml)
- [feature-test-release.yml](../../.github/workflows/feature-test-release.yml)

## Build Artifacts

The reusable `build-firmware.yml` produces:

| File | Description |
|------|-------------|
| `dada-tbd.bin` | Main P4 firmware |
| `bootloader.bin` | ESP-IDF bootloader |
| `partition-table.bin` | Partition table |
| `ota_data_initial.bin` | OTA data (boots ota_0) |
| `dada-tbd-sd.zip` | SD card filesystem archive |
| `dada-tbd-sd-hash.txt` | xxh128 hash of SD contents |
| `dada-tbd-16-{tag}-unified.bin` | Unified flash image (all-in-one) |
| `webui-update-v{version}.zip` | WebUI update package (full, CI-generated) |

Artifact name pattern: `firmware-{build_name}` (retention: 90 days).

The build also outputs `webui_version` (read from `sdcard_image/data/webui-version.json`),
which is passed to the CDN for inclusion in `releases.json`.

## Procedures

### Create a Stable Release

```bash
# 1. Make sure dada-tbd-master is clean and tested
git checkout dada-tbd-master
git pull origin dada-tbd-master

# 2. Tag the release
git tag v0.5.0
git push origin v0.5.0
```

This triggers `create-release.yml` which:
1. Builds firmware via `build-firmware.yml` (includes WebUI update package)
2. Creates a GitHub Release with all artifacts
3. Dispatches to CDN → `receive-firmware.yml` places files in `stable/p4/` and WebUI package in `webui-updates/`

### Push to Staging

```bash
# 1. Merge or push your changes to the staging branch
git checkout staging
git merge dada-tbd-master   # or cherry-pick specific commits
git push origin staging
```

This triggers `staging-release.yml` which:
1. Derives version tag from git: `staging-v{base}-{N}` (e.g. `staging-v0.4.2-3`)
2. Builds firmware (includes WebUI update package)
3. Creates a GitHub pre-release
4. Dispatches to CDN → files go to `staging/p4/`, WebUI package to `webui-updates/`

### Create a Feature Test Build

```bash
# 1. Create and push a feature-test branch
git checkout -b feature-test/cool-thing
# ... make changes ...
git push origin feature-test/cool-thing
```

This triggers `feature-test-release.yml` which:
1. Derives channel + tag from branch: `feature-test/cool-thing` → tag `feature-test-cool-thing`
2. Builds firmware (includes WebUI update package)
3. Creates a GitHub pre-release (each push overwrites the previous tag)
4. Dispatches to CDN → files go to `feature-test-cool-thing/p4/`, WebUI package to `webui-updates/`

### Check CI Status

```bash
# View recent workflow runs
gh run list --repo dadamachines/ctag-tbd --limit 5

# View a specific run
gh run view <run-id> --repo dadamachines/ctag-tbd

# Check CDN for the latest published version
curl -s https://dadamachines.github.io/dada-tbd-firmware/stable/releases.json | jq .
```

## Verification

After any release, verify the CDN has the expected files:

```bash
CHANNEL="stable"  # or staging, feature-test-<name>

# Check releases.json (now includes webuiVersion and webuiUpdate per version)
curl -s "https://dadamachines.github.io/dada-tbd-firmware/${CHANNEL}/releases.json" | jq .

# Check unified image exists (replace TAG with actual version)
TAG="v0.5.0"
curl -sI "https://dadamachines.github.io/dada-tbd-firmware/${CHANNEL}/p4/dada-tbd-16-${TAG}-unified.bin" | head -1

# Check SD card image exists
curl -sI "https://dadamachines.github.io/dada-tbd-firmware/${CHANNEL}/p4/dada-tbd-16-${TAG}-sd.zip" | head -1

# Check WebUI update package exists
curl -sI "https://dadamachines.github.io/dada-tbd-firmware/webui-updates/webui-update-v$(jq -r '.versions[0].webuiVersion' <<< "$(curl -s https://dadamachines.github.io/dada-tbd-firmware/${CHANNEL}/releases.json)").zip" | head -1
```

## Troubleshooting

| Problem | Cause | Fix |
|---------|-------|-----|
| CI didn't trigger | Files changed didn't match `paths` filter in `ci.yml` | Check that modified files are in the paths list |
| Release build failed | ESP-IDF build error | Check workflow run logs: `gh run view <id> --log` |
| CDN not updated | Dispatch failed or CDN workflow failed | Check both repos' Actions tabs |
| `releases.json` missing pico fields | Expected — P4 CI writes P4 fields only | Pico CI patches pico fields separately |
| WebUI package not on CDN | `webui_version` output empty or dispatch failed | Check `sdcard_image/data/webui-version.json` exists and build logs |
| Feature test channel wrong name | Branch name derives the channel | Use `feature-test/<name>` (no nested slashes) |
| GitHub Release not created | Tag didn't match `v*` pattern | Tag must start with `v` (e.g. `v0.5.0`) |

## CI Path Filters

`ci.yml` only triggers on changes to firmware-relevant files:

```
main/**
components/**
CMakeLists.txt
sdkconfig
sdkconfig.defaults
sdkconfig.defaults.*
partitions_example.csv
partitions_no_sd.csv
patches/**
sdcard_image/**
sample_rom/**
create_sd_archive.sh
create_unified_p4_firmware.sh
.github/workflows/build-firmware.yml
.github/workflows/ci.yml
```

Docs-only, WebUI-only, or skill-file commits do NOT trigger a firmware build.

## Multi-Config CI

`ci.yml` runs two parallel jobs:

| Job | Configuration | What it does |
|-----|---------------|--------------|
| `build-check` | TBD-16 (Config D, default) | Full build via `build-firmware.yml` — produces all artifacts |
| `compile-check-configs` (matrix) | Configs A, B, C | Lightweight `idf.py build` — verifies compilation only |

The matrix uses sdkconfig overlay files (`sdkconfig.defaults.tbd-*`) that
are merged on top of `sdkconfig.defaults` via `SDKCONFIG_DEFAULTS`. See
[HARDWARE_CONFIGURATIONS.md](../../HARDWARE_CONFIGURATIONS.md) for
the full configuration reference.

Release workflows always build Config D (TBD-16) only.

## Related Resources

- [GIT_STRATEGY.md](../../docs/GIT_STRATEGY.md) — full architecture reference
- [POSSAN_CDN_SETUP.md](../../docs/POSSAN_CDN_SETUP.md) — Pico-side CDN setup
- [CONTRIBUTING.md](../../CONTRIBUTING.md) — contribution workflows
