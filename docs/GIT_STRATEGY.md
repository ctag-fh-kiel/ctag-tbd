# Git, CI/CD & Binary Asset Strategy for dadamachines-ctag-tbd

> Recommendations for handling firmware binaries, SD card images,
> repository structure, CI firmware builds, and the upstream relationship
> with ctag-fh-kiel/ctag-tbd.
>
> Date: 2026-03-20
>
> **Product status:** Pre-release. ~10 test users, no shipped product.
> This means we have a clean slate — no backwards compatibility burden,
> no public URLs to preserve, no production users depending on current
> repository structure or flash page layout. The strategy below takes
> advantage of this: aggressive cleanup now, proper infrastructure before
> the first real release.
>
> Related documents:
> - `proposal-branching-strategy.md` — upstream/fork collaboration model
> - `proposal-simple-tbd-config.md` — Kconfig flags for hardware configs

---

## 1. The Two Repositories

This strategy covers the **dadamachines product repo** and its relationship
with the **ctag-fh-kiel upstream** (currently dormant):

| | **ctag-tbd** (upstream, dormant) | **dadamachines TBD-16** (active) |
|---|---|---|
| Repository | `ctag-fh-kiel/ctag-tbd` | `dadamachines/ctag-tbd` |
| Primary branch | `p4_main` | `dada-tbd-master` |
| License | GPLv3 | Parts LGPL (product-specific) |
| Hardware | ESP32-P4 only | ESP32-P4 + RP2350 (TBD-16) |
| Focus | Core DSP platform, plugins, research | Commercial product, sequencer, UI, presets |
| Status | 0 commits since fork point | 180+ commits ahead, actively developed |

```
dadamachines/ctag-tbd (dada-tbd-master) ← active development, TBD-16 product
        │
        │  Kconfig guards keep core code portable
        │  upstream can adopt when they're ready
        ▼
ctag-fh-kiel/ctag-tbd (p4_main)        ← dormant; original core platform, GPLv3
```

```
dadamachines/ctag-tbd (dada-tbd-master) ← canonical
        │
        │  engineer forks + PRs
        ▼
engineer/ctag-tbd (feature/*)          ← development, PRs back to dada-tbd-master
```

`dada-tbd-master` was originally based on `p4_main` (now 180+ commits ahead).
All core DSP engine code, audio codec drivers, and plugin architecture came
from upstream. dadamachines added the RP2350 SPI bridge, sequencer,
macro/preset system, PicoSeqRack, WebUI, and product-specific configuration.
Upstream has had zero activity since the fork — dadamachines is the active
project.

---

## 2. Current Situation — Sizes & Branches

### Binary asset sizes in working tree

| Directory | Size | Contents |
|---|---|---|
| `docs/_static/firmware/` | 43 MB | 12 P4 `.bin` + 9 Pico `.uf2` files (accumulated builds) |
| `docs/_static/sdcard_image/` | 203 MB | 8 date-versioned SD card `.zip` + hash files |
| `docs/_static/updates/` | 1.5 MB | 6 WebUI update `.zip` packages |
| `sample_rom/` | 67 MB | 869+ `.wav` samples for factory ROM |
| **Total working tree** | **~434 MB** | mostly in `docs/` |
| **Git pack (`.git/`)** | **246 MB** | every version of the above stored as full blobs |

All binaries are tracked as regular Git objects — no Git LFS, no
`.gitattributes`. Every clone downloads the full 246 MB pack regardless of
whether the user needs firmware or samples.

### Branches (current — dadamachines/ctag-tbd)

```
* dada-tbd-master          ← primary, deploys docs via CI
  dev
  feature/esphome-build-system
  feature/new-web-ui
  feature/spi-communication
  feature/wave_preview
  legacy-master-1.0.0
  master
  p4_main
  p4_main_sdonly
  perf_test
```

### What must stay in the repo (non-negotiable)

- **`sdcard_image/`** — contains the WebUI (`www/`), data directory (presets,
  macro definitions, synth definitions), and sample ROM index. This is the
  filesystem that ships on the device SD card and **must** stay in sync with
  the C++ firmware. Every firmware build needs this.
- **`sample_rom/`** — factory `.wav` samples needed by engineers doing full
  releases (factory SD card image creation via `create_sd_archive.sh`).
- **`docs/*.rst`** — Sphinx source files (lightweight text, no issue here).

---

## 3. Problems to Solve

1. **Repo bloat** — 246 MB pack makes clones slow; grows with every release.
2. **Engineer friction** — contributors must download all firmware + SD images
   to work on C++ code or WebUI.
3. **No way to remove old binaries** — deleting old `.bin`/`.zip` files from
   `HEAD` still keeps them in Git history forever.
4. **Flash tools need stable download URLs** — once the product ships, the
   browser-based flash pages (`docs/flash/`) need permanent download URLs
   for firmware. GitHub Release asset URLs provide this. (Not a concern yet
   with 10 test users, but the infrastructure should be in place before
   launch.)
5. **No CI firmware builds** — firmware is built manually on engineer machines.
   No reproducible build pipeline exists.
6. **Stale branches** — 10+ branches in the repo, most historical.
7. **Informal upstream relationship** — upstream (`ctag-fh-kiel/ctag-tbd`
   `p4_main`) is dormant (0 commits since fork). No formal process exists
   for when/how they adopt dadamachines' improvements.

---

## 4. GitHub Pages — No `gh-pages` Branch Needed

The current CI setup uses the **artifact-based** GitHub Pages deployment
(the modern approach), NOT a `gh-pages` branch.

**How it works now (and should continue to work):**

```
push to dada-tbd-master
  → deploy-docs.yml triggers
    → build-docs.yml runs
      → Docker builds Sphinx HTML from docs/
      → actions/upload-pages-artifact@v3 uploads the built site
    → actions/deploy-pages@v4 publishes to GitHub Pages
```

The `actions/deploy-pages` action deploys directly from the uploaded
artifact. There is **no `gh-pages` branch** and you don't need one. This is
the recommended approach since 2022 — it's cleaner than branch-based
deployment because:

- No generated HTML committed to any branch
- No merge conflicts in built output
- Atomic deploys (artifact is immutable)
- All `docs/_static/` assets (firmware, SD images, updates) are included in
  the artifact because they live inside the `docs/` source tree

The GitHub Pages site is at: `https://dadamachines.github.io/ctag-tbd/`

**To enable this**, the GitHub repo settings must have:
- Settings → Pages → Source: **GitHub Actions** (not "Deploy from a branch")

This is already configured since the current docs deployment works.

---

## 5. Delete Old Binaries, Use GitHub Releases Going Forward

**This is the highest-impact change.** The repo contains ~195 MB of
date-stamped firmware builds and SD card snapshots that no one needs. With
only ~10 test users and no shipped product, there's no reason to preserve
these — just delete them.

### What to delete

```bash
# Delete ALL old firmware builds and SD card images
git rm -r docs/_static/firmware/p4/possan-tbd-*.bin
git rm -r docs/_static/firmware/pico/possan-tbd-*.uf2
git rm -r docs/_static/sdcard_image/2026-*
git rm -r docs/_static/updates/*.zip 2>/dev/null  # old WebUI update zips

# Keep only:
# - docs/_static/firmware/p4/tusb_msc.bin (USB MSC helper, needed by flash pages)
# - docs/_static/picoflash/ (Picoboot flash tool, needed by flash pages)
git commit -m "chore: remove all old firmware builds from repo"
```

There's no migration script, no GitHub Releases for old builds, no URL
redirects. The old builds were pre-release test artifacts for ~10 people.
They're gone.

### Going forward: GitHub Releases for every build

From now on, CI creates a **GitHub Release** for each tagged build. Release
assets get permanent, versioned download URLs:

```
https://github.com/dadamachines/ctag-tbd/releases/download/v0.4.0/dada-tbd-16-v0.4.0.bin
https://github.com/dadamachines/ctag-tbd/releases/download/v0.4.0/dada-tbd-16-groovebox-v0.4.0.uf2
https://github.com/dadamachines/ctag-tbd/releases/download/v0.4.0/dada-tbd-16-p4sd-v0.4.0.zip
https://github.com/dadamachines/ctag-tbd/releases/download/v0.4.0/dada-tbd-16-p4sd-v0.4.0-hash.txt
```

No firmware binaries committed to the repo. The flash pages fetch directly
from Release URLs. The `docs/_static/firmware/` directory only holds
manifests (JSON) and the small MSC helper binary.

### Size impact

| Asset | Current in-repo | After cleanup |
|---|---|---|
| Old firmware `.bin` + `.uf2` | ~40 MB | 0 (deleted) |
| Old SD card `.zip` files | ~180 MB | 0 (deleted) |
| Latest build | ~25 MB | 0 (will be on GitHub Releases) |
| `tusb_msc.bin` (kept) | ~2 MB | ~2 MB |
| **Savings** | | **~220 MB from working tree** |

---

## 6. Git LFS for Remaining Binaries

After the release migration, the remaining binary files in the repo are:

- Latest firmware build (one `.bin`, one `.uf2`)
- Latest SD card `.zip` + hash
- WebUI update `.zip` packages (1.5 MB total)
- `sample_rom/` wav files (67 MB, rarely changes)
- `tusb_msc.bin` (USB MSC helper, rarely changes)
- Logo/mockup images in `docs/` (a few MB)

Track these with Git LFS:

```gitattributes
# .gitattributes — track binary files via Git LFS
docs/_static/firmware/**/*.bin filter=lfs diff=lfs merge=lfs -text
docs/_static/firmware/**/*.uf2 filter=lfs diff=lfs merge=lfs -text
docs/_static/sdcard_image/**/*.zip filter=lfs diff=lfs merge=lfs -text
docs/_static/updates/**/*.zip filter=lfs diff=lfs merge=lfs -text
sample_rom/**/*.wav filter=lfs diff=lfs merge=lfs -text
sample_rom/**/*.tbd filter=lfs diff=lfs merge=lfs -text
*.jpg filter=lfs diff=lfs merge=lfs -text
*.png filter=lfs diff=lfs merge=lfs -text
```

**Benefits for engineers:**

```bash
# Clone without any binaries (fast, ~20 MB):
GIT_LFS_SKIP_SMUDGE=1 git clone <url>

# Then selectively pull what you need:
git lfs pull --include="sdcard_image/**"   # WebUI / data (needed for dev builds)
git lfs pull --include="sample_rom/**"     # factory samples (only for full releases)
```

**GitHub LFS storage:** Free tier includes 1 GB storage + 1 GB/month
bandwidth. After moving historical builds to Releases, LFS usage should be
well under 100 MB.

**CI integration:** Add `lfs: true` to checkout steps in workflows:

```yaml
- uses: actions/checkout@v4
  with:
    lfs: true      # ← downloads LFS files needed for docs build
```

---

## 7. CI Firmware Builds

Currently firmware is built manually on engineer machines. Adding CI builds
provides reproducible releases, artifact archiving, and a path to
fully-automated releases.

### Build requirements

| Component | Version | Notes |
|---|---|---|
| ESP-IDF | **v5.5.3** | Toolchain + build system (see patch note below) |
| CMake | 3.13+ | Required by IDF |
| Python | 3.8+ | For IDF build scripts |
| xxhash | any | `xxh128sum` for SD card checksums |
| C++ | C++20 | Enforced in `CMakeLists.txt` |
| Git | with tags | `git describe --tags --always` for version string |

> **IDF version note:** `idf_component.yml` declares `>=5.5.1` as the
> minimum, and the current `sdkconfig.defaults` was generated with v5.5.2.
> However, the project carries **ESP-IDF patches** (see below) that target
> **v5.5.3** specifically. Use `espressif/idf:v5.5.3` for CI to match
> what engineers build locally. The Docker image exists on Docker Hub.

### ESP-IDF patches (`patches/` directory)

The experimental branch (`nevvkid/dada-tbd-master-experimental`) adds a
`patches/` directory with IDF patches applied at CMake configure time. This
is critical for CI — the patches fix real ESP32-P4 hardware issues:

**`patches/esp-idf-v5.5.3-p4-fixes.patch`:**

1. **MMU map fix** (`esp_mmu_map.c`) — replaces cache disable/enable calls
   with mutex locks in `s_vaddr_to_paddr()` and `s_paddr_to_vaddr()`. Fixes
   a race condition that could cause PSRAM access crashes.

2. **USB DWC HAL fix** (`usb_dwc_hal.c`) — clears USB PHY suspend state on
   ESP32-P4 that may persist after light sleep. Cherry-picked from ESP-IDF
   `release/v5.5` commit `fdfcae2`. Fixes USB reliability after power
   management transitions.

**How the patch system works** (added to `CMakeLists.txt`):

```cmake
# Applied at configure time — idempotent (safe to re-run)
file(GLOB IDF_PATCHES "${CMAKE_SOURCE_DIR}/patches/*.patch")
foreach(PATCH_FILE ${IDF_PATCHES})
    execute_process(
        COMMAND git apply --check "${PATCH_FILE}"
        WORKING_DIRECTORY $ENV{IDF_PATH}
        RESULT_VARIABLE PATCH_CHECK_RESULT
        OUTPUT_QUIET ERROR_QUIET
    )
    if(PATCH_CHECK_RESULT EQUAL 0)
        execute_process(
            COMMAND git apply "${PATCH_FILE}"
            WORKING_DIRECTORY $ENV{IDF_PATH}
        )
    endif()
endforeach()
```

`git apply --check` tests whether the patch is already applied before
attempting to apply it. This means CI can use the stock Espressif Docker
image — the patches are applied automatically during the first build.

> **Important:** These patches and the CMakeLists.txt changes currently live
> on `nevvkid/dada-tbd-master-experimental` (788 commits ahead of
> `dada-tbd-master`). They must be merged into `dada-tbd-master` before
> CI firmware builds will work correctly. The experimental branch also
> updates `sdkconfig.defaults` to remove stale TinyUSB task config options
> and adds `--always` to the `git describe` call (so builds without tags
> still get a version string).

### Build outputs

```
build/
├── ctag-tbd.bin                        ← main application firmware
├── bootloader/bootloader.bin           ← bootloader
├── partition_table/partition-table.bin  ← partition table
├── ota_data_initial.bin                ← OTA metadata
├── dada-tbd-16-p4sd.zip               ← P4 SD card image (WebUI + presets + samples)
└── dada-tbd-16-p4sd-hash.txt          ← xxh128 hash of P4 SD archive
```

The unified firmware image (all 4 binaries merged) is created by the
existing `create_unified_p4_firmware.sh` script:

```
esptool.py merge_bin → docs/_static/firmware/p4/<name>-unified.bin
  0x2000  bootloader.bin
  0x8000  partition-table.bin
  0xd000  ota_data_initial.bin
  0x10000 ctag-tbd.bin
```

### Workflow: Build Firmware (`build-firmware.yml`)

```yaml
# .github/workflows/build-firmware.yml
name: Build Firmware

on:
  workflow_call:
    inputs:
      build_name:
        description: 'Build name (e.g. possan-tbd-2026-03-19)'
        required: true
        type: string
    outputs:
      artifact_name:
        description: 'Name of the uploaded artifact'
        value: ${{ jobs.build.outputs.artifact_name }}

jobs:
  build:
    runs-on: ubuntu-latest
    container:
      image: espressif/idf:v5.5.3
    outputs:
      artifact_name: firmware-${{ inputs.build_name }}

    steps:
      - name: Install system dependencies
        run: apt-get update && apt-get install -y xxhash

      - name: Check out repo
        uses: actions/checkout@v4
        with:
          lfs: true
          fetch-depth: 0          # needed for git describe version string
          submodules: recursive

      - name: Build firmware
        run: |
          # IDF patches in patches/ are applied automatically by CMakeLists.txt
          # during configure. No manual patch step needed.
          . $IDF_PATH/export.sh
          idf.py build

      - name: Create unified firmware image
        run: |
          . $IDF_PATH/export.sh
          ./create_unified_p4_firmware.sh "${{ inputs.build_name }}"

      - name: Create P4 SD card archive
        run: |
          . $IDF_PATH/export.sh
          ./create_sd_archive.sh sdcard_image build "$(which xxh128sum)"
          # Renames output to p4sd naming
          mv build/dada-*-sd.zip build/dada-${{ inputs.build_name }}-p4sd.zip 2>/dev/null || true
          mv build/dada-*-sd-hash.txt build/dada-${{ inputs.build_name }}-p4sd-hash.txt 2>/dev/null || true

      - name: Upload firmware artifacts
        uses: actions/upload-artifact@v4
        with:
          name: firmware-${{ inputs.build_name }}
          retention-days: 90
          path: |
            build/ctag-tbd.bin
            build/bootloader/bootloader.bin
            build/partition_table/partition-table.bin
            build/ota_data_initial.bin
            build/dada-*-p4sd.zip
            build/dada-*-p4sd-hash.txt
            docs/_static/firmware/p4/${{ inputs.build_name }}-unified.bin
```

**Key details:**

- Uses Espressif's official Docker image (`espressif/idf:v5.5.3`) which has
  the full toolchain, esptool.py, and Python pre-installed.
- The ESP-IDF patches in `patches/` are applied **automatically** by the
  patch system in `CMakeLists.txt` during `idf.py build` (at CMake configure
  time). No separate patch step is needed in CI — the stock Docker image +
  the repo's patch directory is all that's required.
- `fetch-depth: 0` is required because the build runs
  `git describe --tags --always` to generate the firmware version string.
- `submodules: recursive` pulls `components/ableton_link/link/` and any
  other submodules.
- The P4 SD card archive is built from `sdcard_image/` using
  `create_sd_archive.sh` — this ensures the WebUI + data bundled in the
  release always matches the firmware.

### Workflow: Create Release (`create-release.yml`)

This ties the firmware build + docs build + GitHub Release together. Can be
triggered manually or on tag push:

```yaml
# .github/workflows/create-release.yml
name: Create Release

on:
  push:
    tags:
      - 'v*'              # trigger on version tags (v2026-03-19, v1.0.0, etc.)
  workflow_dispatch:
    inputs:
      build_name:
        description: 'Build name (e.g. possan-tbd-2026-03-19)'
        required: true

permissions:
  contents: write         # needed to create releases

jobs:
  # ── Step 1: Build firmware ──
  firmware:
    uses: ./.github/workflows/build-firmware.yml
    with:
      build_name: ${{ inputs.build_name || github.ref_name }}

  # ── Step 2: Create GitHub Release with all artifacts ──
  release:
    needs: firmware
    runs-on: ubuntu-latest
    steps:
      - name: Download firmware artifacts
        uses: actions/download-artifact@v4
        with:
          name: firmware-${{ inputs.build_name || github.ref_name }}
          path: release-assets/

      - name: Create GitHub Release
        uses: softprops/action-gh-release@v2
        with:
          tag_name: ${{ inputs.build_name && format('v{0}', inputs.build_name) || github.ref_name }}
          name: "Build ${{ inputs.build_name || github.ref_name }}"
          files: release-assets/**/*
          generate_release_notes: true

  # ── Step 3: Rebuild + deploy docs (picks up latest firmware) ──
  docs:
    needs: release
    uses: ./.github/workflows/build-docs.yml
```

### Workflow: CI on Pull Requests (build-check only)

To catch build regressions before merge:

```yaml
# .github/workflows/ci.yml
name: CI

on:
  pull_request:
    branches: [dada-tbd-master]
    paths:
      - 'main/**'
      - 'components/**'
      - 'CMakeLists.txt'
      - 'sdkconfig.defaults'

jobs:
  build-check:
    runs-on: ubuntu-latest
    container:
      image: espressif/idf:v5.5.3
    steps:
      - name: Install system dependencies
        run: apt-get update && apt-get install -y xxhash

      - uses: actions/checkout@v4
        with:
          fetch-depth: 0
          submodules: recursive

      - name: Build firmware
        run: |
          . $IDF_PATH/export.sh
          idf.py build
```

This runs on PRs that touch firmware code, verifying the build still
succeeds. It doesn't create releases or deploy — just checks.

### Future: Multi-config CI builds (aligns with Kconfig proposal)

Once the Kconfig flags from `proposal-simple-tbd-config.md` are implemented
(`CONFIG_TBD_USE_SD_CARD`, `CONFIG_TBD_USE_RP2350`), CI should build and
verify all four hardware configurations:

```yaml
# Future addition to ci.yml
strategy:
  matrix:
    config:
      - name: "Config D — Full TBD-16"
        sdcard: y
        rp2350: y
      - name: "Config C — SD TBD (no RP2350)"
        sdcard: y
        rp2350: n
      - name: "Config B — Sequencer TBD (no SD)"
        sdcard: n
        rp2350: y
      - name: "Config A — Minimal TBD"
        sdcard: n
        rp2350: n

steps:
  - name: Set build config
    run: |
      echo "CONFIG_TBD_USE_SD_CARD=${{ matrix.config.sdcard }}" >> sdkconfig.defaults
      echo "CONFIG_TBD_USE_RP2350=${{ matrix.config.rp2350 }}" >> sdkconfig.defaults

  - name: Build
    run: |
      . $IDF_PATH/export.sh
      idf.py build
```

This ensures every PR is compatible with all hardware configs, preventing
regressions in the core platform that would affect upstream ctag-fh-kiel.

---

## 8. Staging Channel — Pre-release Distribution

### The problem

Engineers build firmware on their own machines and manually create binaries
for users to test. This has several issues:

- Different toolchains → different binaries → "works on my machine"
- No way to verify that the exact binary users tested is the one being
  released
- Beta users must be individually guided to download links
- No audit trail for which build a user is running

### The goal

A **staging channel** where engineers can push pre-release builds that are:

1. **Built by CI** (same Docker image, same toolchain as production)
2. **Distributed via the docs site** (browser-based flash, same UX as stable)
3. **Clearly labeled** as pre-release (users know it's not stable)
4. **Validated before promotion** — a staging build that passes testing gets
   merged to `dada-tbd-master` and becomes the next stable release

### Branch model

```
dada-tbd-master         ← stable releases (production)
  │
  └── staging            ← pre-release builds (testing)
        │
        ├── feature/...  ← engineer work (on personal forks)
        └── (experimental branches can merge here first)
```

`staging` is a long-lived branch that sits between feature work and
production. It is **not** a merge queue — it's a release candidate channel.

**Workflow:**

1. Engineers merge feature branches into `staging` (via PR or direct push)
2. CI builds firmware on `staging` push → creates a **pre-release** on
   GitHub Releases
3. The staging flash page picks up new builds automatically (via a JSON
   manifest)
4. Beta users test via the browser-based flash tool
5. When ready, `staging` is merged into `dada-tbd-master` → CI creates a
   **stable release**
6. `staging` is rebased/reset to `dada-tbd-master` after each stable release

### How artifacts reach the flash page

The key architectural decision: **staging firmware lives in GitHub Releases,
not in the repo.** This avoids binary bloat in git and works with the
existing docs deployment model.

```
staging push
    │
    ├──▸ CI builds firmware (espressif/idf:v5.5.3 Docker)
    ├──▸ CI creates GitHub pre-release: staging-2026-03-20
    │       ├── dada-tbd-16-staging-2026-03-20.bin   (unified P4 firmware)
    │       ├── dada-tbd-16-groovebox-staging-2026-03-20.uf2 (RP2350 Pico)
    │       ├── dada-tbd-16-p4sd-staging-2026-03-20.zip (P4 SD card image)
    │       └── dada-tbd-16-p4sd-staging-2026-03-20-hash.txt
    │
    └──▸ CI updates staging-manifest.json in docs/_static/firmware/
            (committed back to dada-tbd-master, triggers docs redeploy)
```

The flash page reads `staging-manifest.json` at page load and populates the
package selector dynamically. No RST edits needed per build.

### Manifest format: `docs/_static/firmware/staging-manifest.json`

```json
{
  "channel": "staging",
  "updated": "2026-03-20T14:30:00Z",
  "builds": [
    {
      "name": "dada-tbd-16-staging-2026-03-20",
      "date": "2026-03-20",
      "branch": "staging",
      "commit": "abc1234",
      "changelog": "USB NCM fix, new PicoSeqRack processors",
      "p4Url": "https://github.com/dadamachines/ctag-tbd/releases/download/staging-2026-03-20/dada-tbd-16-staging-2026-03-20.bin",
      "picoUrl": "https://github.com/dadamachines/ctag-tbd/releases/download/staging-2026-03-20/dada-tbd-16-groovebox-staging-2026-03-20.uf2",
      "p4sdUrl": "https://github.com/dadamachines/ctag-tbd/releases/download/staging-2026-03-20/dada-tbd-16-p4sd-staging-2026-03-20.zip",
      "p4sdHashUrl": "https://github.com/dadamachines/ctag-tbd/releases/download/staging-2026-03-20/dada-tbd-16-p4sd-staging-2026-03-20-hash.txt"
    },
    {
      "name": "dada-tbd-16-staging-2026-03-18",
      "date": "2026-03-18",
      "branch": "staging",
      "commit": "def5678",
      "changelog": "I2C stability, macro export fix",
      "p4Url": "https://github.com/dadamachines/ctag-tbd/releases/download/staging-2026-03-18/dada-tbd-16-staging-2026-03-18.bin",
      "picoUrl": "https://github.com/dadamachines/ctag-tbd/releases/download/staging-2026-03-18/dada-tbd-16-groovebox-staging-2026-03-18.uf2",
      "p4sdUrl": "https://github.com/dadamachines/ctag-tbd/releases/download/staging-2026-03-18/dada-tbd-16-p4sd-staging-2026-03-18.zip",
      "p4sdHashUrl": "https://github.com/dadamachines/ctag-tbd/releases/download/staging-2026-03-18/dada-tbd-16-p4sd-staging-2026-03-18-hash.txt"
    }
  ]
}
```

The manifest is a simple JSON file committed to the repo. CI appends new
entries on each staging build. The flash page reads it client-side — the
browser fetches the manifest, then fetches the actual binaries directly from
GitHub Releases (CORS is supported on `github.com` release assets).

> **Why not commit binaries to docs/_static?** Because every staging build
> would add ~16 MB to the repo. With GitHub Releases, the binaries live
> outside the repo and are available forever without bloating git history.

### Workflow: Build & Publish Staging (`staging-release.yml`)

```yaml
# .github/workflows/staging-release.yml
name: Staging Release

on:
  push:
    branches: [staging]

permissions:
  contents: write          # needed to create releases + push manifest

jobs:
  # ── Step 1: Build firmware ──
  build:
    uses: ./.github/workflows/build-firmware.yml
    with:
      build_name: "dada-tbd-16-staging-${{ github.event.head_commit.timestamp }}"

  # ── Step 2: Create pre-release on GitHub Releases ──
  publish:
    needs: build
    runs-on: ubuntu-latest
    steps:
      - name: Generate release tag
        id: tag
        run: |
          DATE=$(date -u +%Y-%m-%d)
          SHORT_SHA=${GITHUB_SHA::7}
          echo "tag=staging-${DATE}" >> "$GITHUB_OUTPUT"
          echo "name=dada-tbd-16-staging-${DATE}" >> "$GITHUB_OUTPUT"
          echo "date=${DATE}" >> "$GITHUB_OUTPUT"
          echo "sha=${SHORT_SHA}" >> "$GITHUB_OUTPUT"

      - name: Download firmware artifacts
        uses: actions/download-artifact@v4
        with:
          name: firmware-dada-tbd-16-staging-*
          path: release-assets/
          merge-multiple: true

      - name: Rename artifacts for release
        run: |
          TAG="${{ steps.tag.outputs.name }}"
          # Unified P4 firmware
          mv release-assets/*-unified.bin "release-assets/${TAG}.bin" 2>/dev/null || \
          mv release-assets/ctag-tbd.bin "release-assets/${TAG}.bin"
          # Pico firmware (if exists)
          if ls release-assets/*.uf2 2>/dev/null; then
            mv release-assets/*.uf2 "release-assets/${TAG}.uf2"
          fi
          # P4 SD card archive
          mv release-assets/*-p4sd.zip "release-assets/${TAG}-p4sd.zip" 2>/dev/null || true
          mv release-assets/*-p4sd-hash.txt "release-assets/${TAG}-p4sd-hash.txt" 2>/dev/null || true

      - name: Create GitHub pre-release
        uses: softprops/action-gh-release@v2
        with:
          tag_name: ${{ steps.tag.outputs.tag }}
          name: "Staging Build ${{ steps.tag.outputs.date }}"
          prerelease: true
          generate_release_notes: true
          files: |
            release-assets/${{ steps.tag.outputs.name }}.bin
            release-assets/${{ steps.tag.outputs.name }}.uf2
            release-assets/${{ steps.tag.outputs.name }}-p4sd.zip
            release-assets/${{ steps.tag.outputs.name }}-p4sd-hash.txt

  # ── Step 3: Update staging manifest (on dada-tbd-master) ──
  update-manifest:
    needs: publish
    runs-on: ubuntu-latest
    steps:
      - name: Check out dada-tbd-master
        uses: actions/checkout@v4
        with:
          ref: dada-tbd-master
          token: ${{ secrets.GITHUB_TOKEN }}

      - name: Update staging-manifest.json
        run: |
          TAG="${{ needs.publish.outputs.tag }}"
          NAME="${{ needs.publish.outputs.name }}"
          DATE="${{ needs.publish.outputs.date }}"
          SHA="${{ needs.publish.outputs.sha }}"
          MANIFEST="docs/_static/firmware/staging-manifest.json"
          BASE_URL="https://github.com/dadamachines/ctag-tbd/releases/download/${TAG}"

          # Read commit message as changelog
          CHANGELOG=$(git -C /tmp log --format=%s -1 "$GITHUB_SHA" 2>/dev/null || echo "Staging build ${DATE}")

          # Create manifest if it doesn't exist
          if [ ! -f "$MANIFEST" ]; then
            echo '{"channel":"staging","updated":"","builds":[]}' > "$MANIFEST"
          fi

          # Prepend new build entry using Python (available in ubuntu-latest)
          python3 -c "
          import json, sys
          from datetime import datetime, timezone
          m = json.load(open('$MANIFEST'))
          m['updated'] = datetime.now(timezone.utc).isoformat()
          entry = {
              'name': '${NAME}',
              'date': '${DATE}',
              'branch': 'staging',
              'commit': '${SHA}',
              'changelog': '''${CHANGELOG}''',
              'p4Url': '${BASE_URL}/${NAME}.bin',
              'picoUrl': '${BASE_URL}/${NAME}.uf2',
              'p4sdUrl': '${BASE_URL}/${NAME}-p4sd.zip',
              'p4sdHashUrl': '${BASE_URL}/${NAME}-p4sd-hash.txt',
          }
          m['builds'].insert(0, entry)
          # Keep last 20 staging builds in manifest
          m['builds'] = m['builds'][:20]
          json.dump(m, open('$MANIFEST', 'w'), indent=2)
          "

      - name: Commit and push manifest
        run: |
          git config user.name "github-actions[bot]"
          git config user.email "github-actions[bot]@users.noreply.github.com"
          git add docs/_static/firmware/staging-manifest.json
          git commit -m "ci: update staging manifest (${{ needs.publish.outputs.date }})"
          git push
```

**Key design decisions:**

- The `staging-release.yml` only triggers on pushes to the `staging` branch.
- Firmware is built with the **same reusable workflow** (`build-firmware.yml`)
  as production releases — same Docker image, same toolchain. This is the
  core guarantee: what users test is what gets released.
- Binaries go to **GitHub Releases** as pre-releases (marked with
  `prerelease: true`). They don't clutter the stable releases list and are
  clearly labeled.
- The manifest update commits to `dada-tbd-master` which triggers a docs
  redeploy. This means the staging flash page is always current.
- Manifest is capped at 20 entries to keep the JSON small.

### Stable release promotion

When a staging build passes testing, the engineer merges `staging` into
`dada-tbd-master`:

```bash
git checkout dada-tbd-master
git merge staging --no-ff -m "Release: promote staging-2026-03-20 to stable"
git tag v2026-03-20
git push origin dada-tbd-master --tags
```

This triggers `create-release.yml` which builds the **same code** again
(deterministic via Docker image) and creates a stable (non-pre-release)
GitHub Release. The `65_beta_channel.rst` page (renamed to "Stable Channel")
gets updated with the new build.

After promotion, reset staging:

```bash
git checkout staging
git reset --hard dada-tbd-master
git push --force-with-lease origin staging
```

### Flash page restructure

The current `docs/flash/` has 10 pages with **extreme code duplication** —
the same ~1,500 lines of ESP32-P4 flash logic, RP2350 Picoboot logic, OTA
partition parsing, MSC firmware switching, and SD card extraction are copy-
pasted across 5 different RST files (50, 60, 65, 66 and partially 25/30).
Every new feature, bug fix, or firmware version change requires editing
multiple files.

#### Current state (10 pages)

| File | Lines | Role | Problem |
|---|---|---|---|
| `index.rst` | 76 | Hub page | Links to dead-end pages |
| `25_flash_dsp.rst` | 273 | ESP32-P4 only flasher | Outdated firmware list, standalone tool |
| `30_flash_ui.rst` | 381 | RP2350 only flasher | Outdated firmware list, standalone tool |
| `50_device_recovery.rst` | 1,247 | 7-step guided recovery | 60% duplicated from 25 + 30 |
| `60_sd_card_recovery.rst` | 1,891 | 5-step SD card restore | Superseded by 65, still referenced |
| `65_beta_channel.rst` | 1,847 | Latest beta (2 paths) | 70% duplicated from 60 |
| `66_beta_channel_archive.rst` | 1,981 | Historical builds | 80% duplicated from 65 |
| `67_beta_troubleshooting.rst` | 103 | FAQ for beta flash | Scoped too narrowly |
| `68_update_updater.rst` | 100 | WebUI updater docs | Should merge with 70 |
| `70_webui_versions.rst` | 78 | WebUI version history | Should merge with 68 |

**Total:** ~8,000 lines, of which ~5,000 are duplicated JavaScript/HTML.

#### Target state (4 pages + shared JS module)

With no shipped product, there's no history worth archiving. The archive
page can be added later when there are real stable releases to list. For
now, the flash section needs exactly 5 pages:

| New file | Title | Content | Source |
|---|---|---|---|
| `index.rst` | **Flash & Updates** | Hub with CTAs (Stable, Staging, App Manager, WebUI) + secondary links (Troubleshooting) | Rewrite |
| `10_stable_channel.rst` | **Stable Channel** | Latest stable release from `dada-tbd-master`. Default flow: P4 firmware + Groovebox .uf2 + P4 SD card (same workflow as current `65_beta_channel.rst`). Link to App Manager for multi-app opt-in. Single hardcoded `ACTIVE_PKG`. | New (extract shared JS from `65_beta_channel.rst` logic) |
| `20_staging_channel.rst` | **Staging & Test Builds** | Pre-release builds from `staging` + feature test channels. Dynamic manifest loading (`staging-manifest.json`). Supports `?channel=<name>` for feature branches. Package selector dropdown. Warning banner. | New (manifest-driven) |
| `30_app_manager.rst` | **App Manager** | Interactive app management: browse catalog, install/remove apps via Picoboot WebUSB, sideload local .uf2 files, switch between multi-app and single-app mode. System tools (bootloader, flash nuke). | New |
| `40_webui_updates.rst` | **WebUI Updates** | How the on-device updater works + version history table + download links. One page for everything WebUI update related. | New (content from `68_update_updater.rst` + `70_webui_versions.rst`) |
| `50_troubleshooting.rst` | **Troubleshooting** | General flash troubleshooting: WebSerial issues, browser compat, Linux udev rules, SD card problems, recovery tips. | New (expanded from `67_beta_troubleshooting.rst`) |

> **No archive page yet.** Once there are 2+ stable releases worth
> preserving, add `60_archive.rst` with manifest-driven P4 SD Deploy only.

#### All old pages are deleted

Since there are no external users depending on current URLs:

```bash
git rm docs/flash/25_flash_dsp.rst
git rm docs/flash/30_flash_ui.rst
git rm docs/flash/50_device_recovery.rst
git rm docs/flash/60_sd_card_recovery.rst
git rm docs/flash/65_beta_channel.rst
git rm docs/flash/66_beta_channel_archive.rst
git rm docs/flash/67_beta_troubleshooting.rst
git rm docs/flash/68_update_updater.rst
git rm docs/flash/70_webui_versions.rst
```

The new pages are built **from scratch** using the shared `tbd-flasher.js`
module. No incremental evolution of old pages needed — the code duplication
problem is solved by simply not carrying it forward.

#### Shared JavaScript module: `docs/_static/js/tbd-flasher.js`

The core architectural change: extract the duplicated ~1,500 lines of flash
logic into a single shared ES module. Every flash page imports it instead of
inlining copies.

```javascript
// docs/_static/js/tbd-flasher.js
// Shared flash infrastructure for all TBD-16 documentation pages

export const FLASH_CONFIG = {
  TUSB_MSC_URL: '../_static/firmware/p4/tusb_msc.bin',
  OTA_DATA_ADDR: 0xd000,
  OTA_DATA_SIZE: 0x2000,
  PT_ADDR: 0x8000,
  PT_READ_SIZE: 0xC00,
  ESPTOOL_CDN: 'https://unpkg.com/esptool-js@0.5.7/bundle.js',
  PICOFLASH_URL: '../_static/picoflash/pkg/index.js',
  UF2_URL: '../_static/picoflash/js/uf2.js',
};

// ── Status & progress helpers ──
export function setStat(el, msg, cls) { /* ... */ }
export function showProg(wrap) { /* ... */ }
export function hideProg(wrap) { /* ... */ }

// ── Binary utilities ──
export function toBinStr(u8, chunkSize) { /* ... */ }

// ── CRC-32 (for OTA data) ──
export function espCrc32(buf) { /* ... */ }

// ── ESP-IDF partition table parsing ──
export function parsePartitionTable(data) { /* ... */ }
export function detectOta1Address(loader) { /* ... */ }

// ── OTA data builder ──
export function buildOtaData(slot) { /* ... */ }

// ── macOS cleanup (FAT32 ._ files) ──
export async function cleanMacOSFiles(dirHandle) { /* ... */ }

// ── ESP32-P4 flash workflow ──
export async function connectP4(statusCb) { /* ... */ }
export async function flashP4(loader, binUrl, progressCb) { /* ... */ }
export async function flashMscAndSwitch(loader, progressCb) { /* ... */ }

// ── RP2350 Picoboot flash workflow ──
export async function connectPico() { /* ... */ }
export async function flashPico(device, uf2Url, progressCb) { /* ... */ }

// ── SD card extraction workflow ──
export async function extractSdCard(dirHandle, zipUrl, hashUrl, progressCb, logCb) { /* ... */ }

// ── Manifest loading ──
export async function loadManifest(url) { /* ... */ }
```

Each flash page then becomes thin — just HTML layout + event wiring:

```javascript
// In 10_stable_channel.rst
import { connectP4, flashP4, connectPico, flashPico,
         extractSdCard, flashMscAndSwitch, loadManifest,
         setStat, showProg, hideProg } from '../_static/js/tbd-flasher.js';

var ACTIVE_PKG = {
  p4Url: '../_static/firmware/p4/possan-tbd-2026-03-19.bin',
  // ... (CI updates this single object on stable release)
};

// Wire up buttons to the shared functions
btnFlashP4.onclick = async () => {
  var loader = await connectP4(msg => setStat(statP4, msg, 'info'));
  await flashP4(loader, ACTIVE_PKG.p4Url, pct => { /* update progress */ });
};
```

**Benefits:**
- Bug fixes in flash logic apply to all pages at once
- New pages (like staging) are ~200 lines instead of ~2,000
- The shared module can be unit-tested independently
- Firmware URL changes only happen in one place per page (the PKG object)

#### Manifest-driven pages (staging + archive)

The staging channel and archive pages load their package lists from JSON
manifests at page load time, so **no RST edits are needed when new builds
are created**:

```javascript
// 20_staging_channel.rst — loads from manifest
import { loadManifest } from '../_static/js/tbd-flasher.js';

// Supports ?channel=<feature-name> for feature test builds
var params = new URLSearchParams(window.location.search);
var channel = params.get('channel') || 'staging';
var manifestUrl = channel === 'staging'
  ? '../_static/firmware/staging-manifest.json'
  : '../_static/firmware/feature-' + channel + '-manifest.json';

var manifest = await loadManifest(manifestUrl);
// Populate dropdown, wire up flash buttons...
```

```javascript
// 30_archive.rst — loads from stable manifest or hardcoded list
// Archive always requires Full SD Deploy (no Quick Update path)
// to guarantee firmware + SD card version match
```

#### Updated `index.rst` structure

```rst
Flash & Updates
===============

.. raw:: html

   <div class="dada-ctas">
     <div class="dada-cta">
       <h3>Stable Channel</h3>
       <p>Flash the latest tested firmware release.</p>
       <a href="10_stable_channel.html">Stable Channel →</a>
     </div>
     <div class="dada-cta">
       <h3>Staging & Test Builds</h3>
       <p>Try pre-release firmware built by CI.
       For beta testers and developers.</p>
       <a href="20_staging_channel.html">Staging Channel →</a>
     </div>
     <div class="dada-cta">
       <h3>WebUI Updates</h3>
       <p>Update the web interface without reflashing firmware.</p>
       <a href="40_webui_updates.html">WebUI Updates →</a>
     </div>
   </div>

   <div class="dada-secondary-links">
     <a href="50_troubleshooting.html">Troubleshooting</a>
   </div>

.. toctree::
   :hidden:

   10_stable_channel
   20_staging_channel
   40_webui_updates
   50_troubleshooting
```

#### Why remove standalone flashers (25, 30)

The current `25_flash_dsp.rst` (P4-only) and `30_flash_ui.rst` (Pico-only)
pages let users flash one chip without the other. This is a **UX hazard**:

- Flashing only the P4 with new firmware while the RP2350 runs old firmware
  (or vice versa) causes SPI protocol mismatches, sequencer failures, and
  confusing crash behavior that looks like a hardware defect.
- The SD card image may not match the firmware version, causing WebUI
  conflicts or missing presets.
- Users who find these pages via search don't understand they need both chips.

The stable and staging pages **already include all flash steps** (P4 + Pico +
SD card) in a guided workflow. The standalone pages become redundant and
harmful.

> **Escape hatch:** If an engineer needs to flash just one chip during
> development, they can use `esptool.py` or Picoboot CLI directly. These are
> developer tools, not end-user pages.

#### Future: Version Archive page

Once there are 2+ stable releases (e.g. v0.1.0 and v0.2.0), add
`30_archive.rst` to the flash section. The archive page should:

- Load a manifest of all stable releases (from GitHub Release API or a
  committed JSON file)
- **Only offer Full SD Deploy** (no Quick Update) — old firmware versions are
  only guaranteed to work with their matching SD card image
- Use the same shared `tbd-flasher.js` module

This is not needed yet — build it when there's history worth archiving.

#### Build from scratch

With no users depending on current flash page URLs, all new pages are built
from scratch using the shared `tbd-flasher.js` module. All old RST files
are deleted in one commit. No incremental migration needed.

**Result:** 10 pages → 4 pages + 1 shared JS module. ~8,000 lines → ~2,500
lines (estimated). Every page uses the same flash infrastructure. New builds
require zero RST edits (manifest-driven for staging, single PKG object for
stable).

### Experimental branches (ad-hoc testing)

For truly experimental work (e.g. `nevvkid/dada-tbd-master-experimental`
which is 788 commits ahead), engineers can:

1. **Merge into `staging`** when ready for user testing
2. Or create a **separate one-off pre-release** manually:

```bash
# Tag from any branch → triggers build workflow
git tag staging-experimental-2026-03-20
git push origin staging-experimental-2026-03-20
```

The CI workflow can be extended to also trigger on `staging-*` tags, creating
a pre-release that appears in the manifest.

### Feature branch testing channel

Sometimes a developer is building a completely new feature that may **never**
land in `staging` or `dada-tbd-master` — but still needs real users to test
it on actual hardware. Examples:

- A new DSP plugin architecture that rewrites the audio pipeline
- An experimental sequencer mode that may be scrapped
- A hardware driver for a prototype board revision

These shouldn't pollute the `staging` branch or its manifest. Instead, they
get their own isolated testing channel using the **same CI infrastructure**.

#### How it works

Any branch prefixed with `feature-test/` triggers the same firmware build
workflow and publishes to a **separate manifest** scoped to that feature:

```
feature-test/new-sequencer     → feature-new-sequencer-manifest.json
feature-test/audio-pipeline-v2 → feature-audio-pipeline-v2-manifest.json
```

The naming convention is:
- Branch: `feature-test/<feature-name>`
- Release tag: `feature-<feature-name>-2026-03-20`
- Manifest: `docs/_static/firmware/feature-<feature-name>-manifest.json`

#### Workflow extension: `feature-test-release.yml`

```yaml
# .github/workflows/feature-test-release.yml
name: Feature Test Release

on:
  push:
    branches:
      - 'feature-test/**'

permissions:
  contents: write

jobs:
  build:
    uses: ./.github/workflows/build-firmware.yml
    with:
      build_name: "tbd-${{ github.ref_name }}"

  publish:
    needs: build
    runs-on: ubuntu-latest
    steps:
      - name: Derive feature name and tag
        id: meta
        run: |
          # feature-test/new-sequencer → new-sequencer
          FEATURE="${GITHUB_REF_NAME#feature-test/}"
          DATE=$(date -u +%Y-%m-%d)
          SHORT_SHA=${GITHUB_SHA::7}
          echo "feature=${FEATURE}" >> "$GITHUB_OUTPUT"
          echo "tag=feature-${FEATURE}-${DATE}" >> "$GITHUB_OUTPUT"
          echo "name=dada-tbd-16-feature-${FEATURE}-${DATE}" >> "$GITHUB_OUTPUT"
          echo "date=${DATE}" >> "$GITHUB_OUTPUT"
          echo "sha=${SHORT_SHA}" >> "$GITHUB_OUTPUT"
          echo "manifest=feature-${FEATURE}-manifest.json" >> "$GITHUB_OUTPUT"

      - name: Download firmware artifacts
        uses: actions/download-artifact@v4
        with:
          path: release-assets/
          merge-multiple: true

      - name: Rename artifacts
        run: |
          NAME="${{ steps.meta.outputs.name }}"
          mv release-assets/*-unified.bin "release-assets/${NAME}.bin" 2>/dev/null || \
          mv release-assets/ctag-tbd.bin "release-assets/${NAME}.bin"
          if ls release-assets/*.uf2 2>/dev/null; then
            mv release-assets/*.uf2 "release-assets/${NAME}.uf2"
          fi

      - name: Create GitHub pre-release
        uses: softprops/action-gh-release@v2
        with:
          tag_name: ${{ steps.meta.outputs.tag }}
          name: "Feature Test: ${{ steps.meta.outputs.feature }} (${{ steps.meta.outputs.date }})"
          prerelease: true
          body: |
            **Feature branch:** `${{ github.ref_name }}`
            **Commit:** `${{ steps.meta.outputs.sha }}`

            This is a feature test build — not part of the staging or stable channel.
          files: |
            release-assets/${{ steps.meta.outputs.name }}.bin
            release-assets/${{ steps.meta.outputs.name }}.uf2
            release-assets/${{ steps.meta.outputs.name }}-p4sd.zip
            release-assets/${{ steps.meta.outputs.name }}-p4sd-hash.txt

  update-manifest:
    needs: publish
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
        with:
          ref: dada-tbd-master
          token: ${{ secrets.GITHUB_TOKEN }}

      - name: Update feature manifest
        env:
          FEATURE: ${{ needs.publish.outputs.feature }}
          TAG: ${{ needs.publish.outputs.tag }}
          NAME: ${{ needs.publish.outputs.name }}
          DATE: ${{ needs.publish.outputs.date }}
          SHA: ${{ needs.publish.outputs.sha }}
          MANIFEST_FILE: ${{ needs.publish.outputs.manifest }}
        run: |
          MANIFEST="docs/_static/firmware/${MANIFEST_FILE}"
          BASE_URL="https://github.com/dadamachines/ctag-tbd/releases/download/${TAG}"
          BRANCH="${GITHUB_REF_NAME}"

          python3 -c "
          import json, os
          from datetime import datetime, timezone
          mf = os.environ['MANIFEST']
          if os.path.exists(mf):
              m = json.load(open(mf))
          else:
              m = {'channel': 'feature', 'feature': os.environ['FEATURE'], 'updated': '', 'builds': []}
          m['updated'] = datetime.now(timezone.utc).isoformat()
          entry = {
              'name': os.environ['NAME'],
              'date': os.environ['DATE'],
              'branch': os.environ['BRANCH'],
              'commit': os.environ['SHA'],
              'p4Url': os.environ['BASE_URL'] + '/' + os.environ['NAME'] + '.bin',
              'picoUrl': os.environ['BASE_URL'] + '/' + os.environ['NAME'] + '.uf2',
              'p4sdUrl': os.environ['BASE_URL'] + '/' + os.environ['NAME'] + '-p4sd.zip',
              'p4sdHashUrl': os.environ['BASE_URL'] + '/' + os.environ['NAME'] + '-p4sd-hash.txt',
          }
          m['builds'].insert(0, entry)
          m['builds'] = m['builds'][:10]  # keep last 10 builds per feature
          json.dump(m, open(mf, 'w'), indent=2)
          "

      - name: Commit and push manifest
        run: |
          git config user.name "github-actions[bot]"
          git config user.email "github-actions[bot]@users.noreply.github.com"
          git add "docs/_static/firmware/${{ needs.publish.outputs.manifest }}"
          git commit -m "ci: update feature manifest ${{ needs.publish.outputs.feature }} (${{ needs.publish.outputs.date }})"
          git push
```

#### Flash page: dynamic feature channel

The staging flash page (`20_staging_channel.rst`) can double as the feature
test page by accepting a `?channel=<feature-name>` query parameter:

```javascript
// At the top of the staging page's <script>
var params = new URLSearchParams(window.location.search);
var channel = params.get('channel') || 'staging';
var manifestUrl = channel === 'staging'
  ? '../_static/firmware/staging-manifest.json'
  : '../_static/firmware/feature-' + channel + '-manifest.json';
```

This means a developer can share a single URL with testers:

```
https://dadamachines.github.io/ctag-tbd/flash/20_staging_channel.html?channel=new-sequencer
```

The page title and warning banner adapt:

```javascript
if (channel !== 'staging') {
  document.getElementById('channelTitle').textContent =
    'Feature Test: ' + channel;
  document.getElementById('channelWarning').textContent =
    'This is an experimental feature build. It may be unstable and is not ' +
    'part of the regular release pipeline.';
}
```

No separate RST page needed per feature — one page serves all channels.

#### Lifecycle

1. Developer creates `feature-test/my-thing` branch
2. Pushes trigger CI → firmware built → pre-release created → manifest updated
3. Developer shares the flash URL with testers
4. When the feature is ready, it gets merged into `staging` (the normal path)
   or abandoned
5. **Cleanup:** When a feature branch is deleted, its manifest and pre-
   releases can be cleaned up manually, or left to expire. Feature manifests
   are capped at 10 builds each so they stay small.

#### When to use which channel

| Channel | Branch | Use case | Audience |
|---|---|---|---|
| **Stable** | `dada-tbd-master` | Tested, ready for all users | All customers |
| **Staging** | `staging` | Release candidate, broad testing | Beta testers |
| **Feature** | `feature-test/*` | Isolated experiment, may be scrapped | Specific testers, invited by developer |

### Summary of the release flow

```
feature branches (on personal forks)
        │
        ├──────────────────────────────────────┐
        │ PR / merge                           │ feature-test/* branch
        ▼                                      ▼
    staging branch                      feature test channel
        │                               (isolated pre-releases,
        │ CI builds → pre-release       per-feature manifest,
        │           → manifest update   shared flash page with
        │           → staging flash     ?channel= parameter)
        │
        │ users test via browser flash
        │
        │ merge --no-ff + tag
        ▼
    dada-tbd-master
        │
        │ CI builds → stable release on GitHub Releases
        │           → docs redeploy → stable flash page
        ▼
    users get stable firmware
```

### CORS note for GitHub Release downloads

GitHub Release assets served from `github.com/…/releases/download/…` support
CORS for browser `fetch()`. The flash page's JavaScript can download
`.bin`/`.uf2`/`.zip` files directly from release URLs without a proxy. This
has been verified and is used by many ESP web flasher projects.

> **If CORS becomes an issue:** An alternative is to use a GitHub Pages
> redirect or store the manifest URLs pointing to a CDN/proxy. But this
> should not be needed — GitHub has supported CORS on release assets since
> 2020.

---

## 9. Versioning Strategy — Firmware, WebUI, and Releases

### Current state: no firmware versions, decoupled WebUI

Today the firmware has **no version number** — builds are identified by
date-based names (`possan-tbd-2026-03-19`). At compile time, `CMakeLists.txt`
runs `git describe --tags --abbrev=4` and bakes whatever it returns into
`version.hpp` as `TBD_FW_VERSION`. Since there are no semantic version tags
on `dada-tbd-master`, the version string is essentially a git hash.

The WebUI has its **own independent version** in
`sdcard_image/data/webui-version.json` (currently `0.3.5`), bumped manually
by the engineer calling `create_webui_update.sh`.

The two are completely decoupled — there is no enforcement that firmware
version X requires WebUI version Y. The SD card image bakes in whatever
WebUI version exists at archive time, but WebUI updates can be applied
independently afterward, potentially creating version mismatches.

### The problem for users

- A user flashes stable firmware `possan-tbd-2026-03-19` but the WebUI on
  their SD card is from `0.3.1`. The macro designer crashes because the
  firmware's REST API changed. There's no way to know the versions don't
  match.
- A user sees "firmware: v1.0.0-5-g1234abc" and "WebUI: 0.3.5" in the
  device config tab. These numbers have no visible relationship. Which goes
  with which?
- The staging flash page shows build `dada-tbd-16-staging-2026-03-20`. Is that
  compatible with WebUI 0.3.5? Nobody knows without asking the engineer.

### Proposed versioning scheme: unified MAJOR.MINOR

Align firmware and WebUI under a **shared MAJOR.MINOR** version, with
independent PATCH numbers:

```
Release version:  0.4
                   │ │
                   │ └── MINOR — increments on each stable release
                   └──── MAJOR — increments on breaking platform changes

Firmware:         0.4.0 → 0.4.1 → 0.4.2   (P4 + Pico, from git tags)
WebUI:            0.4.0 → 0.4.1 → 0.4.3   (from webui-version.json)

Compatibility:    Firmware 0.4.x works with any WebUI 0.4.x
                  Firmware 0.5.x requires WebUI 0.5.x (breaking API change)
```

#### Rules

1. **MAJOR.MINOR is the "release train"** — firmware and WebUI share it.
   When you bump the minor (0.3 → 0.4), both firmware and WebUI get new
   versions.

2. **PATCH is independent** — firmware can be at `0.4.2` while WebUI is at
   `0.4.5`. They are developed at different speeds. A WebUI bug fix doesn't
   require a firmware release.

3. **Any firmware `0.X.*` works with any WebUI `0.X.*`** — within the same
   minor, the REST API contract is stable. Bug fixes, UI tweaks, and new
   presets don't break the API.

4. **A MINOR bump means potential API changes** — if the firmware adds new
   REST routes, changes JSON response formats, or restructures the SPI
   protocol, bump MINOR for both. Old WebUI won't work with the new firmware.

5. **MAJOR stays at 0 until the product ships v1** — this is standard for
   products still in active development. The first "golden master" release
   becomes `1.0.0`.

#### What this looks like for users

| Where | What they see | Example |
|-------|---------------|---------|
| Device config tab | `FW: 0.4.2 / WebUI: 0.4.5` | Clear that both are "0.4" compatible |
| Stable flash page | "Release 0.4 — Firmware 0.4.2 + WebUI 0.4.5" | One release name |
| Staging flash page | "Staging 0.5-dev (2026-03-20)" | Pre-release, version TBD |
| Archive page | Dropdown: "0.4", "0.3", "0.2" | One entry per minor release |
| WebUI updater | "Installed: 0.4.3 → Available: 0.4.5" | Compatible, safe to update |
| WebUI updater | "Installed: 0.3.9 → Available: 0.4.0" | Shows warning: firmware update also required |

### Implementation

#### Git tags for firmware versioning

Replace date-based build names with semantic version tags on
`dada-tbd-master`:

```bash
git tag v0.4.0 -m "Release 0.4.0 — USB NCM fix, PicoSeqRack, macro system"
git push origin v0.4.0
```

`CMakeLists.txt` already runs `git describe --tags --always --abbrev=4`.
With a tag like `v0.4.0`, the firmware version becomes:

| Scenario | `git describe` output | `TBD_FW_VERSION` |
|---|---|---|
| Tagged commit (stable release) | `v0.4.0` | `v0.4.0` |
| 3 commits after tag (staging) | `v0.4.0-3-gabc1` | `v0.4.0-3-gabc1` |
| No tags reachable (CI on fork) | `abc1234` | `abc1234` (with `--always`) |

Users see `v0.4.0` for stable, `v0.4.0-3-gabc1` for staging (clearly a
pre-release), and a bare hash for untagged local builds.

#### WebUI version alignment

When bumping for a new minor release:

```bash
# webui-version.json — start of the 0.4 train
{ "version": "0.4.0", "date": "2026-04-01", "description": "..." }
```

When doing WebUI-only patches within the train:

```bash
# webui-version.json — patch within 0.4
{ "version": "0.4.1", "date": "2026-04-05", "description": "..." }
```

The MAJOR.MINOR **must match** the current firmware release train. The PATCH
can differ.

#### Compatibility check in the WebUI updater

The updater page (`webui-update.html`) can enforce compatibility:

```javascript
// Compare MAJOR.MINOR of installed firmware vs available WebUI update
var fwVersion = capabilities.FWV;  // e.g. "v0.4.2" from /api/v2/device
var updateVersion = manifest.version;  // e.g. "0.4.5" from latest.json

var fwMajorMinor = fwVersion.replace(/^v/, '').split('.').slice(0, 2).join('.');
var uiMajorMinor = updateVersion.split('.').slice(0, 2).join('.');

if (fwMajorMinor !== uiMajorMinor) {
  showWarning('This WebUI update (' + updateVersion + ') requires firmware ' +
    uiMajorMinor + '.x. Your firmware is ' + fwVersion + '. ' +
    'Please update your firmware first via the Stable Channel page.');
}
```

This is a **soft warning**, not a hard block — advanced users can override.
But most users will understand "your firmware is 0.3, this WebUI needs 0.4,
update firmware first."

#### SD card image versioning

The P4 SD card archive (`dada-tbd-16-p4sd.zip`) includes the WebUI at a
specific version. When a user does a P4 SD Deploy, they get a matched set:

| Release | Firmware | WebUI in SD image | Result |
|---|---|---|---|
| 0.4 | `v0.4.2` | `0.4.5` | Firmware + WebUI matched, guaranteed working |
| Archive: 0.3 | `v0.3.8` | `0.3.9` | Old but matched, guaranteed working |

The optional Pico SD curated bundle (see Section 10) is versioned per
MAJOR.MINOR train — one bundle per `0.4.x`, one per `0.5.x`, etc. It is
not released with every patch.

The **Quick Update** path (firmware flash + WiFi WebUI update) also ensures
matching: the flash page points users to the correct WebUI update for that
firmware version, or the WebUI updater auto-detects.

#### Staging and feature build versions

Staging builds don't get stable version tags — they show the commit distance:

```
v0.4.0-15-g1a2b3c4     ← 15 commits after the 0.4.0 tag
```

This is automatic from `git describe`. Users see this on the staging flash
page and in the device config tab, making it obvious they're running a
pre-release.

Feature test builds may have no version tag at all — `git describe --always`
falls back to a bare hash. The flash page labels them with the branch name
and date.

#### Release manifest with version metadata

The staging and stable manifests include version info:

```json
{
  "builds": [
    {
      "name": "dada-tbd-16-staging-2026-03-20",
      "firmwareVersion": "v0.4.0-15-g1a2b3c4",
      "webuiVersion": "0.4.5",
      "compatRange": "0.4",
      "date": "2026-03-20",
      ...
    }
  ]
}
```

The flash page can display: "Firmware v0.4.0-15-g1a2b, WebUI 0.4.5,
compatible with any 0.4.x WebUI."

### How flash pages communicate versioning

| Page | What users see |
|---|---|
| **Stable Channel** | "**Release 0.4** — Firmware v0.4.2 + WebUI 0.4.5. This is the latest tested release. The Quick Update will flash both firmware chips and update the WebUI to 0.4.5. The Full P4 SD Deploy includes a matched SD card image." |
| **Staging Channel** | "**Staging (0.4-dev)** — Pre-release firmware v0.4.0-15-g1a2b built by CI. Your current WebUI 0.4.x is compatible. If this build bumps to 0.5, you'll need the included P4 SD card image (Full Deploy)." |
| **Archive** | "**Release 0.3** — Firmware v0.3.8 + WebUI 0.3.9. Full SD Deploy only — this ensures the firmware and WebUI versions match. Quick Update is not available for archive releases." |
| **WebUI Updates** | "**WebUI 0.4.5** — Compatible with firmware 0.4.x. If your firmware is older (0.3.x), update firmware first via the Stable Channel." Version history table shows all 0.4.x and 0.3.x releases grouped by compatibility. |

### Timeline for adoption

This versioning scheme aligns with the CI migration:

1. **Phase 1 (CI builds):** Keep date-based names. CI uses `git describe`
   which already outputs the right format if tags exist.
2. **Phase 4 (flash page restructure):** Add version display and
   compatibility info to flash pages. Add the `compatRange` field to
   manifests.
3. **First tagged release:** Tag `v0.4.0` on `dada-tbd-master`. Bump
   `webui-version.json` to `0.4.0`. From this point, all builds on
   `dada-tbd-master` automatically get `v0.4.x` version strings.
4. **WebUI updater enhancement:** Add the MAJOR.MINOR compatibility check.
   Soft warning only.

### Starting version: 0.4.0

The current WebUI is at `0.3.5`. With only ~10 test users and no shipped
product, there's no real legacy to protect — but 2+ years of firmware
development shouldn't look like a v0.1 to the public either. **Start at
0.4.0.**

This is the natural continuation of the existing WebUI numbering. The 0.3.x
WebUI versions become the implicit "pre-versioning" era. Starting at 0.4.0
signals maturity without claiming a finished product (that's what 1.0 is
for).

- `v0.4.0` — first versioned release (current feature set, aligned firmware + WebUI)
- `v0.5.0` — next release with breaking API changes
- `v1.0.0` — first "golden master" shipped product

```bash
# One-time: start the new versioning scheme
git tag v0.4.0 dada-tbd-master -m "First versioned release"
git push origin v0.4.0

# Update WebUI version
echo '{"version":"0.4.0","date":"2026-03-20","description":"First versioned release"}' \
  > sdcard_image/data/webui-version.json
```

---

## 10. Hardware Targets, Multi-Target Naming & App Distribution

### Hardware targets

The dadamachines product line has three tiers, each sharing the ESP32-P4
DSP core but differing in enclosure, IO, and co-processors:

| Target | Slug | Description | Processors |
|--------|------|-------------|------------|
| **TBD-16** | `tbd-16` | Full desktop instrument — enclosure, UI board (30 buttons, 4 pots, OLED), audio codec, MIDI DIN | ESP32-P4 + RP2350B + ESP32-C6 |
| **TBD-Core** | `tbd-core` | Developer module — 30-pin FFC connector, no built-in UI board, bring-your-own interface | ESP32-P4 + RP2350B + ESP32-C6 |
| **Custom Integration** | `custom` | OEM-level PCB integration — bare ESP32-P4, custom codec, custom IO | ESP32-P4 only |

TBD-16 and TBD-Core share **identical processors** (P4 + RP2350B + C6).
The difference is the UI board: TBD-16 includes a built-in UI board with
an STM32F030R8T6 I/O controller, 30 buttons, 4 pots, and an OLED display.
TBD-Core exposes the same 30-pin FFC connector but ships without a UI
board — you design your own.

> **Important: two separate SD cards.** Each device has **two** micro SD
> slots — one for the ESP32-P4 and one for the RP2350. They serve different
> purposes:
>
> | SD Card | Contains | Managed by |
> |---------|----------|------------|
> | **P4 SD** | WebUI (`www/`), presets (`data/`), synth definitions, sample ROM (`tbdsamples/`) | P4 firmware, WiFi WebUI updater |
> | **Pico SD** | UF2 bootloader (`BOOT2350.uf2`), apps (`tbd-apps/*.uf2`), app user data | RP2350 bootloader, USB MSC app, or physical card reader |
>
> The RP2350 also has its own **USB-C port** (USB-C #2 on the TBD-16),
> used for web-based firmware flashing, BOOTSEL mode, and USB MSC access.

Each target gets its own firmware binary. The P4 firmware is the same
codebase but built with different `sdkconfig` flags (see Section 7,
`build-firmware.yml` matrix). What differs:

- **TBD-16:** RP2350 SPI bridge enabled, UI command handlers, macro system.
  Ships with Groovebox as default app on the Pico SD card. The built-in
  UI board supports apps that need buttons/pots/OLED (Groovebox, Multi FX).
- **TBD-Core:** Same P4 firmware with RP2350 SPI bridge enabled. No built-in
  UI board, so apps that require TBD-16's hardware UI (Groovebox, Multi FX)
  won't work — but apps that use a custom UI board or are headless (MCL,
  USB MSC, Debug Probe) will. Ships with a minimal app set on Pico SD.
- **Custom:** Bare-bones P4 build. No WiFi (no C6), no RP2350. Audio codec
  driver configurable (TLV320AIC3254 vs external I2S).

### Artifact naming convention

Every build artifact follows a strict naming pattern:

```
dada-{target}[-{app}]-{version}.{ext}
```

| Component | Values | Example |
|-----------|--------|---------|
| `dada` | Always present — brand prefix | `dada-` |
| `{target}` | `tbd-16`, `tbd-core`, `custom` | `dada-tbd-16-` |
| `{app}` | Optional — RP2350 app name (only for `.uf2` files) | `dada-tbd-16-groovebox-` |
| `{version}` | Semver tag or staging/feature date | `v0.4.0`, `staging-2026-03-20` |
| `{ext}` | `bin` (P4), `uf2` (Pico), `zip` (SD), `txt` (hash) | `.bin` |

#### Complete artifact set per target

**TBD-16 release (v0.4.0) — standard (4 files):**
```
dada-tbd-16-v0.4.0.bin              ← P4 firmware (unified OTA image)
dada-tbd-16-groovebox-v0.4.0.uf2   ← Default RP2350 app (standalone, no bootloader)
dada-tbd-16-p4sd-v0.4.0.zip        ← P4 SD card image (WebUI + presets + sample ROM)
dada-tbd-16-p4sd-v0.4.0-hash.txt   ← SHA-256 of P4 SD zip
```

This is the same workflow as the existing `65_beta_channel.rst` page:
P4 firmware + Groovebox .uf2 + P4 SD card. Updates are simple — flash the
new versions of each. The Pico SD card (ships pre-loaded on the device)
is not touched during standard updates.

**TBD-Core release (v0.4.0) — standard (4 files):**
```
dada-tbd-core-v0.4.0.bin               ← P4 firmware (same codebase, RP2350 enabled)
dada-tbd-core-usb-msc-v0.4.0.uf2       ← Utility app (SD card access)
dada-tbd-core-p4sd-v0.4.0.zip          ← P4 SD card image (WebUI + presets + samples)
dada-tbd-core-p4sd-v0.4.0-hash.txt     ← SHA-256 of P4 SD zip
```

TBD-Core ships with USB MSC as the default RP2350 app. Apps like
Groovebox, Multi FX, MCL, and Debug Probe are available for download
from the App Manager page (see below).

**Optional Pico SD curated bundle (once per MAJOR.MINOR train):**
```
dada-tbd-16-picosd-v0.4.zip        ← Pico SD image (BOOT2350 + all default apps)
dada-tbd-core-picosd-v0.4.zip      ← Same, headless app set for TBD-Core
```

These are **not** part of standard releases. The TBD-16 device ships with
its Pico SD card pre-loaded at the factory. The downloadable bundle is a
convenience for users who need to restore or update their Pico SD card
(e.g., after a MAJOR.MINOR version change that requires new app builds,
or if the card was accidentally wiped). Ships once per MAJOR.MINOR train.

**TBD-16 staging build (4 files):**
```
dada-tbd-16-staging-2026-03-20.bin
dada-tbd-16-groovebox-staging-2026-03-20.uf2
dada-tbd-16-p4sd.zip
dada-tbd-16-p4sd-hash.txt
```

### CI build matrix

Extend the `build-firmware.yml` reusable workflow with a hardware target
matrix:

```yaml
# In build-firmware.yml
jobs:
  build:
    strategy:
      matrix:
        target:
          - slug: tbd-16
            sdkconfig: sdkconfig.tbd16
            default_app: groovebox
          - slug: tbd-core
            sdkconfig: sdkconfig.tbd-core
            default_app: usb-msc
    steps:
      - name: Build P4 firmware
        run: |
          cp ${{ matrix.target.sdkconfig }} sdkconfig
          idf.py build

      - name: Upload P4 artifact
        uses: actions/upload-artifact@v4
        with:
          name: firmware-dada-${{ matrix.target.slug }}-p4
          path: build/ctag-tbd.bin

      - name: Upload default Pico app
        uses: actions/upload-artifact@v4
        with:
          name: firmware-dada-${{ matrix.target.slug }}-pico
          path: build/${{ matrix.target.default_app }}.uf2
```

The release workflow then downloads artifacts from all matrix legs and
renames them following the naming convention.

### RP2350 app ecosystem

#### Architecture

RP2350 apps live in **separate repositories owned by different people**:

| App | Repo | Maintainer | License |
|-----|------|------------|---------|
| Groovebox | `possan/tbd-pico-seq3` (or dadamachines fork) | Per-Olov Jernberg (@possan) | — |
| Multi FX | `dadamachines/rp2350-arduino-tbd-fw` | dadamachines (internal) | — |
| MCL | `jmamma/MCL` | Justin Mammarella (@jmamma) | GPL |
| Debug Probe | `dadamachines/rp2350-arduino-tbd-fw` | dadamachines (internal) | — |
| USB MSC | `dadamachines/rp2350-arduino-tbd-fw` | dadamachines (internal) | — |
| UI Test | `dadamachines/rp2350-arduino-tbd-fw` | dadamachines (internal) | — |
| Game | `dadamachines/rp2350-arduino-tbd-fw` | dadamachines (internal) | — |
| MIDI Controller | TBD | dadamachines (planned) | — |

All apps are built with PlatformIO + Arduino using the TBD RP2350 template
library. The P4 firmware repository does NOT build Pico apps — it only
references them at Pico SD card archive time.

**Key point: all RP2350 apps share the same P4 firmware.** The Groovebox,
Multi FX, MCL, and every other app all run against the identical P4 binary.
There is no per-app P4 firmware variant — the P4 runs the DSP engine and
exposes the SPI protocol; the RP2350 app decides what to do with it. This
means a single P4 firmware release (`dada-tbd-16-v0.4.0.bin`) is compatible
with all RP2350 apps tagged for the same MAJOR.MINOR version.

#### The default experience

The TBD-16 **physically ships** with a Pico SD card pre-loaded with all
apps (Groovebox, Multi FX, MCL, USB MSC, Debug Probe, UI Test) and the
`BOOT2350.uf2` bootloader. However, **the default RP2350 firmware is the
Groovebox flashed standalone** — no bootloader, no boot menu. This keeps
the update flow simple and identical to the existing `65_beta_channel.rst`
workflow:

1. Flash the P4 firmware (`dada-tbd-16-v0.4.0.bin`)
2. Flash the Groovebox app (`dada-tbd-16-groovebox-v0.4.0.uf2`) to the
   RP2350 via USB-C port #2
3. Deploy the P4 SD card (`dada-tbd-16-p4sd-v0.4.0.zip`) — or update the
   WebUI via the existing WebUI updater

That's it. The user has a working Groovebox synthesizer. Updates are
equally simple — just repeat steps 1–3 with the new release. No Pico SD
card management, no bootloader, no multi-app complexity.

**Multi-app is one step away.** Because the Pico SD card ships pre-loaded
with all apps, the user only needs to flash the bootloader to unlock
multi-app mode (see Paths below). The apps are already there, waiting.

**Pre-loaded Pico SD card contents (factory):**

| File | Location | Purpose |
|------|----------|---------|
| `BOOT2350.uf2` | SD card root | UF2 bootloader (dormant until bootloader firmware is flashed) |
| `groovebox.uf2` | `tbd-apps/` | Default app (same as standalone, for boot menu) |
| `multi_fx.uf2` | `tbd-apps/` | Official multi-effect |
| `mcl.uf2` | `tbd-apps/` | Partner sequencer (jmamma) |
| `tusb_msc.uf2` | `tbd-apps/` | Utility: USB mass storage |
| `dbg_prb.uf2` | `tbd-apps/` | Utility: CMSIS-DAP debug probe |
| `ui_test.uf2` | `tbd-apps/` | Utility: hardware interaction test |

| Component | Default flow (Groovebox) | Multi-app flow (opt-in) |
|-----------|-------------------------|------------------------|
| P4 firmware | Flash via stable page | Same |
| RP2350 app | Groovebox .uf2 (standalone) | Flash bootloader → boot menu |
| P4 SD card | Deploy or WebUI update | Same |
| Pico SD card | Ships pre-loaded, not touched | Already set up — just works |

#### App boot flow

The RP2350 supports two operating modes:

**Standalone mode (default):**

The Groovebox `.uf2` is flashed directly into RP2350 flash. It runs
immediately on power-up — no bootloader, no SD card dependency, no boot
menu. This is the factory default and the easiest to update (just flash
the new `.uf2`).

**Multi-app mode (opt-in):**

When the user flashes `bootloader_pico2.uf2` to the RP2350 via BOOTSEL
mode, the bootloader activates. It loads `BOOT2350.uf2` from the Pico SD
card, which in turn manages the boot menu:

1. Checks SD card at `/tbd-apps/*.uf2`
2. Remembers the last loaded app
3. Holding **Page Up** at boot → shows boot menu to select between apps
4. Without interaction → loads the last app automatically

Since the Pico SD card ships pre-loaded with all default apps, enabling
multi-app mode is a single step — flash the bootloader and reboot.

This means app switching is a user-initiated action at boot time, not
firmware-controlled. The P4 firmware has **no knowledge** of which Pico app
is running — they communicate over SPI but there's no app identification
command (see AnnounceApp section below).

**Switching between modes:**

| From → To | Steps |
|-----------|-------|
| Standalone → Multi-app | Flash `bootloader_pico2.uf2` via BOOTSEL. Pico SD card already has `BOOT2350.uf2` + apps. |
| Multi-app → Standalone | Flash `flash_nuke.uf2` via BOOTSEL (erases bootloader), then flash desired app `.uf2` via BOOTSEL. |

Both `bootloader_pico2.uf2` and `flash_nuke.uf2` are available on the
App Manager page. Source repos:
[uf2loader](https://github.com/ctag-fh-kiel/uf2loader),
[pico-universal-flash-nuke](https://github.com/Gadgetoid/pico-universal-flash-nuke).

**Two bootloader files — important distinction:**

| File | Size | Where it lives | Purpose |
|------|------|----------------|---------|
| `bootloader_pico2.uf2` | 14 KB | Flashed into RP2350 flash via BOOTSEL | The actual bootloader firmware |
| `BOOT2350.uf2` | 82 KB | Pico SD card root (`/BOOT2350.uf2`) | UF2 bootloader app loaded by the flash bootloader |

The flash bootloader (`bootloader_pico2.uf2`) loads and executes
`BOOT2350.uf2` from the SD card, which in turn manages the boot menu
and app selection. Both are required for multi-app mode.

### App Registry: `dadamachines/tbd-app-registry`

The central question is: how do we collect binaries from multiple repos
owned by different people into a single curated release bundle?

**Solution: a dedicated app registry repository** that acts as the
coordination layer between app developers and the TBD-16 release process.

```
dadamachines/tbd-app-registry
├── apps/
│   ├── groovebox/
│   │   ├── manifest.json         ← app metadata + download URLs
│   │   ├── README.md             ← user-facing docs (rendered on website)
│   │   └── icon.png              ← 128×128 app icon (boot menu / web catalog)
│   ├── multi-fx/
│   │   ├── manifest.json
│   │   ├── README.md
│   │   └── icon.png
│   ├── mcl/
│   │   ├── manifest.json
│   │   ├── README.md
│   │   └── icon.png
│   ├── tusb-msc/
│   │   ├── manifest.json
│   │   └── icon.png
│   ├── dbg-prb/
│   │   ├── manifest.json
│   │   └── icon.png
│   ├── ui-test/
│   │   ├── manifest.json
│   │   └── icon.png
│   └── game/
│       ├── manifest.json
│       └── icon.png
├── system-tools/
│   ├── bootloader/
│   │   └── manifest.json         ← bootloader_pico2.uf2 + BOOT2350.uf2
│   └── flash-nuke/
│       └── manifest.json         ← flash_nuke.uf2 from Gadgetoid
├── bundles/
│   ├── tbd-16-v0.4.0.json       ← "what ships with TBD-16 v0.4.0"
│   └── tbd-16-v0.5.0.json
├── schema/
│   ├── manifest.schema.json      ← JSON schema for app manifests
│   └── bundle.schema.json        ← JSON schema for bundle definitions
├── .github/
│   └── workflows/
│       ├── validate-pr.yml       ← CI: validate manifest + test download
│       └── build-bundle.yml      ← CI: assemble SD card app bundle
└── README.md                     ← contributor guide
```

#### App manifest format

Each app has a `manifest.json` that describes where to find the binary and
what it's compatible with:

```json
{
  "id": "groovebox",
  "name": "Groovebox",
  "description": "16-track drum machine and sequencer",
  "author": {
    "name": "Per-Olov Jernberg",
    "github": "possan",
    "url": "https://possan.codes/"
  },
  "repo": "https://github.com/possan/tbd-pico-seq3",
  "license": "MIT",
  "category": "instrument",
  "tags": ["sequencer", "drums", "midi", "link"],
  "sdFilename": "groovebox.uf2",
  "releases": [
    {
      "version": "0.4.0",
      "firmwareCompat": "0.4",
      "date": "2026-03-20",
      "downloadUrl": "https://github.com/possan/tbd-pico-seq3/releases/download/v0.4.0/groovebox.uf2",
      "sha256": "a1b2c3d4...",
      "size": 524288,
      "changelog": "Initial release for TBD-16 v0.4.0"
    }
  ]
}
```

Key fields:

| Field | Purpose |
|-------|---------|
| `id` | Machine-readable slug, matches folder name |
| `sdFilename` | What the file is called on the SD card (`/tbd-apps/groovebox.uf2`) |
| `releases[].firmwareCompat` | MAJOR.MINOR — which firmware train this works with |
| `releases[].downloadUrl` | Direct link to `.uf2` on the app's own GitHub Releases |
| `releases[].sha256` | Integrity check — CI verifies this on bundle assembly |
| `category` | `instrument`, `utility`, `effect`, `sequencer` |

**The actual .uf2 binaries are NOT stored in the registry repo.** They live
as GitHub Release assets on each app's own repository. The registry only
stores metadata + download pointers. This keeps the registry lightweight
and lets app developers own their build/release process.

#### Bundle definition format

A bundle file pins exactly which app versions ship with a given firmware
release:

```json
{
  "target": "tbd-16",
  "firmwareVersion": "0.4.0",
  "firmwareCompat": "0.4",
  "description": "Default app bundle for TBD-16 v0.4.0",
  "bootloader": {
    "flash": {
      "downloadUrl": "https://github.com/ctag-fh-kiel/uf2loader/releases/download/v1.1/bootloader_pico2.uf2",
      "sha256": "...",
      "description": "Flash bootloader (flash to RP2350 via BOOTSEL)"
    },
    "sdcard": {
      "downloadUrl": "https://github.com/ctag-fh-kiel/uf2loader/releases/download/v1.1/BOOT2350.uf2",
      "sha256": "...",
      "description": "UF2 bootloader app (copy to Pico SD card root)"
    }
  },
  "apps": [
    { "id": "groovebox", "version": "0.4.0", "default": true },
    { "id": "multi-fx",  "version": "0.4.0" },
    { "id": "mcl",       "version": "0.4.0" },
    { "id": "tusb-msc",  "version": "0.4.0", "alwaysInstalled": true },
    { "id": "dbg-prb",   "version": "0.4.0", "alwaysInstalled": true },
    { "id": "ui-test",   "version": "0.4.0", "alwaysInstalled": true }
  ]
}
```

The `"default": true` flag marks Groovebox as the app that boots on first
power-up (the bootloader remembers the last app — on a fresh SD card, this
is the first entry). Apps with `"alwaysInstalled": true` are utilities that
ship with every bundle and are pre-selected on the App Manager page.

#### External contributor workflow

How does an external developer like @jmamma get their MCL app into the
TBD-16 ecosystem?

```
┌─────────────────────────────────────────────────────────┐
│  App Developer (e.g. jmamma/MCL)                        │
│  1. Build app using rp2350-arduino-tbd-fw template      │
│  2. Test on TBD-16 hardware                             │
│  3. Create GitHub Release with mcl.uf2 attached         │
│  4. Note the SHA-256 of the .uf2 file                   │
└─────────────────────┬───────────────────────────────────┘
                      │
                      ▼  PR to tbd-app-registry
┌─────────────────────────────────────────────────────────┐
│  tbd-app-registry PR                                     │
│  Add/update: apps/mcl/manifest.json                      │
│              apps/mcl/README.md                           │
│              apps/mcl/icon.png                            │
│                                                          │
│  CI runs automatically:                                  │
│  ✓ Validate manifest.json against schema                 │
│  ✓ Download .uf2 from the release URL                    │
│  ✓ Verify SHA-256 matches                                │
│  ✓ Check file size is reasonable (< 2 MB)                │
│  ✓ Verify firmwareCompat range is valid                  │
└─────────────────────┬───────────────────────────────────┘
                      │
                      ▼  dadamachines reviews & merges
┌─────────────────────────────────────────────────────────┐
│  App is now in the catalog                               │
│  - Listed on the website app directory                   │
│  - Available for individual download on flash page       │
│  - Eligible for inclusion in official bundles            │
└─────────────────────────────────────────────────────────┘
```

For an **app update** (new version), the contributor opens a PR that adds a
new entry to the `releases[]` array in their manifest. The CI re-validates.

#### App tiers

Not all apps have the same status. The registry distinguishes:

| Tier | Meaning | Bundle eligibility | Review bar |
|------|---------|-------------------|------------|
| **Official** | Built/maintained by dadamachines | Always in default bundle | Internal QA |
| **Official (utility)** | Utilities: USB MSC, Debug Probe, UI Test | Always installed, cannot be removed | Internal QA |
| **Partner** | External, tested on TBD-16, endorsed | Eligible for default bundle | dadamachines tests on hardware |
| **Community** | External, self-published | Install via App Manager only | Manifest validates, download works |
| **System** | Bootloader, flash nuke — mode-switching tools | Not in app bundle, listed as system tools | Internal |

MCL is a **Partner** app — maintained by @jmamma, tested by dadamachines,
ships in the default TBD-16 bundle. A random community developer's
experimental synth would be **Community** tier — available to install via
the App Manager but not in the box.

The tier is a field in the manifest:

```json
{ "tier": "partner" }
```

For utility apps, an additional flag makes them non-removable:

```json
{ "tier": "official", "alwaysInstalled": true }
```

Only `official` and `partner` apps can be referenced in bundle definitions.
`community` apps appear in the App Manager catalog but users install them
individually. `system` tools are listed separately on the App Manager page
for mode switching (see bootloader / flash nuke above).

#### Sideloading — running your own apps without the registry

The app registry and App Manager catalog are *one* way to distribute apps,
but they are **not required**. Any developer can build a `.uf2` from the
`rp2350-arduino-tbd-fw` template and run it on their TBD-16 without ever
publishing to the registry, submitting a PR, or interacting with
dadamachines. This is "sideloading" — the open, zero-friction path for
developers who want to experiment, prototype, or keep their work private.

**Why this matters:** The TBD-16 is a platform for creative developers.
Many users will write RP2350 apps that are personal tools, one-off
experiments, live performance setups, or proprietary projects. The system
must make this as easy as possible — the registry is for *sharing*, not
for *running*.

**Sideloading in standalone mode (no bootloader):**

The simplest path. No multi-app, no bootloader, no Pico SD card involved:

1. Build your app with PlatformIO (`pio run`)
2. Enter BOOTSEL mode on USB-C port #2 (hold right front button)
3. Drag-and-drop `firmware.uf2` onto the USB drive, or `pio run -t upload`
4. Device reboots — your app is running

This is how the Groovebox ships by default (standalone `.uf2` on the
RP2350 flash). Any `.uf2` can replace it the same way. To go back, just
flash the Groovebox `.uf2` or any other app via BOOTSEL.

**Sideloading in multi-app mode (with bootloader):**

If the user has enabled multi-app, sideloaded apps live alongside
registry apps on the Pico SD card:

1. Build your app with PlatformIO (`pio run`)
2. Flash USB MSC on the RP2350 (from the App Manager or boot menu)
3. Copy your `.uf2` to `tbd-apps/` on the Pico SD card
4. Eject and reboot — your app appears in the boot menu

That's it. No manifest, no SHA-256, no PR, no review. The bootloader
picks up any `.uf2` file in `tbd-apps/` regardless of where it came from.

**Sideloading via the App Manager page:**

The App Manager page should also support local file upload as an
alternative to the catalog:

```
═══════════════════════════════════════════════
  Install Local App
  ─────────────────────────────────────────────
  Have a .uf2 you built yourself?
  [Choose File]  my-synth.uf2
  [Install to Pico SD]

  The app will be copied to tbd-apps/ on the
  Pico SD card. No registry or catalog needed.
═══════════════════════════════════════════════
```

This uses the same USB MSC + File System Access API mechanism as catalog
installs — just with a local file instead of a download URL.

**Sideloading via PlatformIO (developer workflow):**

For developers iterating rapidly, the PlatformIO upload target can flash
directly to the RP2350 via USB:

```bash
# Standalone mode — flash to RP2350 flash directly
pio run -t upload

# Multi-app mode — copy to Pico SD card (requires USB MSC running)
cp .pio/build/pico2/firmware.uf2 /Volumes/PICOSD/tbd-apps/my-app.uf2
```

The `docs/apps/getting-started.rst` guide covers the full PlatformIO
setup, build process, and project structure for building custom apps
against the `rp2350-arduino-tbd-fw` template.

**What sideloaded apps can do:**

Sideloaded apps have full access to the same SPI API as registry apps.
They can:

- Control any DSP plugin on the P4 via the SPI command interface
- Read/write the P4's macro and preset system
- Access the OLED display, buttons, encoders, LEDs, and MIDI
- Use the full Arduino ecosystem (libraries, USB stack, etc.)

There is **no capability difference** between a sideloaded app and a
registry app. The only difference is distribution — registry apps are
discoverable in the App Manager catalog; sideloaded apps are not.

**Version warnings for sideloaded apps:**

Sideloaded apps won't appear in the `app-catalog.json`, so the App
Manager page can't check their `firmwareCompat`. This is fine — the
developer who built the app knows what firmware they're targeting.

If the app implements AnnounceApp (includes name and version in the SPI
handshake), the P4 will still show the app identity in the WebUI. If
the app doesn't implement AnnounceApp, the WebUI shows "Unknown app" —
which is perfectly acceptable for development.

**Sideloading → registry (optional graduation):**

If a developer decides to share their app, they can publish it to the
registry at any time:

1. Create a GitHub Release with the `.uf2` attached
2. Open a PR to `tbd-app-registry` with a `manifest.json`
3. CI validates → dadamachines reviews → app appears in the catalog

This is optional. Sideloading is a fully supported, first-class workflow
— not a workaround or unsupported hack.

### SD card images

As described in the dual SD card callout above, the TBD-16 and TBD-Core
have **two physically separate micro SD cards**. The P4 SD card image
(WebUI + presets + samples) is part of every standard release. The Pico
SD card ships **factory pre-loaded** with the bootloader and default app
bundle — users never need to touch it for the standard Groovebox
experience. An optional downloadable Pico SD bundle is available for
restore or update scenarios (once per MAJOR.MINOR train).

#### P4 SD card archive — `dada-tbd-16-p4sd-v0.4.0.zip`

Built by `create_sd_archive.sh` (existing script, no changes needed):

```
dada-tbd-16-p4sd-v0.4.0.zip
├── data/
│   ├── spm-config.json
│   ├── trackdefaults.json
│   └── sp/                   ← synth definitions
├── www/                      ← WebUI (gzipped)
├── tbdsamples/               ← factory audio samples
└── dbup/                     ← backup folder (pre-created)
```

This archive does NOT contain any `.uf2` files — those belong on the Pico SD.

#### Pico SD card image — `dada-tbd-16-picosd-v0.4.zip`

The Pico SD card ships **factory pre-loaded** on TBD-16 devices. Users
never need to download or deploy this for the standard Groovebox setup.
The Pico SD contents are dormant until the user enables multi-app mode
by flashing the bootloader.

Factory contents:

```
Pico SD card (factory pre-loaded)
├── BOOT2350.uf2              ← UF2 bootloader (loaded by flash bootloader)
└── tbd-apps/
    ├── groovebox.uf2         ← default app (boots first on fresh install)
    ├── multi_fx.uf2          ← official
    ├── mcl.uf2               ← partner (jmamma)
    ├── tusb_msc.uf2          ← utility (USB mass storage)
    ├── dbg_prb.uf2           ← utility (CMSIS-DAP debug probe)
    └── ui_test.uf2           ← utility (hardware interaction test)
```

An **optional downloadable bundle** (`dada-tbd-16-picosd-v0.4.zip`) is
available for restore or update scenarios:

- **MAJOR.MINOR firmware change** — app binaries may need rebuilding for
  a new firmware train (e.g., `v0.4` → `v0.5`)
- **Wiped Pico SD card** — user accidentally formatted the card
- **TBD-Core users** — TBD-Core does not ship with a pre-loaded Pico SD

The bundle is published once per MAJOR.MINOR train (not every PATCH
release). It is assembled from the app registry (see below).

#### Bundle assembly workflow

The Pico SD curated bundle is built by a **separate workflow**, not part of
the standard release pipeline. It runs once per MAJOR.MINOR train and is
triggered manually (or by a registry update).

```yaml
# build-picosd-bundle.yml (dadamachines/ctag-tbd) — separate from release CI
name: Build Pico SD Curated Bundle
on:
  workflow_dispatch:
    inputs:
      train:
        description: 'MAJOR.MINOR train (e.g. 0.4)'
        required: true
      target:
        description: 'Hardware target'
        required: true
        default: 'tbd-16'
        type: choice
        options: [tbd-16, tbd-core]

jobs:
  bundle-pico-sd:
    runs-on: ubuntu-latest
    steps:
      - name: Check out firmware repo
        uses: actions/checkout@v4

      - name: Check out app registry
        uses: actions/checkout@v4
        with:
          repository: dadamachines/tbd-app-registry
          path: app-registry

      - name: Read bundle definition
        id: bundle
        run: |
          TRAIN="${{ inputs.train }}"
          TARGET="${{ inputs.target }}"
          BUNDLE="app-registry/bundles/${TARGET}-v${TRAIN}.json"

          if [ ! -f "$BUNDLE" ]; then
            echo "::error::No bundle definition at ${BUNDLE}"
            exit 1
          fi
          echo "bundle=${BUNDLE}" >> "$GITHUB_OUTPUT"

      - name: Download bootloader and bundled apps
        run: |
          mkdir -p picosd-staging/tbd-apps
          BUNDLE="${{ steps.bundle.outputs.bundle }}"

          # Download bootloader
          BOOT_URL=$(jq -r '.bootloader.downloadUrl' "$BUNDLE")
          BOOT_SHA=$(jq -r '.bootloader.sha256' "$BUNDLE")
          curl -fsSL -o picosd-staging/BOOT2350.uf2 "$BOOT_URL"
          echo "${BOOT_SHA}  picosd-staging/BOOT2350.uf2" | sha256sum -c -

          # Download each app
          for APP_ID in $(jq -r '.apps[].id' "$BUNDLE"); do
            APP_VER=$(jq -r ".apps[] | select(.id==\"${APP_ID}\") | .version" "$BUNDLE")
            MANIFEST="app-registry/apps/${APP_ID}/manifest.json"

            DL_URL=$(jq -r ".releases[] | select(.version==\"${APP_VER}\") | .downloadUrl" "$MANIFEST")
            DL_SHA=$(jq -r ".releases[] | select(.version==\"${APP_VER}\") | .sha256" "$MANIFEST")
            SD_NAME=$(jq -r '.sdFilename' "$MANIFEST")

            echo "Downloading ${APP_ID} v${APP_VER} → tbd-apps/${SD_NAME}"
            curl -fsSL -o "picosd-staging/tbd-apps/${SD_NAME}" "$DL_URL"
            echo "${DL_SHA}  picosd-staging/tbd-apps/${SD_NAME}" | sha256sum -c -
          done

      - name: Build Pico SD curated bundle
        run: |
          TARGET="${{ inputs.target }}"
          TRAIN="${{ inputs.train }}"
          cd picosd-staging
          echo "${TRAIN}" > bundle-version.txt
          zip -r "../dada-${TARGET}-picosd-v${TRAIN}.zip" .

      - name: Create GitHub Release
        uses: softprops/action-gh-release@v2
        with:
          tag_name: "picosd-${{ inputs.target }}-v${{ inputs.train }}"
          name: "Pico SD Bundle — ${{ inputs.target }} v${{ inputs.train }}"
          body: |
            Pico SD card bundle for the v${{ inputs.train }}.x firmware train.
            Contains BOOT2350.uf2 + all default apps (Groovebox, Multi FX,
            MCL, USB MSC, Debug Probe, UI Test). Extract to the Pico SD card.
          files: dada-${{ inputs.target }}-picosd-v${{ inputs.train }}.zip
```

This ensures every `.uf2` in the bundle is:
1. **Pinned** to a specific version (from the bundle JSON)
2. **Integrity-verified** (SHA-256 check on download)
3. **Sourced from the original repo** (app dev controls their own release)

#### How this changes `create_sd_archive.sh`

`create_sd_archive.sh` continues to build the **P4 SD card** archive only
(data/, www/, tbdsamples/). No changes needed — it correctly handles P4 SD
content.

The Pico SD curated bundle is a completely separate artifact built by the
`build-picosd-bundle.yml` workflow above, triggered manually once per
MAJOR.MINOR train. It does not affect the standard release pipeline.

### App versioning

#### The problem

Today, RP2350 apps have **no version numbers**. The `.uf2` files on the
SD card are just `groovebox.uf2` — no version embedded in the filename
or binary metadata. This creates problems:

1. **No compatibility tracking** — which Groovebox version works with
   firmware v0.4.2? Nobody knows without testing.
2. **No update detection** — users can't tell if their on-device app is
   outdated.
3. **No rollback** — if an app update breaks, there's no way to identify
   what version was installed before.

#### Solution: version tagging in each app's repo

App developers tag releases in their own repository:

```bash
# In possan/tbd-pico-seq3
git tag v0.4.0 -m "Compatible with TBD-16 firmware 0.4.x"
git push origin v0.4.0

# Create GitHub Release, attach groovebox.uf2
gh release create v0.4.0 --title "Groovebox v0.4.0" \
  .pio/build/rp2350/firmware.uf2#groovebox.uf2
```

They then update their manifest in `tbd-app-registry` with the new
release entry (version, download URL, SHA-256, firmware compat range).

#### Compatibility rule

Same MAJOR.MINOR rule from Section 9:

```
Firmware 0.4.x ↔ Any app tagged 0.4.x
Firmware 0.5.x ↔ Any app tagged 0.5.x (SPI protocol changed)
```

When a firmware release bumps MINOR, all bundled apps must be rebuilt and
tagged with the new MINOR. App PATCH versions can diverge (Groovebox at
0.4.3 while MCL is at 0.4.1), but the MAJOR.MINOR guarantees SPI
compatibility.

#### On-device filenames stay simple

The Pico SD card always has `groovebox.uf2`, `mcl.uf2`, etc. — short names
for the boot menu. The version is tracked in:
- The app registry manifest (what was downloaded)
- The `announceApp` SPI response (runtime, see below)
- The release download filename (`dada-tbd-16-groovebox-v0.4.0.uf2`)

### User experience: managing apps

Users interact with RP2350 apps at three levels:

#### Path 1: Default setup (recommended — Groovebox standalone)

The standard workflow from the **Stable Channel** flash page, identical to
the existing `65_beta_channel.rst` flow:

1. **Flash P4 firmware** (`dada-tbd-16-v0.4.0.bin`) via the web flasher
2. **Flash Groovebox** (`dada-tbd-16-groovebox-v0.4.0.uf2`) to the RP2350
   via USB-C port #2
3. **Deploy P4 SD card** (`dada-tbd-16-p4sd-v0.4.0.zip`) — extract to the
   P4 SD card, or use the WebUI updater to update just the WebUI

Done. The TBD-16 boots Groovebox automatically. No Pico SD card setup, no
app selection, no bootloader — the .uf2 runs standalone on the RP2350.

Updates work the same way: flash the new P4 firmware, flash the new
Groovebox .uf2, update the P4 SD card. Simple and predictable.

#### Path 2: Enable multi-app (one step)

The TBD-16 ships with the Pico SD card pre-loaded with all default apps.
To unlock multi-app mode, the user visits the App Manager page:

1. **Visit the App Manager page** on the docs site
2. The page detects (or asks for) the P4 firmware version
3. **Enter BOOTSEL mode** on USB-C port #2 (hold right front button)
4. **Flash `bootloader_pico2.uf2`** via the Picoboot web flasher
5. **Reboot** — multi-app is active

**If firmware is newer than the Pico SD apps** (e.g., firmware `0.5.x`
but Pico SD has `0.4.x` apps): the page shows a warning banner
recommending a Pico SD update and offers a guided update flow. But the
user is **never blocked** — they can always proceed with "Flash
Bootloader" and use their existing apps. Some apps (like MCL) may work
fine across firmware trains; others may not. The warning makes the risk
visible without dictating what the user can do.

The RP2350 bootloader loads `BOOT2350.uf2` from the Pico SD card,
detects all apps in `tbd-apps/`, and shows the boot menu. Hold **Page Up**
at boot to select.

The default app in multi-app mode is still **Groovebox** (it boots first
on a fresh setup since the bootloader remembers the last-used app).

#### Path 3: App Manager — browse, install, update

The **App Manager** page (`docs/flash/30_app_manager.rst`) is an
interactive browser-based tool for managing the RP2350 app collection.
It serves several purposes:

**Enable / disable multi-app mode:**
- Flash `bootloader_pico2.uf2` to enable multi-app (requires BOOTSEL mode)
- Flash `flash_nuke.uf2` to erase the bootloader and return to standalone
  mode (then flash desired app .uf2 directly)

**Browse and install apps:**
1. Connect USB-C port #2 — flash USB MSC if needed to expose the Pico SD
   card
2. The page reads the Pico SD contents and compares installed app versions
   against the detected firmware train — outdated apps are flagged
3. Browse the **App Catalog** — available apps loaded from
   `app-catalog.json`, filtered by firmware version
4. Select apps via checkboxes — utilities (USB MSC, Debug Probe, UI Test)
   are pre-selected by default
5. **Update outdated apps** — if any installed apps don't match the
   firmware train, an "Update All" button replaces them in one click
6. Install new apps to `tbd-apps/` on the Pico SD card
7. Eject and reboot — the bootloader picks up the new/updated apps

**Download system tools:**

| Tool | Purpose | How to use |
|------|---------|-----------|
| `bootloader_pico2.uf2` | Enable multi-app mode | Flash via BOOTSEL mode |
| `flash_nuke.uf2` | Return to standalone mode | Flash via BOOTSEL mode |
| `BOOT2350.uf2` | UF2 bootloader for Pico SD card root | Copy to SD card root (if missing) |

**Download Pico SD bundle:**

If the user needs to restore or update their Pico SD card (e.g., after a
MAJOR.MINOR firmware change), the optional curated bundle is available:

- Download `dada-tbd-16-picosd-v0.4.zip`
- Extract to the Pico SD card (via USB MSC or card reader)
- Contains: `BOOT2350.uf2` + all default apps in `tbd-apps/`

The catalog section loads `app-catalog.json` (generated by registry CI)
and renders a browsable, interactive grid:

```json
{
  "apps": [
    {
      "id": "groovebox",
      "name": "Groovebox",
      "description": "16-track drum machine and sequencer",
      "author": "Per-Olov Jernberg (@possan)",
      "category": "instrument",
      "tier": "official",
      "latestVersion": "0.4.0",
      "firmwareCompat": "0.4",
      "downloadUrl": "https://github.com/possan/tbd-pico-seq3/releases/download/v0.4.0/groovebox.uf2",
      "iconUrl": "https://dadamachines.github.io/tbd-app-registry/apps/groovebox/icon.png",
      "docsUrl": "https://dadamachines.github.io/ctag-tbd/apps/groovebox.html"
    },
    {
      "id": "mcl",
      "name": "MegaCommand Live",
      "description": "MIDI sequencer for Elektron gear",
      "author": "Justin Mammarella (@jmamma)",
      "category": "sequencer",
      "tier": "partner",
      "latestVersion": "0.4.0",
      "firmwareCompat": "0.4",
      "downloadUrl": "https://github.com/jmamma/MCL/releases/download/v0.4.0-tbd/mcl.uf2",
      "iconUrl": "https://dadamachines.github.io/tbd-app-registry/apps/mcl/icon.png",
      "docsUrl": "https://dadamachines.github.io/ctag-tbd/apps/mcl.html"
    },
    {
      "id": "tusb-msc",
      "name": "USB Mass Storage",
      "description": "Exposes Pico SD card as USB drive",
      "author": "dadamachines",
      "category": "utility",
      "tier": "official",
      "alwaysInstalled": true,
      "latestVersion": "0.4.0",
      "firmwareCompat": "0.4",
      "downloadUrl": "https://github.com/dadamachines/rp2350-arduino-tbd-fw/releases/download/v0.4.0/tusb_msc.uf2"
    },
    {
      "id": "dbg-prb",
      "name": "Debug Probe",
      "description": "CMSIS-DAP debug bridge for OpenOCD",
      "author": "dadamachines",
      "category": "utility",
      "tier": "official",
      "alwaysInstalled": true,
      "latestVersion": "0.4.0",
      "firmwareCompat": "0.4",
      "downloadUrl": "https://github.com/dadamachines/rp2350-arduino-tbd-fw/releases/download/v0.4.0/dbg_prb.uf2"
    },
    {
      "id": "ui-test",
      "name": "UI Test",
      "description": "Hardware interaction and display test",
      "author": "dadamachines",
      "category": "utility",
      "tier": "official",
      "alwaysInstalled": true,
      "latestVersion": "0.4.0",
      "firmwareCompat": "0.4",
      "downloadUrl": "https://github.com/dadamachines/rp2350-arduino-tbd-fw/releases/download/v0.4.0/ui_test.uf2"
    }
  ],
  "systemTools": [
    {
      "id": "bootloader",
      "name": "RP2350 Bootloader",
      "description": "Enable multi-app mode (flash via BOOTSEL)",
      "downloadUrl": "https://github.com/ctag-fh-kiel/uf2loader/releases/download/v1.1/bootloader_pico2.uf2",
      "flashMode": "bootsel"
    },
    {
      "id": "flash-nuke",
      "name": "Flash Nuke",
      "description": "Return to standalone mode — erases RP2350 flash (flash via BOOTSEL)",
      "downloadUrl": "https://github.com/Gadgetoid/pico-universal-flash-nuke/releases/latest/download/flash_nuke.uf2",
      "flashMode": "bootsel"
    },
    {
      "id": "boot2350",
      "name": "BOOT2350.uf2",
      "description": "UF2 bootloader for Pico SD card root (if missing from SD card)",
      "downloadUrl": "https://github.com/ctag-fh-kiel/uf2loader/releases/download/v1.1/BOOT2350.uf2",
      "flashMode": "sdcard-root"
    }
  ]
}
```

#### Path 4: Sideloading — build and run your own apps

For developers who build their own RP2350 apps using the
`rp2350-arduino-tbd-fw` template. No registry, no catalog, no PR needed.

**Standalone mode** (simplest — no bootloader):
1. Build with PlatformIO: `pio run`
2. Enter BOOTSEL on USB-C port #2 (hold right front button)
3. `pio run -t upload` or drag-and-drop `.uf2` onto the USB drive
4. Device reboots — your app is running

**Multi-app mode** (alongside other apps):
1. Build with PlatformIO: `pio run`
2. Flash USB MSC on the RP2350 (from boot menu or App Manager)
3. Copy `.uf2` to `tbd-apps/` on the Pico SD card
4. Eject and reboot — your app appears in the boot menu

**Via the App Manager page** ("Install Local App" section):
1. Connect USB-C port #2 with USB MSC running
2. Click "Choose File" and select your local `.uf2`
3. Click "Install to Pico SD" — file is copied to `tbd-apps/`

Sideloaded apps have **full access** to the same SPI API, display,
buttons, encoders, LEDs, and MIDI as registry apps. There is no
capability difference. See `docs/apps/getting-started.rst` for the
complete PlatformIO setup and project structure.

If a developer later wants to share their app, they can optionally
publish it to the `tbd-app-registry` — but this is never required.

### AnnounceApp: P4 ↔ RP2350 app identification

#### Current state

An `announceApp` feature **already exists** on the experimental branch
(`nevvkid/ctag-tbd`, branch `dada-tbd-master-experimental`). It uses the
SPI protocol to communicate to the P4 which RP2350 app is active. This is
the foundation — but it needs to be formalized and extended to deliver a
good user experience.

On the current `dada-tbd-master` branch in this workspace, the feature has
not been merged yet. The SPI protocol (`SpiAPI.hpp`) defines 60+ command
types but does not yet include the announceApp command.

#### What works today (experimental branch)

The RP2350 sends its identity to the P4 over SPI after boot. The P4 knows
which app is running.

#### What still needs improvement

To deliver the best UX to users, the announceApp mechanism should be
extended with:

1. **App version reporting** — the RP2350 should include its version string
   (from `git describe`) so the P4 can expose it via the REST API and WebUI.
   Today the SPI payload may only include the app name.

2. **Capability flags** — include what the app supports (e.g., `seq` for
   sequencer, `midi` for MIDI output, `dsp` for audio processing) so the
   macro system and WebUI can adapt their UI to the active app.

3. **REST API exposure** — extend the `/api/v2/device` response to include
   the active Pico app info:
   ```json
   {
     "HWV": "dada",
     "FWV": "v0.4.2",
     "picoApp": "groovebox",
     "picoVersion": "0.4.1",
     "picoCaps": ["seq", "midi"]
   }
   ```

4. **WebUI integration** — display active app name and version in the
   device info panel. Show a warning if the app's MAJOR.MINOR doesn't match
   the firmware's MAJOR.MINOR.

5. **Compatibility awareness** — if the app reports version `0.3.x` but
   the firmware is `0.4.x`, the WebUI can show a dismissible notice:
   "Groovebox v0.3.x hasn't been tested with firmware v0.4.x. An update
   may be available on the App Manager page." The app still runs — this
   is purely informational.

#### Formalization plan

When merging `announceApp` from experimental into `dada-tbd-master`:

- Assign a stable SPI command ID (e.g., `0x1A`) and document it in
  `SpiAPI.hpp`.
- Define the payload structure (app name, version, capabilities).
- Add the P4-side handler that stores app identity in a struct accessible
  to `DeviceAPI.cpp`.
- Wire it into the REST API and WebUI.

This should land **after** the CI pipeline and versioning scheme are
established (Phase 2), so the version strings have meaning from day one.

### Firmware ↔ app compatibility enforcement

#### The problem

The TBD-16 ships at v0.4.0 with a factory-loaded Pico SD card containing
apps built for the `0.4` firmware train. Over time the user updates their
P4 firmware, P4 SD card, and standalone Groovebox to `v0.5.0`. If they
later activate multi-app mode, the Pico SD card still has v0.4.x apps —
and those apps may not work with the v0.5.x P4 backend because the SPI
protocol, macro system, or hardware API changed between trains.

This is an inevitable consequence of the "Groovebox standalone by default,
multi-app opt-in later" model. The gap between "when the device shipped"
and "when the user enables multi-app" can be weeks, months, or multiple
firmware trains.

#### The guideline

**MAJOR.MINOR should match between P4 firmware and RP2350 apps.**
An app built for `0.4.x` is *tested* against firmware `0.4.x`. If the
user upgrades to firmware `0.5.x`, the Pico SD apps *should* be updated
to `0.5.x` builds — but some apps may still work fine with a mismatch.
Apps that are self-contained (e.g., MCL does mostly MIDI and doesn't
heavily use the P4 backend) are less likely to break than apps that
depend on SPI commands, the macro system, or WebUI integration.

PATCH can always diverge — `groovebox 0.4.3` works with `firmware 0.4.0`.

The system **warns but never blocks**. The user always has the final say.

#### Where enforcement happens — three layers

**Layer 1: App Manager page (browser-side, primary guard)**

The App Manager is the primary path to enable multi-app and manage apps.
This is where version awareness lives. The flow:

1. User visits the App Manager page
2. Page asks or detects the P4 firmware version (via WebSerial to
   `/api/v2/device`, or manual dropdown if not connected)
3. Page compares firmware MAJOR.MINOR against the Pico SD bundle
   MAJOR.MINOR that shipped with the device
4. **If mismatch** → the page shows a warning banner recommending a
   Pico SD update, with a guided update flow. The user can update
   first or proceed anyway:

```
═══════════════════════════════════════════════
  ⚠ Pico SD Update Recommended
  ─────────────────────────────────────────────
  Your firmware is v0.5.0, but the Pico SD
  apps were built for v0.4.x. Some apps may
  not work correctly with the newer firmware.

  Recommended: update the Pico SD card first:
  1. [Flash USB MSC]  (exposes Pico SD card)
  2. [Download Pico SD Bundle v0.5]
  3. Extract to the Pico SD card
  4. Eject and reconnect

  [Update Pico SD First]   [Skip — Proceed Anyway]
═══════════════════════════════════════════════
```

5. **If match** → no warning, activation proceeds directly

**Ongoing use — after multi-app is already active:**

The version check isn't only for first-time activation. Every time a user
visits the App Manager page (whether multi-app is active or not), the
page performs the same check:

1. Detect firmware version (WebSerial or manual selection)
2. Read Pico SD contents via USB MSC + File System Access API
3. Read `bundle-version.txt` for the overall train, **and** check each
   `.uf2` in `tbd-apps/` against `app-catalog.json` to determine
   individual app versions (filename matching or embedded metadata)
4. Flag every app whose `firmwareCompat` doesn't match the firmware train

The user can **update individual apps, update all at once, or ignore the
warnings entirely**. Apps that aren't tagged for the current firmware
train may still work — the warning is informational.

**Scenario: user updates P4 firmware from 0.4.x → 0.5.0, multi-app
already active:**

```
═══════════════════════════════════════════════
  App Manager — TBD-16
  ─────────────────────────────────────────────
  Firmware: v0.5.0 (train 0.5)

  ⚠ 4 apps not yet tagged for firmware 0.5
  ─────────────────────────────────────────────
  These apps were built for firmware 0.4.x.
  Updated versions may be available. Apps may
  still work — update recommended, not required.

  ⚠ Groovebox v0.4.0      → v0.5.0 available  [Update]
  ⚠ Multi FX v0.4.0       → v0.5.0 available  [Update]
  ⚠ MCL v0.4.0            (no 0.5 build yet)  [Keep]
  ⚠ UI Test v0.4.0        → v0.5.0 available  [Update]
  ✓ USB MSC v0.5.0        (up to date)
  ✓ Debug Probe v0.5.0    (up to date)

  [Update All Available]   [Dismiss Warnings]
  [Download Pico SD Bundle v0.5]  ← or replace all
═══════════════════════════════════════════════
```

The **"Update All"** button downloads the matching `.uf2` files from
`app-catalog.json` and writes them to the Pico SD card (via USB MSC +
File System Access API), replacing the outdated versions in place.
Utilities marked `alwaysInstalled` are included automatically.

Alternatively the user can download the full Pico SD bundle and extract
it manually — same result, just a different workflow.

After updating, the user ejects and reboots. All apps now match the
firmware train and work correctly.

This is a **soft warning**, not a hard block. The "Flash Bootloader"
button is always enabled — the user can always proceed. Some apps may work
fine across firmware trains (e.g., MCL is mostly self-contained MIDI and
may not depend on the P4 backend at all). We never want to block a user
from using an app that might work. The warning makes the risk visible and
the recommended action clear, but the decision is always the user's.

**How the page knows the firmware version:**

- **Primary:** WebSerial connection to P4 via USB-C port #1 → query
  `/api/v2/device` → read `FWV` field. Same mechanism the Stable
  Channel page already uses for firmware flashing.
- **Fallback:** Manual dropdown ("What firmware version are you running?")
  pre-populated with known release versions. This works even if the
  user only has USB-C port #2 connected.

**How the page knows the Pico SD app versions:**

- **Primary:** If USB MSC is running, the page can read the Pico SD
  card contents via File System Access API and check filenames or a
  `bundle-version.txt` marker file placed by the bundle build.
- **Fallback:** The page asks "Is this a freshly shipped device?" and
  shows what firmware train the device shipped with (from a lookup
  table of ship dates → firmware versions).

**Layer 2: P4 runtime (AnnounceApp, secondary guard)**

Once AnnounceApp is formalized (Phase 2+), the P4 firmware gets a
runtime compatibility check:

1. RP2350 app boots and sends `AnnounceApp` via SPI, including app
   name and version (e.g., `groovebox 0.4.1`)
2. P4 compares `app MAJOR.MINOR` against `firmware MAJOR.MINOR`
3. If mismatch → P4 sets a `picoAppCompat: false` flag in the device
   state, exposed via REST API
4. WebUI shows a persistent warning banner:
   ```
   ⚠ Groovebox v0.4.1 is not compatible with firmware v0.5.0.
   Update your Pico SD apps → App Manager
   ```
5. The P4 **never blocks** the app from running. The warning is purely
   informational — the app may still work fine (especially self-contained
   apps like MCL that mostly do MIDI and don't depend heavily on the P4
   backend). The user can always dismiss the banner or ignore it.

This catches cases where the user bypasses the App Manager (e.g.,
manually copies a `bootloader_pico2.uf2` to RP2350 via BOOTSEL without
visiting the page). It also serves as ongoing monitoring — if an app
is working fine despite a version mismatch, the user can safely ignore
the warning.

**Layer 3: Bootloader (optional, future)**

The UF2 bootloader (`BOOT2350.uf2`) could optionally read a firmware
version hint from a marker file on the Pico SD card and **visually flag**
mismatched apps in the boot menu (e.g., show a `⚠` icon next to the app
name). The bootloader should **never skip or hide** apps — the user always
sees all installed apps and can boot any of them.

- Requires a `firmware-version.txt` file on the Pico SD card written
  by the Pico SD bundle or by the P4 firmware via a cross-SD mechanism
- Adds complexity to the bootloader — defer to a future phase
- Current bootloader has no knowledge of P4 firmware version

**Recommendation:** Layer 1 (App Manager warnings) ships first — it
covers most users. Layer 2 (AnnounceApp) adds runtime awareness.
Layer 3 (bootloader hints) is future-proofing. **No layer ever blocks
the user** — all three are informational.

#### Pico SD bundle versioning

To support the compatibility check, the Pico SD bundle needs a version
marker. The `build-picosd-bundle.yml` workflow should write a
`bundle-version.txt` to the bundle root:

```
dada-tbd-16-picosd-v0.5.zip
├── bundle-version.txt            ← "0.5" (firmware train this bundle targets)
├── BOOT2350.uf2
└── tbd-apps/
    ├── groovebox.uf2
    ├── multi_fx.uf2
    ├── mcl.uf2
    ├── tusb_msc.uf2
    ├── dbg_prb.uf2
    └── ui_test.uf2
```

The App Manager page reads `bundle-version.txt` via USB MSC + File System
Access API to determine the installed train. If the file is missing (older
bundles or factory pre-loads), the page falls back to the ship-date lookup.

Factory devices should ship with this file pre-installed on the Pico SD.

#### Timeline for compatibility enforcement

| Phase | What ships | Compatibility mechanism |
|-------|-----------|----------------------|
| **Phase 3 (flash pages)** | App Manager with version awareness | Layer 1: browser-side firmware version check, warning banner if mismatch, guided Pico SD update offered (never blocks — user can always proceed) |
| **Phase 4 (AnnounceApp)** | AnnounceApp merged + formalized | Layer 2: P4 runtime check, WebUI warning banner, REST API `picoAppCompat` flag (informational only — app always runs) |
| **Phase 5+ (future)** | Bootloader version hints | Layer 3: bootloader reads `firmware-version.txt`, flags mismatched apps in boot menu (never hides or skips apps) |

### App distribution via flash pages and docs

The flash pages and documentation site serve app distribution through two
separate pages — the default Stable Channel and the interactive App Manager:

#### 1. Stable Channel flash page — default experience

The Stable Channel page handles the standard Groovebox setup: P4 firmware +
Groovebox .uf2 + P4 SD card. Same workflow as the current
`65_beta_channel.rst` page:

```
═══════════════════════════════════════════════
  Release 0.4 — Stable Channel
═══════════════════════════════════════════════

  Step 1: Flash P4 Firmware
  P4 Firmware:   v0.4.2
  [Flash P4 Firmware]  (via USB-C port #1)

  Step 2: Flash RP2350 Groovebox
  Groovebox:     v0.4.0
  [Flash Groovebox]  (via USB-C port #2)

  Step 3: Deploy P4 SD Card
  P4 SD Card:    v0.4.5 (WebUI + presets + samples)
  [Deploy P4 SD]  or  [Update WebUI Only]

  ─────────────────────────────────────────────
  Want multi-app mode? → App Manager
  (One step: flash the bootloader to unlock
   Groovebox, Multi FX, MCL and more.)
═══════════════════════════════════════════════
```

A link at the bottom points users to the App Manager page for enabling
multi-app mode, managing apps, or switching back to standalone.

#### 2. App Manager page — interactive app management

A new page (`docs/flash/30_app_manager.rst`) for managing RP2350 apps
interactively via the browser. Users arrive here from the Stable Channel
page when they want to enable multi-app mode or manage their app collection:

```
═══════════════════════════════════════════════
  App Manager — TBD-16
═══════════════════════════════════════════════

  Your firmware: v0.4.2 (detected or selected)

  ─────────────────────────────────────────────
  Enable Multi-App Mode
  ─────────────────────────────────────────────
  Your TBD-16 ships with a Pico SD card
  pre-loaded with Groovebox, Multi FX, MCL,
  and utility apps. Flash the bootloader to
  unlock multi-app mode.

  Firmware detected: v0.5.0 (train 0.5)
  Pico SD apps:     v0.4.x ⚠ MISMATCH

  ⚠ Your Pico SD apps (0.4.x) are not
  compatible with firmware 0.5.x. Update the
  Pico SD card first:
  1. [Flash USB MSC]  (exposes Pico SD card)
  2. [Download Pico SD Bundle v0.5]
  3. Extract to the Pico SD card
  4. Eject and reconnect

  [Flash Bootloader]  [Proceed Anyway →]

  (Recommended: update Pico SD first. But you
  can proceed — some apps may still work fine.)

  ─────────────────────────────────────────────
  Installed Apps
  ─────────────────────────────────────────────
  ✓ Groovebox v0.4.0        [Remove]
  ✓ Multi FX v0.4.0         [Remove]
  ✓ MCL v0.4.0              [Remove]
  ✓ USB MSC v0.4.0          (required)
  ✓ Debug Probe v0.4.0      (required)
  ✓ UI Test v0.4.0          (required)

  ─────────────────────────────────────────────
  Available Apps
  ─────────────────────────────────────────────
  ┌─────────────────────────────────────────┐
  │ ☐ 🎮 Game              Official  v0.4.0 │
  │   Simple game demo                       │
  │   [Docs]                                 │
  ├─────────────────────────────────────────┤
  │ ☐ 🎹 MIDI Controller   Official  v0.5.0 │
  │   USB & TRS MIDI controller              │
  │   (requires firmware v0.5.x)  ⚠ incompat│
  └─────────────────────────────────────────┘

  [Install Selected]

  ─────────────────────────────────────────────
  System Tools
  ─────────────────────────────────────────────
  These require BOOTSEL mode (hold right front
  button while connecting USB-C port #2):

  [Flash Bootloader]  Enable multi-app mode
  [Flash Nuke]        Return to standalone mode
                      (then flash any .uf2 directly)

  ─────────────────────────────────────────────
  Restore Pico SD Card
  ─────────────────────────────────────────────
  Wiped your Pico SD? Download the full bundle:
  [Download Pico SD Bundle v0.4]
  Contains: BOOT2350 + Groovebox + Multi FX +
  MCL + USB MSC + Debug Probe + UI Test
═══════════════════════════════════════════════
```

**How the interactive flashing works:**

The App Manager page leverages the existing Picoboot infrastructure
(`docs/_static/picoflash/`) for browser-based WebUSB communication
with the RP2350. The workflow:

1. User connects USB-C port #2 → browser detects RP2350 via WebUSB
2. If RP2350 is running USB MSC, the page can read the Pico SD card
   contents to show installed apps
3. User selects apps from the catalog — utilities are pre-checked by
   default (recommended, but user can uncheck if they know what they're doing)
4. "Install Selected" downloads `.uf2` files from each app's GitHub
   Release and writes them to `tbd-apps/` on the Pico SD card
5. System tools (bootloader, flash nuke) use Picoboot to flash directly
   to RP2350 flash (requires BOOTSEL mode)

The catalog is loaded from `app-catalog.json` (generated by registry CI,
hosted on GitHub Pages). Downloads link directly to each app's GitHub
Release — no binary hosting needed on the documentation site.

This makes the flash page count **5 pages + shared JS**:
1. Stable Channel (default: P4 firmware + Groovebox + P4 SD card)
2. Staging Channel (pre-release builds)
3. App Manager (multi-app opt-in + interactive app management)
4. WebUI Updates (OTA via WiFi)
5. Troubleshooting

### Registry CI workflows

#### `validate-pr.yml` — runs on every PR to the registry

```yaml
name: Validate App Submission
on:
  pull_request:
    paths: ['apps/**']

jobs:
  validate:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Find changed apps
        id: changed
        run: |
          APPS=$(git diff --name-only origin/main -- apps/ \
            | cut -d'/' -f2 | sort -u | tr '\n' ' ')
          echo "apps=${APPS}" >> "$GITHUB_OUTPUT"

      - name: Validate manifests
        run: |
          for APP in ${{ steps.changed.outputs.apps }}; do
            MANIFEST="apps/${APP}/manifest.json"
            echo "Validating ${MANIFEST}..."

            # Schema validation
            python3 -c "
            import json, sys
            schema = json.load(open('schema/manifest.schema.json'))
            manifest = json.load(open('${MANIFEST}'))
            # Basic field checks (full jsonschema validation optional)
            required = ['id', 'name', 'description', 'author', 'repo',
                        'category', 'sdFilename', 'releases']
            missing = [f for f in required if f not in manifest]
            if missing:
                print(f'Missing required fields: {missing}')
                sys.exit(1)
            for r in manifest['releases']:
                for f in ['version', 'firmwareCompat', 'downloadUrl', 'sha256']:
                    if f not in r:
                        print(f'Release missing field: {f}')
                        sys.exit(1)
            print('Schema OK')
            "

            # Download and verify latest release binary
            LATEST_URL=$(jq -r '.releases[0].downloadUrl' "$MANIFEST")
            LATEST_SHA=$(jq -r '.releases[0].sha256' "$MANIFEST")
            echo "Downloading from ${LATEST_URL}..."
            curl -fsSL -o /tmp/app.uf2 "$LATEST_URL"
            echo "${LATEST_SHA}  /tmp/app.uf2" | sha256sum -c -
            echo "SHA-256 verified ✓"

            # Basic sanity checks
            SIZE=$(stat -c%s /tmp/app.uf2)
            if [ "$SIZE" -gt 2097152 ]; then
              echo "WARNING: .uf2 is larger than 2MB (${SIZE} bytes)"
            fi
          done
```

#### `build-catalog.yml` — regenerates the app catalog

Runs on every merge to `main`. Reads all manifests and produces
`app-catalog.json`, deployed to GitHub Pages on the registry repo
(or committed to the main firmware repo's docs).

### Timeline and phases

The default experience is Groovebox standalone — simple and predictable.
Multi-app is an opt-in feature unlocked by flashing the bootloader. The
App Manager page and registry add progressive layers of interactivity:

| Phase | What happens | Default flow | Multi-app / App Manager |
|-------|-------------|-------------|-------------------------|
| **Phase 2 (CI pipeline)** | CI builds P4 firmware + Groovebox .uf2 + P4 SD archive. Standard 4-artifact release for TBD-16. | Fully working (Groovebox standalone) | Pico SD ships factory pre-loaded; no CI needed for standard releases |
| **Phase 3 (flash pages)** | Stable/Staging flash pages live. Default flow (P4 + Groovebox + P4 SD) works end-to-end. | Complete | Link to App Manager for multi-app opt-in |
| **Phase 4 (App Manager)** | Interactive App Manager page: flash bootloader to enable multi-app, browse catalog, install/remove apps, system tools. | Unchanged | Interactive: one-click bootloader flash + app management via browser |
| **Phase 5 (app registry)** | Create `tbd-app-registry` repo. Automated catalog CI. `app-catalog.json` auto-generated. External contributors submit apps via PR. Pico SD bundle workflow for MAJOR.MINOR updates. | Unchanged | Full: catalog + community apps + optional Pico SD bundle downloads |
| **Future** | On-device app catalog (in WebUI) that can download and install apps over WiFi directly to the Pico SD card. | Unchanged | Dynamic, user-managed from device |

### Multi-target flash pages

When TBD-Core releases become available, the flash pages need to support
target selection. Options:

**Option A — Single page with target selector:**
```
[TBD-16 ▼]  ← dropdown at top of Stable Channel page
             Switches which manifest to load and which artifacts to link
```

**Option B — Separate pages per target:**
```
flash/10_stable_tbd16.rst         ← TBD-16 Stable Channel
flash/11_stable_tbd_core.rst      ← TBD-Core Stable Channel
```

**Recommendation: Option A.** Start with a single page and a target
selector. The manifest JSON already includes all artifact URLs — just
filter by target slug. This avoids duplicating page content and keeps
the user experience simple. If the product line grows beyond 2-3 targets,
switch to Option B.

---

## 11. P4 Plugins — MultiFX Strategy, MIDI API & Hardware UI Adaptation

### Current state: 57 plugins, one adapted

The P4 firmware ships with **57 DSP plugins** in `components/ctagSoundProcessor/`
— synthesizers, effects, drum machines, granular processors, filters, and more.
The plugin index in `docs/plugins/index.rst` lists all of them with a searchable
table showing name, type (Synth Voice / FX / Oscillator / Filter / Drums / Noise /
Sampler / Modulation), and a **MIDI API** status column.

**Today, every single plugin shows "Planned" for MIDI API.** The only plugin
that is fully adapted for RP2350 hardware control is **PicoSeqRack** — the
multi-module drum machine / synth rack that powers the Groovebox firmware.
PicoSeqRack is a special case: it's not a single DSP voice but a full
rack of sub-modules (digital/analog bass drums, snares, hi-hats, FM kick,
303-style synth, polypad, delay, reverb, master mixer) orchestrated by the
Groovebox RP2350 app via the SPI Macro/Preset system.

All other 56 plugins work perfectly through the **WebUI** — the browser
interface auto-generates parameter controls from the `mui-*.json` files on
the P4 SD card. But they have **no MIDI-to-parameter mapping**, no hardware
UI page layout, and no way to control them from the TBD-16 knobs, buttons,
and OLED display.

### The two-slot architecture

The P4 sound processor runs **two plugin slots** (`sp[0]` and `sp[1]`) in
`SPManager.cpp`:

- **Mono plugins** can be loaded independently into each slot (A and B)
- **Stereo plugins** occupy both slots when loaded into slot A
- Audio runs at 48 kHz with 32-sample buffers

Each plugin's `Process()` method receives a `ProcessData` struct containing:

```cpp
struct ProcessData {
    float *buf;                    // Audio I/O buffer
    void *controlData;             // Points to SPI macro data (PicoSeqRack only)
    float *cv;                     // CV inputs (0–1.0)
    uint8_t *trig;                 // Gate/trigger inputs
    uint8_t midi_bytes[400];       // Raw MIDI messages from RP2350
    uint32_t midi_bytes_length;
    uint32_t sequencer_tempo;      // BPM × 100
    uint8_t sequencer_quantum;     // Quantize setting
};
```

Today only PicoSeqRack reads `controlData`, `midi_bytes`, and
`sequencer_tempo`. All other plugins ignore these fields — they only
respond to `cv[]` and `trig[]` (analog modulation from the WebUI or
external hardware).

### The MultiFX firmware goal

The **MultiFX** is a planned RP2350 app — **not a separate codebase but a
variant of the Groovebox firmware** — that lets the user **load any adapted
plugin into slots A and B** and control them from the TBD-16 hardware UI.
It is the general-purpose complement to the Groovebox's dedicated sequencer
workflow.

**Groovebox and MultiFX share code.** Both apps are built from the same
`tbd-pico-seq3` codebase (or a shared library extracted from it). They
reuse the same:

| Shared component | What it does |
|-----------------|-------------|
| `Ui.cpp` / `UiDisplay.cpp` | OLED rendering (128×64 SSD1309, 8 KB framebuffer, 4-wire SPI bit-bang) |
| `UiInputs.cpp` | Button/encoder event polling from STM32 via I2C (4× knobs, 35 buttons, 10 ms cycle) |
| `Knob.cpp` / `MenuHelper.cpp` | Multi-resolution encoder output (fine/medium/coarse) + smooth page transitions |
| `Midi.cpp` / `MidiP4.cpp` | MIDI aggregation from UART1, UART2, USB Host; SPI double-buffer exchange with P4 |
| `SpiAPI.cpp` | P4 communication: query plugins, send MIDI, receive waveform/Link data |
| NeoPixel LED driver | 21× RGB status LEDs |
| Dual-core architecture | Core 0 = UI polling/rendering, Core 1 = audio/SPI (real-time) |

The Groovebox adds the 16-track sequencer, drumpad modes, and Launchpad
integration on top of this shared layer. The MultiFX replaces those with a
simpler plugin-selector + parameter-page UI. **Sharing code and UX patterns
means the user learns one set of interactions** — page navigation, knob
behavior, preset recall, slot switching — that work identically across both
apps.

**What the MultiFX UI looks like on the hardware:**

Instead of the Groovebox's 16-track pages with sub-pages for machine
parameters, the MultiFX shows **parameter pages following the signal flow
of each plugin**, with N knobs per page (4 on TBD-16, potentially more
on future hardware with additional encoders):

```
┌──────────────────────────────────────┐
│ OLED display                         │
│                                      │
│  Slot A: CStrip                      │
│  Page 2/3  [EQ]                      │
│                                      │
│  Knob 1: Treble ████████░░ 3200      │
│  Knob 2: Mid    ██████░░░░ 2400      │
│  Knob 3: Bass   █████░░░░░ 2048      │
│  Knob 4: LPF    ██████████ 4095      │
│                                      │
└──────────────────────────────────────┘
  ◄ Page ►          ◄ Slot A | B ►
```

- **Left/Right navigation**: pages of parameters, signal-flow order
- **Slot switching**: toggle between Slot A and Slot B
- **OLED**: shows which parameters are mapped to the hardware knobs
- **Page size is hardware-dependent**: 4 knobs on TBD-16; future hardware
  targets (e.g. with 8 encoders) get wider pages automatically — the
  `hwui` definition stores parameters per page, the app paginates based
  on how many knobs the hardware has

### MIDI API: from legacy to generalized

There are currently **three MIDI implementations** across the platform:

| Implementation | Location | Used by | Status |
|---|---|---|---|
| **Legacy MIDI Parser** | `rp2350-arduino-tbd-fw` (`MidiParser.cpp`) | Template project / Multi FX prototype | Working but limited — 5 channel groups, 90 CVs, 40 triggers, hardcoded voice modes |
| **Groovebox MIDI** | `tbd-pico-seq3` (`MidiP4.cpp`) | Groovebox firmware | Production — 5-deep FIFO, 960-byte MIDI payload per slot, channel routing |
| **MacroTranslator** | `main/MacroTranslator.cpp` (P4 side) | PicoSeqRack | Production — maps incoming MIDI CCs to 16 tracks × 32 parameters with curve types |

**The goal**: replace the legacy MIDI Parser in the RP2350 template with a
**simplified, generalized** version of the Groovebox's MIDI system from
`tbd-pico-seq3`. The new API should:

1. **Aggregate MIDI from all sources** — UART1, UART2, USB Host, internal
2. **Route to plugin slots** — MIDI channel → Slot A or Slot B
3. **Map CCs to plugin parameters** — using the `mui-*.json` parameter
   definitions, not hardcoded channel groups
4. **Support note events** — note on/off, velocity, pitch bend for synth plugins
5. **Keep tempo sync** — Ableton Link / sequencer BPM via SPI
6. **Stay simple** — no 16-track sequencer logic, no drumpad mode, no
   Launchpad detection — just "MIDI in → plugin control"

This new MIDI API will live in the `rp2350-arduino-tbd-fw` template so all
RP2350 apps (including MultiFX) can use it. The Groovebox keeps its own
specialized MIDI system; the MultiFX and any future apps use the generalized one.

### Adapting plugins: what needs to happen

For a plugin to work with hardware UI control (MultiFX or any RP2350 app
that sends MIDI), two things must be true:

1. **The plugin must respond to MIDI** — its `Process()` method reads
   `midi_bytes` from `ProcessData` and maps incoming CC messages to its
   internal parameters. Today, plugins only read `cv[]` and `trig[]`.

2. **The plugin must declare a hardware UI layout** — which parameters
   appear on which page, in what order, with what display names and
   ranges, so the RP2350 app can render them on the OLED.

#### Part 1: MIDI CC mapping in the plugin

Each plugin already has a `mui-*.json` file that defines all parameters
with IDs, types, and ranges. The MIDI mapping can be derived from this:

- Parameter index in the `mui-*.json` → CC number offset
- Value range (e.g. 0–4095) → scaled from CC 0–127
- Boolean parameters → CC value > 63 = true

This means **the MIDI mapping can be automatic** — no per-plugin
configuration needed. The RP2350 app reads the `mui-*.json` (or a
derivative), assigns CC numbers sequentially, and sends CCs that the
P4 plugin translates back to parameter values.

However, plugins need code changes to actually read `midi_bytes` from
`ProcessData` and apply the values. This is the adaptation work.

#### Part 2: hardware UI layout + parameter range mapping

The existing `mui-*.json` files define parameter groups with a hierarchy:

```json
{
  "id": "CStrip",
  "isStereo": true,
  "params": [
    { "id": "treble", "name": "Treble", "type": "int", "min": 0, "max": 4095 },
    { "id": "mid",    "name": "Mid",    "type": "int", "min": 0, "max": 4095 },
    { "id": "bass",   "name": "Bass",   "type": "int", "min": 0, "max": 4095 },
    ...
  ]
}
```

For the hardware UI, we need to know:
- **Page grouping**: which parameters go on each page
- **Display order**: signal-flow left-to-right (e.g. input → EQ → comp → output)
- **Display names**: short names that fit on the 128×64 OLED (≤8 chars)
- **Which parameters are shown vs hidden**: not all params need hardware knobs
- **Knob-to-DSP range mapping**: which sub-range of the DSP parameter range
  (e.g. 295–768 out of 0–4095) the knob actually controls
- **Response curve**: linear, logarithmic, or exponential scaling

**Strategy: extend the existing `mui-*.json` files** rather than create new
files. Add an optional `hwui` block that carries both page layout and
per-parameter mapping:

```json
{
  "id": "CStrip",
  "isStereo": true,
  "hwui": {
    "pages": [
      {
        "name": "Input",
        "params": [
          { "id": "hipass" },
          { "id": "gate" }
        ]
      },
      {
        "name": "EQ",
        "params": [
          { "id": "treble" },
          { "id": "mid" },
          { "id": "bass" },
          { "id": "bassfreq", "label": "Bass F", "min": 200, "max": 3000, "curve": "log" }
        ]
      },
      {
        "name": "Comp",
        "params": [
          { "id": "comp" },
          { "id": "compspd", "label": "Speed" }
        ]
      },
      {
        "name": "Output",
        "params": [
          { "id": "timelag", "label": "Lag" },
          { "id": "outgain", "label": "Gain" }
        ]
      }
    ]
  },
  "params": [
    { "id": "treble", "name": "Treble", "type": "int", "min": 0, "max": 4095 },
    ...
  ]
}
```

**Per-parameter `hwui` fields** (all optional — sensible defaults apply):

| Field | Default | Purpose |
|-------|---------|---------|
| `id` | *(required)* | References a param `id` from the `params` array |
| `label` | Param `name` (truncated to 8 chars) | Short display name for OLED |
| `min` | Param's original `min` | Knob range start (DSP value). Only this sub-range of the full DSP range is mapped to the knob. |
| `max` | Param's original `max` | Knob range end (DSP value) |
| `curve` | `"linear"` | Response curve: `"linear"`, `"log"`, or `"exp"` |

This is the **generalized version of the Groovebox's MacroDeviceDefinition
mapping** — the same concept of `start`, `mul`, `div`, and `curve` from
files like `td3-allparams.json` and `db-phatpunch.json`, but expressed
as a simple sub-range on each parameter rather than a separate mapping
file with source/destination indices. The proven patterns from PicoSeqRack
are preserved; only the format is simplified.

**Example: mapping a knob to a sub-range with a curve**

The DSP parameter `cutoff` has a full range of 0–4095 in the plugin. But
the plugin creator or sound designer wants the knob to only sweep
295–768 with a logarithmic response (because that's the sweet spot):

```json
{
  "id": "cutoff",
  "label": "Cutoff",
  "min": 295,
  "max": 768,
  "curve": "log"
}
```

The RP2350 sends a MIDI CC value 0–127. The P4 maps it:

```
CC 0   → DSP 295  (knob fully left)
CC 64  → DSP ~450 (log curve: more resolution in low range)
CC 127 → DSP 768  (knob fully right)
```

The curve functions reuse the same integer-only approach from
`MacroTranslator.cpp::applyCurve()`:

| Curve | Behavior | Use case |
|-------|----------|----------|
| `linear` | 1:1 mapping | General parameters |
| `log` | Piecewise linear, expands low range | Frequency, cutoff (hear more detail at low values) |
| `exp` | Quadratic (`val²/127`) | Decay times, envelope shapes (fine control for short times) |

**If no `hwui` override is given**, the knob maps the full param range
with a linear curve — the simplest possible default.

If a plugin's `mui-*.json` has no `hwui` block at all, it has no hardware
UI layout — the MultiFX can either skip it or show a fallback (sequential
pages of parameters in definition order, no signal-flow grouping, no
range constraints).

**No extra JSON files.** The existing `mui-*.json` and `mp-*.json` pairs
remain the single source of truth. The `hwui` extension is additive —
plugins without it continue to work in the WebUI exactly as before.

### The `mp-*.json` preset files: storing hardware presets

The existing `mp-*.json` files store patch presets per plugin:

```json
{
  "activePatch": 0,
  "patches": [
    {
      "name": "Default",
      "params": [
        { "id": "treble", "current": 2047, "cv": -1 },
        { "id": "mid",    "current": 2047, "cv": -1 },
        ...
      ]
    }
  ]
}
```

These presets already work for recalling parameter snapshots. The MultiFX
can use them directly — select a patch, send all parameter values as CCs.
No changes needed to `mp-*.json` for basic preset recall.

If we want hardware-specific preset features (e.g. "which parameters are
CC-mapped vs. fixed"), that can be an extension to `mp-*.json` later. For
the initial MultiFX, the existing preset system is sufficient.

### Curated plugin list: only adapted plugins on the hardware

**Not all 57 plugins will be hardware-adapted at launch.** Adapting a
plugin requires code changes (MIDI CC reading) + UI layout definition +
testing on actual hardware. This is per-plugin effort.

The strategy is:

1. **Start with a curated list** of plugins that are fully adapted and
   tested for hardware UI control
2. **Non-adapted plugins remain available in the WebUI** — they work
   perfectly through the browser, just not from the knobs/OLED
3. **The plugin docs (`docs/plugins/index.rst`) must clearly show** which
   plugins are hardware-adapted — the existing MIDI API column is already
   set up for this (currently all show "Planned")
4. **Over time, adapt more plugins** based on user demand and community
   contributions

#### Initial curated list (proposed)

These plugins are good candidates for first adaptation because they have
clear signal flows, reasonable parameter counts, and cover key use cases:

| Plugin | Type | Ch | Params | Why first |
|--------|------|------|--------|-----------|
| **CStrip** | FX (Channel Strip) | Stereo | 12 | Classic mixing tool, clear signal flow |
| **CStripM** | FX (Channel Strip) | Mono | 12 | Mono variant of CStrip |
| **MonoDelay** | FX (Delay) | Mono | 8 | Essential effect, few params |
| **TDelay** | FX (Delay) | Stereo | ~10 | Stereo delay |
| **GDVerb** | FX (Reverb) | Stereo | ~12 | Modern reverb |
| **MIVerb** | FX (Reverb) | Stereo | ~8 | Mutable Instruments reverb, few params |
| **EChorus** | FX (Chorus) | Stereo | ~6 | Simple modulation FX |
| **MoogFilt** | Filter | Mono | ~8 | Classic Moog filter, iconic |
| **MISVF** | Filter | Mono | ~6 | Mutable Instruments SVF |
| **TBD03** | Synth Voice | Mono | ~20 | TB-303 emulation, beloved |
| **SubSynth** | Synth Voice | Mono | ~14 | Subtractive synth |
| **MacOsc** | Oscillator | Mono | ~8 | Macro oscillator (Braids) |
| **TBDaits** | Synth Voice | Mono | ~12 | Plaits port |
| **SpaceFX** | FX (Multi) | Stereo | ~10 | Combined space effects |
| **FVerb** | FX (Reverb) | Stereo | ~10 | Freeverb classic |

This gives users a working MultiFX palette of ~15 effects and synths
from day one, with more added over time.

### Build exclusion: keeping non-adapted plugins out of MultiFX

There are two distinct concerns:

1. **P4 firmware build**: All 57 plugins are compiled into the P4 firmware
   regardless — they're needed for the WebUI. The plugin factory in
   `ctagSoundProcessorFactory.hpp` (auto-generated from CMakeLists.txt)
   includes every plugin. **This does not change.**

2. **MultiFX app on the RP2350**: The MultiFX app needs to know which
   plugins are hardware-adapted so it only presents those in its UI.
   Non-adapted plugins should not appear in the MultiFX plugin selector.

**Strategy: the MultiFX app reads the `hwui` block from `mui-*.json`.**

When the MultiFX boots, it queries the P4 for the list of available plugins
(already possible via the REST/SPI API). For each plugin, it checks whether
the `mui-*.json` on the P4 SD card has an `hwui` block. If yes → show in
the MultiFX selector. If no → skip.

This means:
- **No build system changes needed** — the P4 firmware compiles all plugins
- **No separate allow/deny list** — the `hwui` presence in `mui-*.json` is
  the signal
- **Adapting a new plugin = adding `hwui` to its JSON + testing** — the
  plugin automatically appears in MultiFX
- **The WebUI continues to show all plugins** — it ignores the `hwui` block

#### Alternative considered: compile-time exclusion

We could add a Kconfig option or CMake variable to exclude non-adapted plugins
from TBD-16/TBD-Core builds entirely. However, this is **not recommended**
because:

- Non-adapted plugins still work in the WebUI — removing them reduces
  functionality for no benefit
- It would require maintaining a separate build config for each hardware target
- The P4 firmware binary size is not the bottleneck (PSRAM is plentiful)
- Users who control plugins via the WebUI or custom RP2350 apps should
  still have access to all 57 plugins

The right exclusion boundary is the **MultiFX UI**, not the **build system**.

### The plugin docs: making adaptation status visible

The `docs/plugins/index.rst` already has a MIDI API column with CSS classes
for status badges. Currently all show `midi-planned`. The column should
reflect three states:

| Status | CSS class | Badge | Meaning |
|--------|-----------|-------|---------|
| **Adapted** | `midi-ready` | ✓ | Hardware UI ready — works with MultiFX and MIDI CCs |
| **Planned** | `midi-planned` | — | Not yet adapted — WebUI only |
| **Not planned** | `midi-na` | n/a | Unlikely to be adapted (e.g. RecNPlay, which depends on WebUI interaction) |

When a plugin is adapted:
1. Update its row in `docs/plugins/index.rst` from `midi-planned` to `midi-ready`
2. Add the `hwui` block to its `mui-*.json`
3. Add MIDI CC handling to its `Process()` method
4. Test on TBD-16 hardware with MultiFX

The plugin docs page should also include a note explaining what "MIDI API"
means — that it indicates whether the plugin can be controlled from the
TBD-16 hardware knobs via the MultiFX app or other RP2350 apps that send
MIDI CCs.

### How this connects to the Macro/Preset system

The Groovebox uses a layered mapping system built for PicoSeqRack:

```
MacroDeviceDefinition (JSON on SD card: /data/macrodefinitions/*.json)
  → defines: which macro params map to which plugin params
  → with: start value, multiplier, divider, curve (linear / log / exp)
  → example: td3-allparams.json, db-phatpunch.json

MacroSoundPreset
  → stores: parameter snapshots for each track
  → with: 32 values per track

MacroTranslator (C++ on P4: main/MacroTranslator.cpp)
  → runtime: receives MIDI CCs from RP2350
  → maps: CC → track → parameter via MacroDeviceDefinition
  → applies: applyCurve() — integer-only piecewise log/quadratic exp
```

This system is powerful but deeply coupled to the 16-track sequencer model.
Each track has a "machine" (a sub-module of PicoSeqRack), and each machine
has its own `macrodefinitions/*.json` file with per-parameter mapping. The
34 existing macro definition files cover just the PicoSeqRack sub-modules.

**The `hwui` block generalizes the best ideas from this system:**

| MacroDeviceDefinition concept | `hwui` equivalent | What changed |
|------------------------------|-------------------|-------------|
| `groups[].pages` with 4 params each | `hwui.pages[]` with N params each | Same concept, not hardcoded to 4 |
| `parameters[].min` / `parameters[].max` | `hwui.pages[].params[].min` / `.max` | Same concept — sub-range of the DSP range |
| `parameters[].def` (default value) | `mp-*.json` `current` field | Already exists in preset files |
| `parameters[].name` (short OLED label) | `hwui.pages[].params[].label` | Same concept — ≤8 chars for OLED |
| `parameters[].ui` (widget type: freq, envdecay, etc.) | Not needed initially | MultiFX uses generic bar/number display |
| `mapping[].start` (base offset) | `hwui.pages[].params[].min` | Subsumed by min — same effect |
| `mapping[].add[].mul` / `.div` | Implicit in min/max range | The range defines the scaling directly |
| `mapping[].add[].curve` | `hwui.pages[].params[].curve` | Same three curves: linear, log, exp |
| Separate `.json` file per machine | Inline in `mui-*.json` | **No extra files** — single source of truth |

**Example comparison:**

The Groovebox maps TBD03's cutoff using two separate files:

*td3-allparams.json (macro definition):*
```json
{"ctrl": 12, "start": 0, "add": [{"src": 4, "mul": 1, "div": 1, "curve": "log"}]}
```

*The `hwui` equivalent (inline in mui-TBD03.json):*
```json
{ "id": "cutoff", "label": "Cutoff", "curve": "log" }
```

And for a case where the Groovebox restricts the range (like
`db-phatpunch.json` where `start: 4, mul: 1, div: 3` scales punch to 1/3):

*MacroDeviceDefinition:*
```json
{"ctrl": 13, "start": 4, "add": [{"src": 2, "mul": 1, "div": 3}]}
```

*The `hwui` equivalent:*
```json
{ "id": "punch", "min": 4, "max": 46 }
```

The `min`/`max` sub-range achieves the same result as `start` + `mul/div`
but is far easier to read, write, and reason about. The sound designer or
plugin creator simply says: "this knob sweeps 4–46" instead of calculating
multiplier/divider fractions.

**What we keep from the Macro system:**
- The `applyCurve()` function (integer-only, real-time safe) → reused for
  `hwui` curve processing
- The page-based parameter grouping concept → `hwui.pages[]`
- The sub-range mapping concept → `hwui` `min`/`max`
- The preset system (`mp-*.json`) → used as-is

**What we simplify:**
- No separate macro definition files → everything in `mui-*.json`
- No `mul`/`div` fractions → replaced by explicit min/max sub-range
- No `ctrl` index indirection → sequential CC mapping from parameter order
- No 16-track model → just Slot A and Slot B
- No machine switching per track → plugin selection per slot

### SPI protocol: what needs to change

The current SPI protocol (`SpiProtocol.h`) already carries MIDI bytes
from RP2350 → P4:

```
p4_spi_request2:
  magic (4B) | synth_midi_length (4B) | synth_midi[256] | sequencer_tempo | active_track
```

For MultiFX, the same SPI buffer works — the RP2350 sends MIDI CCs that
the P4 routes to the appropriate plugin slot. No protocol changes needed.

What does change is **on the P4 side**: instead of routing all MIDI through
the MacroTranslator (which is PicoSeqRack-specific), the MultiFX mode needs
a simpler path:

1. RP2350 sends CC messages with a channel assignment (e.g. ch1 → Slot A,
   ch2 → Slot B)
2. P4 receives the MIDI bytes in `ProcessData.midi_bytes`
3. Each plugin's `Process()` reads its assigned MIDI channel's CCs and
   updates parameters directly

This keeps the P4 firmware generic — it doesn't need to know about
MultiFX specifically. The SPI carries MIDI; each plugin reads its own CCs.

### Timeline for plugin adaptation

Plugin adaptation is incremental. It does not block any other milestone:

| Phase | What happens |
|-------|-------------|
| **v0.4.x** | Groovebox ships as default (PicoSeqRack only). All 57 plugins available via WebUI. MultiFX app is a prototype using the legacy MIDI Parser. |
| **v0.5.0** | New generalized MIDI API in `rp2350-arduino-tbd-fw`. First batch of ~15 plugins adapted with `hwui` blocks. MultiFX app released as official app. Plugin docs updated with adaptation status. |
| **v0.5.x+** | Community contributes `hwui` blocks and MIDI handling for more plugins. Plugin docs page reflects growing adaptation coverage. |
| **Future** | On-device plugin browser on the OLED. Parameter mapping editor in the WebUI. User-defined `hwui` overrides (drag-and-drop parameter pages). |

### File extension migration: `.jsn` → `.json`

All JSON files in the project previously used the non-standard `.jsn` extension.
They have been renamed to `.json` as part of Phase 1. The rename was done
alongside the `hwui` block additions to avoid touching files twice.

**Files renamed (122 total):**

| Location | Count | Pattern |
|----------|-------|---------|
| `sdcard_image/data/sp/` | 57 | `mui-*.jsn` → `mui-*.json` |
| `sdcard_image/data/sp/` | 57 | `mp-*.jsn` → `mp-*.json` |
| `sdcard_image/data/` | 1 | `spm-config.jsn` → `spm-config.json` |
| `sdcard_image/data/` | 1 | `favs.jsn` → `favs.json` |
| `sample_rom/tbdsamples/` | 4 | `*.jsn` → `*.json` (bank definition files) |
| `generators/` | 1 | `mui-template.jsn` → `mui-template.json` |
| `simulator/data/` | 1 | `spm-config.jsn` → `spm-config.json` |
| **Total** | **122** | |

**Code updated (27 source files):**

- `SPManager.cpp` — loads `mui-*` / `mp-*` by name, constructs path strings
- `SPManagerDataModel.cpp` — reads `spm-config.json`
- `Favorites.cpp` / `FavoritesModel.cpp` — reads/writes `favs.json`
- `SampleAPI.cpp` — `.json` extension check fixed (was broken `len-4` for 5-char ext)
- `ctagSampleRomModel.cpp` — `sample_rom.json` definition file path
- `sample_rom.json` — internal references to bank `.json` filenames
- `simulator/WebServer.cpp` — removed duplicate MIME entry
- `dev-server.js` — simplified extension check; fixed plugin list to scan `mui-*.json`
- 20+ additional source files with `.jsn` string references
- `RestServer.cpp` — **no changes needed** (pure HTTP routing, no file paths)
- `create_sd_archive.sh` — **no changes needed** (uses generic file copying)

**Migration completed:**

1. ✅ `git mv` all 122 files
2. ✅ `sed` find-and-replace `.jsn` → `.json` in all 24 source files
3. ✅ 3 edge cases fixed manually (SampleAPI len check, dev-server duplicate, WebServer MIME)
4. ✅ `.gz` bundles regenerated for `app-bundle.js` and `macro-bundle.js`
5. ✅ Dev server verified — all 57 plugins load correctly
6. ✅ `create_sd_archive.sh` confirmed clean (no `.jsn` globs)
7. ✅ Full codebase audit — zero `.jsn` files or text references remain

This was a mechanical rename — no logic changes, no data format changes.
The `macrodefinitions/*.json` files already used `.json`, so after this
migration every JSON file in the project has the standard extension.

### Summary of the plugin strategy

| Decision | Rationale |
|----------|-----------|
| Extend `mui-*.json` with optional `hwui` block | Single source of truth, no file proliferation; generalizes MacroDeviceDefinition concept |
| Per-parameter sub-range mapping (`min`/`max`/`curve`) | Plugin creator / sound designer controls exactly what knob range and response curve each param uses |
| Reuse `applyCurve()` (linear / log / exp) | Proven integer-only real-time-safe curves from Groovebox MacroTranslator |
| No changes to `mp-*.json` (for now) | Existing presets work for MultiFX |
| MultiFX shares codebase with Groovebox | Same UI rendering, knob handling, page navigation, SPI layer — users learn one workflow |
| Page size adapts to hardware | `hwui` defines logical pages; apps paginate by knob count (4 on TBD-16, potentially more on future targets) |
| No build exclusion of non-adapted plugins | They still work in WebUI; exclusion is in MultiFX UI |
| `hwui` presence = "adapted" signal | MultiFX reads it; WebUI ignores it |
| Start with ~15 curated plugins | Ship something useful, expand incrementally |
| Generalize MIDI API from Groovebox | One API for all future RP2350 apps |
| Plugin docs show adaptation status | Users know what works on hardware |
| Adaptation doesn't block releases | WebUI covers all plugins always |
| Rename `.jsn` → `.json` | Standard extension; done alongside `hwui` additions to avoid touching files twice |

---

## 12. Branching Strategy — dadamachines

### Target branch structure

```
dada-tbd-master     ← stable releases (production)
                       CI builds + deploys docs on every push
                       no gh-pages branch needed (artifact-based deploy)

staging             ← pre-release builds (testing)
                       CI builds + publishes to GitHub Releases
                       manifest auto-updates → staging flash page
```

### Branches to archive or delete

| Branch | Action | Reason |
|---|---|---|
| `master` | Delete | Confusing alongside `dada-tbd-master` |
| `dev` | Delete after merge | Use forks + PRs instead |
| `p4_main` | Archive tag + delete | Historical, superseded by `dada-tbd-master` |
| `p4_main_sdonly` | Archive tag + delete | Historical |
| `perf_test` | Archive tag + delete | One-off testing |
| `feature/*` | Delete after merge | Should live in contributor forks |
| `legacy-master-1.0.0` | Keep as tag | Preserve as `v1.0.0` tag, delete branch |

Preserve history before deleting:

```bash
# Tag branches so nothing is lost
git tag archive/p4_main p4_main
git tag archive/p4_main_sdonly p4_main_sdonly
git tag archive/perf_test perf_test
git tag archive/dev dev
git tag v1.0.0 legacy-master-1.0.0

# Then delete the branches (remote)
git push origin --delete p4_main p4_main_sdonly perf_test dev master
```

### Development workflow for dadamachines engineers

```
┌──────────────────────────────────────────┐
│  dadamachines/ctag-tbd                    │
│  branch: dada-tbd-master                  │  ← canonical
│  remote: ctag-upstream → ctag-fh-kiel     │
└────────────────┬─────────────────────────┘
                 │
    fork ────────┼──── fork ──────── fork
                 │
┌────────────────┴──────────────┐
│  engineer/ctag-tbd             │  ← personal fork
│  branch: feature/my-thing      │
└────────────────────────────────┘
                 │
                 └──→ Pull Request → dada-tbd-master
```

- Engineers **fork** the repo on GitHub
- Work in feature branches on their fork
- Submit PRs to `dada-tbd-master` upstream
- CI runs build check + docs deploy automatically on merge
- **No need to remove `docs/` from forks** — with LFS, the binary files are
  just 130-byte pointers. The `docs/*.rst` source files are tiny.

---

## 13. Upstream Relationship — ctag-fh-kiel

### Current reality: upstream is dormant

The ctag-fh-kiel/ctag-tbd `p4_main` branch has had **zero new commits**
since the point where `dada-tbd-master` forked from it. The upstream team
is not actively working on the ESP32-P4 project. Meanwhile dadamachines
has been the sole active developer for months — 180+ commits on
`dada-tbd-master`, plus 190+ on the leading feature branch, plus the full
macro/preset system from a third contributor.

**dadamachines is the project now.** Our strategy reflects that:

1. Build a clean foundation on `dada-tbd-master` (Phases 0–3)
2. Execute the full plan (Phases 4–7)
3. When upstream is ready, they adopt our work — not the other way around

### The model: dadamachines leads, upstream adopts

```
dadamachines/ctag-tbd (dada-tbd-master)   ← active development, LGPL product layer
        │
        │  Kconfig guards keep core code portable
        │  upstream can merge when ready
        ▼
ctag-fh-kiel/ctag-tbd (p4_main)           ← dormant; merges FROM dadamachines
                                              when they resume work
```

Best case: ctag-fh-kiel adopts `dada-tbd-master` (or a derivative) as
their new primary branch, with Kconfig adaptations for their platform.
Worst case: they cherry-pick individual improvements when their schedule
permits. Either way, **dadamachines does not wait on upstream.**

### What this means in practice

- **No periodic merges FROM upstream.** There is nothing to merge — `p4_main`
  has not moved. We do not pull from a dormant branch.
- **Kconfig guards are for portability, not merge conflict reduction.**
  The `#ifdef CONFIG_TBD_USE_RP2350` and `#ifdef CONFIG_TBD_USE_SD_CARD`
  guards make the codebase buildable for different hardware targets (TBD-16
  vs TBD-Core vs upstream's ESP32-P4-only config). This means upstream can
  build our code with product features disabled — a clean adoption path.
- **Contributing back is an offer, not a dependency.** If we fix a DSP
  engine bug or improve the audio codec, we can open a PR against `p4_main`.
  But our release cadence does not depend on their review timeline.
- **We keep the `ctag-upstream` remote** as a reference for history, not as
  a sync point.

### What dadamachines owns (all of it)

With upstream dormant, there is no meaningful "upstream-only" layer.
Everything is in `dada-tbd-master`:

| Layer | Components | License |
|-------|-----------|--------|
| Core platform | ESP-IDF setup, audio codec, PSRAM, plugin architecture, DSP base class, sample ROM, core plugins, REST skeleton, WiFi/network, filesystem | GPLv3 (inherited from upstream) |
| Shared layer | Shoelace WebUI (themeable, feature-gated), core plugin manager UI, device config UI | GPLv3 (dadamachines contribution, offered upstream) |
| Product layer | RP2350 SPI bridge, sequencer, macros, presets, PicoSeqRack, rack DSP machines, macro WebUI, product config | LGPL (dadamachines only) |

### What upstream can adopt when they're ready

| Component | Adoption path |
|---|---|
| Shoelace WebUI (themeable, feature-gated) | Merge with their theme; macro sections hidden when `CONFIG_TBD_USE_RP2350=n` |
| DSP engine improvements (codec, PSRAM, plugins) | Cherry-pick or merge — pure core code |
| New GPLv3 sound processor plugins | Cherry-pick individual plugins |
| REST API core routes (plugin config, device API) | Included in WebUI adoption |
| Bug fixes discovered during TBD-16 development | Cherry-pick individual commits |
| Kconfig build configuration system | Adopt as-is — gives them hardware configurability for their own targets |

### How Kconfig flags enable portability

The `proposal-simple-tbd-config.md` Kconfig flags (`CONFIG_TBD_USE_SD_CARD`,
`CONFIG_TBD_USE_RP2350`) make the single codebase buildable for multiple
hardware configurations. This is valuable regardless of whether upstream
ever adopts the code:

| File | Core concern | Product concern | Guard |
|------|-------------|-----------------|-------|
| `main/SPManager.cpp` | Audio pipeline, plugin hosting | SPI buffer exchange, macro system | `#ifdef CONFIG_TBD_USE_RP2350` |
| `main/RestServer.cpp` | Base API routes, CORS, server | MacroAPI route, product endpoints | `#ifdef CONFIG_TBD_USE_RP2350` |
| `main/main.cpp` | Boot sequence, filesystem init | SpiAPI start, sequencer init | `#ifdef CONFIG_TBD_USE_RP2350` |
| `CMakeLists.txt` | Core source list | rack/ sources, macro files | `if(CONFIG_TBD_USE_RP2350)` |
| `components/.../fs.cpp` | Filesystem mount | SD card vs LittleFS | `#ifdef CONFIG_TBD_USE_SD_CARD` |
| `components/.../ctagSampleRom.cpp` | Sample loading | SD WAV vs flash `.tbd` | `#ifdef CONFIG_TBD_USE_SD_CARD` |

With these guards:
- TBD-16 builds with `CONFIG_TBD_USE_RP2350=y` — full product
- TBD-Core builds with `CONFIG_TBD_USE_RP2350=y`, different pin mapping
- Upstream's ESP32-P4-only config builds with both flags `=n` — product code compiles out
- Multi-config CI catches regressions across all configurations

### WebUI feature gating (runtime)

One WebUI binary works on all hardware configs. A `/api/v1/capabilities`
endpoint returns what's available:

```json
{ "macros": true, "sdcard": true, "sequencer": true }
```

The WebUI reads this on load and shows/hides sections. No separate builds needed.

### If upstream resumes active development

If ctag-fh-kiel resumes work on `p4_main`, the Kconfig guards and clean
layering make integration straightforward in either direction:

- **They adopt our master:** Merge `dada-tbd-master` into their repo.
  Build with `CONFIG_TBD_USE_RP2350=n` to get the core platform + WebUI
  without product features. Apply their own theme.
- **They diverge independently:** Cherry-pick specific improvements.
  The Kconfig guards mean our commits to shared files are self-contained.
- **We pick up their work:** If they add new plugins or DSP improvements,
  we can cherry-pick from `p4_main` at that point — but only when there
  is actually something to pick up.

The strategy is the same in all cases: **dadamachines moves forward.
Upstream catches up on their schedule.**

---

## 14. Keep `sdcard_image/` and `sample_rom/` in the Monorepo

**Do not split docs or assets into a separate repo.** Reasons:

- `sdcard_image/www/` is the **WebUI source** that must stay in sync with C++
  firmware. Cross-repo versioning creates coordination overhead and bugs.
- `sdcard_image/data/` contains presets, macro definitions, and synth
  definitions that are tightly coupled to the firmware's JSON parsing.
- `sample_rom/` is needed for factory SD card image builds. With Git LFS,
  engineers who don't need it pay no clone cost.
- Docs (`.rst` files) reference the firmware code and vice versa.
- CI already builds docs in the same repo.
- The `create_sd_archive.sh` script bundles `sdcard_image/` into the release
  archive — having it in the same repo guarantees the WebUI and firmware are
  always version-matched.

---

## 15. Clean Up Git History (do this in Phase 3b)

After deleting old binaries from the working tree, the `.git/` pack still
contains 246 MB of old binary blobs. With only ~10 test users, a history
rewrite is trivial to coordinate — **do it now** rather than carrying dead
weight forever.

```bash
brew install git-filter-repo

git filter-repo \
  --path-glob 'docs/_static/firmware/p4/possan-tbd-2026-02-*.bin' --invert-paths \
  --path-glob 'docs/_static/firmware/p4/possan-tbd-2026-03-0*.bin' --invert-paths \
  --path-glob 'docs/_static/firmware/pico/possan-tbd-2026-02-*.uf2' --invert-paths \
  --path-glob 'docs/_static/sdcard_image/2026-02-*' --invert-paths \
  --path-glob 'docs/_static/sdcard_image/2026-03-0*' --invert-paths

git push origin --force --all
git push origin --force --tags
```

> **Note:** This rewrites history. All ~10 test users and contributors must
> re-clone or re-fork after this. Coordinate via a simple message — with this
> team size, it's a non-event.

**Expected savings:** `.git/` should drop from ~246 MB to under ~50 MB.

---

## 16. Migration Checklist

> **Context:** Pre-release product, ~10 test users, no public users depending
> on current URLs or artifacts. This allows aggressive cleanup — history
> rewrite, bulk deletion, fresh flash pages — without migration scripts or
> backwards compatibility.
>
> **Ordering principles:** Each phase has a single clear deliverable that
> can be tested end-to-end. Dependencies flow strictly downward — no phase
> references work from a later phase. Highest-value work ships first
> (CI + working flash page). Mechanical changes (rename, cleanup) happen
> early to avoid conflicts with later feature work.

```
Phase 0 — Merge experimental branch (hard prerequisite for CI) ✅ DONE
  ─────────────────────────────────────────────────────────────
  The patches/ directory, updated sdkconfig.defaults, and the
  git describe --always fix were on
  nevvkid/dada-tbd-master-experimental. Merged into
  dadamachines/ctag-tbd dada-tbd-master via PR (2026-03-21).
  ─────────────────────────────────────────────────────────────
  [x] Cherry-pick or merge from experimental into dada-tbd-master:
      - patches/ directory (ESP-IDF v5.5.3 P4 fixes)
      - CMakeLists.txt patch-apply block (idempotent git apply)
      - sdkconfig.defaults cleanup (remove stale TinyUSB task config)
      - git describe --always flag addition
  [x] Verify local build succeeds with stock espressif/idf:v5.5.3 Docker image
  [x] Verify patches apply cleanly inside the Docker container
  Deliverable: dada-tbd-master builds correctly in the CI container image.

Phase 1 — Clean slate (repo cleanup + file rename)
  ─────────────────────────────────────────────────────────────
  Mechanical changes that don't rewrite history. The .jsn → .json
  rename goes here so every subsequent phase uses the final paths.
  Branch archiving is non-destructive (just tags + remote deletes).
  ─────────────────────────────────────────────────────────────
  [x] Delete all old firmware builds + SD card images from docs/_static/ (12 .bin + 9 .uf2 + 8 SD .zip)
  [x] Delete all old WebUI update zips from docs/_static/updates/ (6 .zip + latest.json)
  [x] Rename all 122 .jsn files to .json (git mv — 122 files across sdcard_image/, sample_rom/, generators/, simulator/)
  [x] Update all C++ path strings referencing .jsn → .json (27 source files; 3 edge cases fixed manually)
  [x] Verify create_sd_archive.sh and scripts — no .jsn globs found, no changes needed
  [x] Verify dev server — all 57 plugins load; fixed pre-existing bug (was showing only 23 Groovebox machines)
  [x] Full codebase audit — zero .jsn files or text references remain (only old binaries in docs/_static/)
  [ ] Tag historical branches for archival (archive/p4_main, etc.)
  [ ] Delete stale branches from origin (master, dev, p4_main, etc.)
  [ ] Verify GitHub default branch is dada-tbd-master
  Deliverable: Clean working tree. All JSON files use .json. Old binaries
  deleted. Stale branches archived. No history rewrite yet — that happens
  after CI + flash pages are confirmed working (Phase 3b).

Phase 2 — CI pipeline + versioning + minimal flash page
  ─────────────────────────────────────────────────────────────
  Get automated builds working, tag v0.4.0, and restore a
  working browser flash page so testers aren't blocked.
  TBD-Core multi-target matrix is deferred — ship TBD-16 first.
  ─────────────────────────────────────────────────────────────
  [ ] Add build-firmware.yml workflow (reusable, container: espressif/idf:v5.5.3)
  [ ] Add ci.yml workflow for PR build checks (paths: main/**, components/**, CMakeLists.txt)
  [ ] Add create-release.yml workflow (tag trigger + manual dispatch)
  [ ] Verify build succeeds in CI (firmware + P4 SD archive)
  [ ] Tag dada-tbd-master: git tag v0.4.0
  [ ] Bump webui-version.json to 0.4.0
  [ ] Test: tag push → CI builds → GitHub Release created with 4 TBD-16 artifacts
  [ ] Verify P4 SD card zip contains data/www/tbdsamples (no .uf2 files)
  [ ] Extract shared JS into docs/_static/js/tbd-flasher.js
  [ ] Create 10_stable_channel.rst (P4 firmware + Groovebox .uf2 + P4 SD card)
  [ ] Create minimal flash/index.rst with CTA to Stable Channel
  [ ] Test: end-to-end browser flash from Stable Channel page using CI-built release
  Deliverable: CI builds on every push/tag. v0.4.0 release on GitHub.
  Testers can flash via the new Stable Channel page.

Phase 3 — Staging channel + remaining flash pages
  ─────────────────────────────────────────────────────────────
  Depends on: Phase 2 (build-firmware.yml must exist for staging
  to call it as a reusable workflow).
  ─────────────────────────────────────────────────────────────
  [ ] Create staging branch from dada-tbd-master
  [ ] Add staging-release.yml workflow (pushes to staging → pre-release + manifest update)
  [ ] Add feature-test-release.yml workflow (feature-test/* branches → per-feature manifests)
  [ ] Create docs/_static/firmware/staging-manifest.json (empty initial)
  [ ] Test: push to staging → verify pre-release created → verify manifest updated
  [ ] Create 20_staging_channel.rst (manifest-driven, ?channel= support for feature tests)
  [ ] Create 30_app_manager.rst (placeholder: bootloader flash via BOOTSEL, system tools, sideload section)
  [ ] Create 40_webui_updates.rst (WebUI updater docs + version history)
  [ ] Create 50_troubleshooting.rst (general flash troubleshooting)
  [ ] Rewrite flash/index.rst with full CTA layout + toctree (all 5 pages)
  [ ] Add compatRange field to staging + stable manifests
  [ ] Add MAJOR.MINOR compatibility check to WebUI updater page (soft warning)
  [ ] Test: staging flash page loads manifest, flash works end-to-end
  [ ] Delete old flash pages: 25, 30, 50, 60, 65, 66, 67, 68, 70
      (deferred from Phase 1 — delete only after new pages are verified working)
  Deliverable: Full 5-page flash section. Staging channel live.
  Feature-test channel available for ad-hoc experiments.

Phase 3b — Git LFS + history rewrite + force-push
  ─────────────────────────────────────────────────────────────
  Deferred from Phase 1. Now that CI builds work, flash pages
  are verified, and old pages are deleted, this is the right
  time to do the irreversible history rewrite. The team
  re-clones exactly once, after the full foundation is solid.
  ─────────────────────────────────────────────────────────────
  [ ] Enable Git LFS (.gitattributes for *.wav, *.bin, *.uf2, *.zip, *.png, *.jpg)
  [ ] Run git filter-repo to remove old binary blobs from git history
  [ ] Force-push cleaned history
  [ ] Notify team (~10 people) to re-clone
  Deliverable: .git/ drops from ~350 MB to under ~50 MB. All contributors
  re-clone once. Future binary assets tracked by LFS.

Phase 4 — Kconfig guards + multi-target CI
  ─────────────────────────────────────────────────────────────
  Can run in parallel with Phase 3 (no dependency).
  Kconfig guards are needed before TBD-Core launches and
  to keep the codebase portable for future upstream adoption.
  Upstream (ctag-fh-kiel) is dormant — there is nothing to
  merge from p4_main. We move forward; they catch up later.
  ─────────────────────────────────────────────────────────────
  [ ] Implement CONFIG_TBD_USE_SD_CARD and CONFIG_TBD_USE_RP2350 Kconfig flags
  [ ] Add #ifdef guards to the 6 shared files (SPManager, RestServer, main, CMakeLists, fs, ctagSampleRom)
  [ ] Verify the codebase builds with both Kconfig flags disabled (upstream-compatible config)
  [ ] Create sdkconfig.tbd16 and sdkconfig.tbd-core for multi-target builds
  [ ] Add multi-config CI matrix (all 4 hardware configurations)
  [ ] Verify CI matrix produces dada-tbd-16-*.bin and dada-tbd-core-*.bin
  [ ] Add ctag-upstream remote as a reference (for history, not for syncing)
  Deliverable: Codebase builds for TBD-16, TBD-Core, and upstream's
  ESP32-P4-only config. Multi-config CI catches regressions across all
  hardware variants. Upstream can adopt the code whenever they're ready.

Phase 5 — AnnounceApp + firmware ↔ app compatibility
  ─────────────────────────────────────────────────────────────
  Depends on: Phase 2 (versioning scheme must exist so version
  strings have meaning). Separated from the app registry because
  this is P4 firmware + WebUI work, not repo infrastructure.
  ─────────────────────────────────────────────────────────────
  [ ] Merge announceApp from experimental branch into dada-tbd-master
  [ ] Assign stable SPI command ID, document payload in SpiAPI.hpp
  [ ] Add P4-side handler: store app identity (name, version, caps) in DeviceAPI
  [ ] Add picoApp, picoVersion, picoCaps fields to /api/v2/device REST response
  [ ] Add WebUI device info panel: show active Pico app name + version
  [ ] Add picoAppCompat flag when app MAJOR.MINOR ≠ firmware MAJOR.MINOR
  [ ] Add dismissible WebUI warning banner for compatibility mismatch
  [ ] Add firmware version detection via WebSerial on App Manager page
  [ ] Add fallback firmware version dropdown for manual selection on App Manager
  [ ] Implement firmware ↔ app version check on App Manager (warning banner, never blocks)
  [ ] Test: flash old Groovebox on new firmware → WebUI shows warning → app still runs
  Deliverable: P4 knows which Pico app is running. WebUI and App Manager
  show version info and soft warnings. No layer ever blocks the user.

Phase 6 — App registry + App Manager catalog + Pico SD bundles
  ─────────────────────────────────────────────────────────────
  Depends on: Phase 3 (30_app_manager.rst must exist as the
  shell page) and Phase 5 (compatibility check must work).
  ─────────────────────────────────────────────────────────────
  [ ] Create dadamachines/tbd-app-registry repo with folder structure
  [ ] Define manifest.schema.json and bundle.schema.json
  [ ] Add validate-pr.yml workflow (schema + download + SHA-256 check)
  [ ] Add build-catalog.yml workflow (generates app-catalog.json on merge)
  [ ] Migrate Groovebox, Multi FX, MCL manifests into registry (apps/)
  [ ] Migrate USB MSC, Debug Probe, UI Test, Game manifests (apps/, alwaysInstalled)
  [ ] Add system-tools/bootloader/ and system-tools/flash-nuke/ manifests
  [ ] Create tbd-16-v0.4.json bundle definition (all default apps + utilities)
  [ ] Add build-picosd-bundle.yml workflow (assembles Pico SD bundle from registry)
  [ ] Add bundle-version.txt to Pico SD bundle build output
  [ ] Wire app-catalog.json into App Manager page (30_app_manager.rst)
  [ ] Integrate Picoboot WebUSB for bootloader flashing on App Manager
  [ ] Integrate Picoboot WebUSB for interactive app install on App Manager
  [ ] Add "Install Local App" (.uf2 sideload) section to App Manager page
  [ ] Add bundle-version.txt reading via File System Access API on App Manager
  [ ] Test App Manager: install/remove apps via Picoboot WebUSB
  [ ] Test App Manager: sideload local .uf2 via File System Access API
  [ ] Test system tools: flash bootloader + flash nuke via BOOTSEL
  [ ] Write contributor guide (README.md in app-registry repo)
  [ ] Onboard first external contributor (jmamma/MCL → partner manifest)
  Deliverable: App registry live. App Manager fully interactive with
  catalog, install/remove, sideload, and system tools. Pico SD bundle
  downloadable for restore/update scenarios.

Phase 7 — Plugin adaptation + MultiFX
  ─────────────────────────────────────────────────────────────
  Depends on: Phase 6 (MultiFX publishes to the app registry).
  The .jsn → .json rename was done in Phase 1 — all files are
  already .json. This phase is purely plugin + RP2350 app work.
  ─────────────────────────────────────────────────────────────
  [ ] Extract shared MultiFX/Groovebox components into common library (Ui, UiDisplay, Knob, MenuHelper, Midi, SpiAPI)
  [ ] Design generalized MIDI API for rp2350-arduino-tbd-fw (replace legacy MidiParser)
  [ ] Implement CC-to-parameter routing in P4 ProcessData path (per-slot MIDI channel)
  [ ] Define hwui JSON schema (page grouping, per-param sub-range + curve, display names)
  [ ] Add hwui block to first batch of ~15 mui-*.json files (see curated list in section 11)
  [ ] Add MIDI CC handling to Process() of each adapted plugin
  [ ] Update docs/plugins/index.rst MIDI API column (midi-planned → midi-ready per plugin)
  [ ] Add CSS for midi-ready and midi-na badge styles to plugin docs
  [ ] Add explanatory note to plugin docs about MIDI API / hardware UI meaning
  [ ] Build and test MultiFX RP2350 app with adapted plugins on TBD-16 hardware
  [ ] Test MultiFX plugin selector (only shows plugins with hwui block)
  [ ] Test OLED page navigation (knobs per page adapts to hardware — 4 on TBD-16)
  [ ] Test sub-range mapping + curve accuracy (verify knob sweep matches hwui min/max/curve)
  [ ] Test preset recall via mp-*.json in MultiFX context
  [ ] Publish MultiFX as official app in tbd-app-registry
  [ ] Update rp2350-arduino-tbd-fw template README with new MIDI API docs
  Deliverable: MultiFX app live in the registry. ~15 plugins controllable
  from TBD-16 knobs/OLED. Plugin docs reflect adaptation status.
```

### Dependency graph

```
Phase 0 ─── Merge experimental (patches, sdkconfig)
  │
  ▼
Phase 1 ─── Clean slate (delete binaries + rename files + archive branches)
  │
  ▼
Phase 2 ─── CI pipeline + v0.4.0 + Stable Channel page
  │
  ├──────────────────┐
  ▼                  ▼
Phase 3              Phase 4
Staging channel      Kconfig guards
+ flash pages        + multi-target CI
  │                  (independent)
  ▼
Phase 3b ── Git LFS + history rewrite + force-push (team re-clones once)
  │
  ▼
Phase 5 ─── AnnounceApp + compatibility warnings
  │
  ▼
Phase 6 ─── App registry + App Manager catalog
  │
  ▼
Phase 7 ─── Plugin adaptation + MultiFX
```

**Phases 3 and 4 can run in parallel** — they have no mutual dependency.
Both depend only on Phase 2. This is the main parallelism opportunity.
**Phase 3b** runs after Phase 3 — the history rewrite happens only after
CI, flash pages, and old page deletion are all confirmed working. The team
re-clones exactly once.

---

## 17. Summary

| Change | Effort | Impact |
|---|---|---|
| Delete old binaries + rewrite git history | Low (10 users re-clone) | ~450 MB less total (~220 working tree + ~230 git history) |
| CI firmware builds | Medium | Reproducible builds, automated releases |
| CI build checks on PRs | Low | Catch regressions before merge |
| Unified versioning (v0.4.0 start) | Low | Clear firmware ↔ WebUI compatibility |
| New flash pages (5 pages + shared JS) | Medium | 8,000 → 2,500 lines, zero code duplication |
| Staging + feature test channels | Medium | Pre-release testing via browser flash, CI-built |
| Git LFS for remaining binaries | Low | Fast clones for engineers |
| Branch cleanup (two long-lived branches) | Low | Cleaner repo, less confusion |
| Kconfig guards + multi-target CI | Medium | Portable codebase; TBD-Core builds; upstream can adopt when ready |
| Multi-target naming (`dada-{target}-*`) | Low | Clear hardware identification in every artifact |
| RP2350 app versioning + AnnounceApp design | Medium | App compatibility tracking, future app store |
| App registry repo (`tbd-app-registry`) | Medium | External contributors submit apps via PR, CI-verified bundles |
| Interactive App Manager page | Medium | Browser-based app install/remove via Picoboot WebUSB |
| Sideloading support for custom apps | Low | Developers can build and run their own .uf2 without the registry — zero friction |
| Firmware ↔ app compatibility awareness | Medium | Version check warns when Pico SD apps may be outdated — never blocks the user |
| Single-app ↔ multi-app mode switching | Low | Flash nuke + bootloader as system tools on App Manager |
| Plugin adaptation for hardware UI (MultiFX) | High | ~15 plugins controllable from TBD-16 knobs/OLED; `hwui` block in `mui-*.json`; sub-range mapping with curve selection |
| Generalized MIDI API for RP2350 apps | Medium | Replaces legacy MidiParser; one API for MultiFX and future apps |
| MultiFX / Groovebox shared codebase | Medium | Same Ui, Knob, Display, Midi, SPI code — users learn one workflow |
| `.jsn` → `.json` file extension rename | Low | 122 files renamed to standard `.json`; mechanical rename, no logic changes — ✅ DONE |
| Plugin docs adaptation status | Low | `docs/plugins/index.rst` MIDI API column reflects which plugins work on hardware |

### What stays the same

- **Monorepo** — code, docs, WebUI, samples all in one repo
- **`sdcard_image/`** — stays in repo, always in sync with firmware
- **`sample_rom/`** — stays in repo, needed for factory builds
- **GitHub Pages** — artifact-based deploy (no `gh-pages` branch)
- **Fork model** — dadamachines/ctag-tbd is a fork of ctag-fh-kiel/ctag-tbd (upstream is dormant; we lead, they adopt when ready)

### What changes

- Old firmware/SD builds → **deleted** (no migration, no archive — they're pre-release test artifacts)
- Git history → **rewritten** (only 10 people need to re-clone)
- Remaining binaries → **Git LFS** (fast clones)
- Manual firmware builds → **CI pipeline** (reproducible, automated)
- Date-based firmware names → **semantic versioning** starting at `v0.4.0`
- No pre-release testing path → **staging channel** (CI-built, browser flash)
- 10 flash pages with 5,000 duplicated lines → **5 pages + shared JS module** (stable, staging, app manager, WebUI updates, troubleshooting)
- 10+ branches → **two long-lived branches** (`dada-tbd-master` + `staging`) + forks + PRs
- Informal upstream relationship → **Kconfig guards for portability** (upstream is dormant; they adopt our work when ready, not the other way around)
- No firmware/WebUI version coupling → **shared MAJOR.MINOR** (firmware `v0.4.x` ↔ WebUI `0.4.x`)
- Generic filenames → **`dada-{target}` naming** (every artifact identifies its hardware target)
- RP2350 apps unversioned → **tagged per release train** (app `0.4.x` ↔ firmware `0.4.x`)
- P4 has no knowledge of Pico app → **AnnounceApp SPI command** (existing on experimental, to be formalized)
- Multi-app requires manual setup → **interactive App Manager page** with Picoboot WebUSB install + catalog
- No curated Pico SD bundle → **optional Pico SD bundle per MAJOR.MINOR train** (BOOT2350 + all default apps; device ships factory pre-loaded, bundle available for restore/update)
- RP2350 runs standalone Groovebox by default → **multi-app one step away** (flash bootloader to unlock; Pico SD already has all apps)
- No version check between P4 and Pico apps → **firmware ↔ app compatibility awareness** (App Manager warns when Pico SD train ≠ firmware train with guided update; AnnounceApp runtime notice as safety net — never blocks)
- No single-app ↔ multi-app switching → **mode switching via flash nuke + bootloader** on App Manager page
- No external contributor path → **app registry repo** (`tbd-app-registry`) with PR-based submissions
- No sideloading workflow → **first-class sideloading** (build .uf2 with PlatformIO, drag-and-drop or copy to Pico SD — no registry required)
- No app catalog for users → **App Catalog on App Manager page** + downloadable `app-catalog.json`
- All 57 plugins WebUI-only → **~15 plugins adapted for hardware UI** via `hwui` block in `mui-*.json` (MultiFX controls them from knobs/OLED; per-parameter sub-range + curve mapping)
- All `.jsn` files → **renamed to `.json`** (standard extension; done early in Phase 1 to avoid path conflicts in later phases)
- Legacy MIDI Parser in RP2350 template → **generalized MIDI API** derived from Groovebox, simplified for MultiFX and future apps
- MultiFX as separate codebase → **shares UI/Knob/Display/Midi/SPI code with Groovebox** (users learn one workflow; page size adapts to hardware)
- No visibility into plugin hardware readiness → **MIDI API column in plugin docs** shows adapted / planned / n/a per plugin
