# Storage Architecture Refactor — Implementation Plan

> **Branch:** `feature-test/storage-arch-refactor` in both repos
> **Audit doc:** `docs/storage-architecture-audit.md` (Pico repo)
> **Started:** 2026-04-06

## Repos

| Repo | Path | Role |
|------|------|------|
| P4 (ESP32-P4) | `dadamachines-ctag-tbd` | SD card owner, overlay resolution, SPI responder |
| Pico (RP2350) | `tbd-pico-seq3` | Sequencer, UI, SPI requester |

---

## Decisions

| Decision | Choice |
|----------|--------|
| Top-left button menu name | **"Home"** (was "Project") — contains Project, Kit, Preset, Settings, Track Defaults |
| Track default sets | **3-5 global templates** stored on P4 SD (factory + user), max `MAX_TRACK_TEMPLATES` |
| Per-project track defaults | **Copy at creation time** — project gets a snapshot, then independent |
| Machine change in existing project | Doesn't affect saved data; **snapshot presets on save** (Phase 3) |
| Phase priority | **Phase 1 first:** No-Pico-SD + SPI project storage |
| Pico SD card | Not required. Bootloader-only. All user data on P4 SD via SPI |
| Naming convention | Hyphen-separated for new files (per audit) |

---

## Phase 0 — SD Card Restructure & Overlay Resolution

**Goal:** Restructure P4 SD card into factory/user/system zones with overlay resolution.

**Status: COMPLETE** ✅ — Committed `0b946b85` on 2026-04-06

### What was done

- [x] Create branch `feature-test/storage-arch-refactor` from `dada-tbd-master` (both repos)
- [x] Restructure `sdcard_image/` into overlay zones:
  ```
  sdcard_image/
  ├── factory/
  │   ├── presets/          ← from data/macrosoundpresets/
  │   ├── macros/           ← from data/macrodefinitions/
  │   ├── patches/          ← from data/sp/
  │   ├── trackdefaults/
  │   │   └── default.json  ← from data/trackdefaults.json
  │   └── synthdefinitions.json
  ├── user/
  │   ├── projects/         ← empty at factory
  │   ├── presets/           ← empty at factory
  │   ├── macros/            ← empty at factory
  │   ├── patches/           ← empty at factory
  │   ├── trackdefaults/     ← empty at factory
  │   ├── kits/              ← empty at factory
  │   ├── config/
  │   │   ├── device.json    ← was spm-config.json
  │   │   └── favorites.json ← was favs.json
  │   └── version.json       ← schema version
  ├── system/
  │   └── webui-version.json
  └── www/                   ← unchanged
  ```
- [x] Create `main/StorageOverlay.hpp` — overlay resolution + auto-migration
- [x] Update 13 source files for overlay paths (reads check user→factory, writes always to user)
- [x] `ctagSPDataModel.cpp` uses inline overlay (avoids circular component dependency)
- [x] `SampleAPI.cpp` has TODO for WebUI-coordinated migration
- [x] Update `create_sd_archive.sh` (new layout + legacy `/data/` backward compat)
- [x] P4 build verified clean

### On-device verification

- [x] P4 boots with new SD layout; overlay resolution serves presets correctly ✅ 2026-04-06
- [x] WebUI still loads (presets, macros, samples visible) ✅ 2026-04-06 — API v2 returns full plugin list
- [ ] Old-layout SD card auto-migrates on P4 boot (test with pre-refactor SD image)
- [x] Pico boots, gets track defaults via SPI 0xA5 — PicoSeqRack loaded correctly ✅ 2026-04-06

### Key files (Phase 0)

| File | Repo | Change |
|------|------|--------|
| `main/StorageOverlay.hpp` | P4 | NEW — overlay resolution, migration, directory helpers |
| `main/SPManager.cpp` | P4 | Calls `initOverlay()` at boot before model init |
| `main/SPManagerDataModel.hpp` | P4 | `MODELJSONFN` → `user/config/device.json` |
| `main/SPManagerDataModel.cpp` | P4 | Patches scan via `listMergedDir`, presets via overlay |
| `main/MacroSoundPresetDataModel.cpp` | P4 | Overlay read, user-only write |
| `main/MacroDeviceDefinitionDataModel.cpp` | P4 | Overlay read, user-only write |
| `main/FavoritesModel.cpp` | P4 | `user/config/favorites.json` |
| `main/SynthDefinitionDataModel.cpp` | P4 | `factory/synthdefinitions.json` |
| `main/SpiAPI.cpp` | P4 | Overlay trackdefaults, `system/webui-version.json` |
| `main/MacroAPI.cpp` | P4 | `read_all_json_overlay()` helper |
| `components/.../ctagSPDataModel.cpp` | P4 | Inline overlay for patches (avoids dep cycle) |
| `main/SampleAPI.cpp` | P4 | ✅ Fully overlay-migrated (resolveFile, userFilePath, scan_overlay_configs) |
| `create_sd_archive.sh` | P4 | New layout + legacy `/data/` for backward compat |

---

## Phase 0.5 — ctagSPDataModel Crash Fix

**Goal:** Fix crash on fresh SD when loading plugins whose user-patch files don't exist yet.

**Status: COMPLETE** ✅ — Applied 2026-04-06, committed with Phase 1 (`4645531e`)

### Root cause

`ctagSPDataModel` constructor set `mpFileName` to the **user write path** (`/sdcard/user/patches/mp-PicoSeqRack.json`) which doesn't exist on a fresh SD. `LoadPreset()` called `loadJSON(mp, mpFileName)` on the nonexistent file → corrupt rapidjson `Document` → `activePreset.CopyFrom()` triggered a **Guru Meditation Error: Load access fault** on Core 0.

### Fix

Split read/write paths:
- `mpFileName` = `resolveOverlayPatch(...)` — checks user dir first, falls back to factory
- `mpWriteFileName` = `userPatchPath(...)` — always writes to user dir
- After `storeJSON()`, set `mpFileName = mpWriteFileName` so subsequent reads use the user copy

### Files changed

| File | Change |
|------|--------|
| `components/ctagSoundProcessor/ctagSPDataModel.cpp` | Split read/write paths, use `mpWriteFileName` for `storeJSON()` |
| `components/ctagSoundProcessor/ctagSPDataModel.hpp` | Added `string mpWriteFileName` member |

---

## Phase 1 — SPI Project Storage (No Pico SD)

**Goal:** Move all project I/O from Pico SD to P4 SD via SPI. Pico SD card no longer required.

**Status: COMPLETE** ✅ — Committed `4645531e` (P4) / `d03181a` (Pico) on 2026-04-07

*Depends on Phase 0 (overlay resolution must be working) ✅*

### P4 repo

- [x] Add new SPI commands in `SpiAPI.hpp` / `SpiAPI.cpp`:
  - `0xB0 SaveProjectToP4` — receive binary from Pico, write to `/user/projects/projectXXX.bin`
  - `0xB1 LoadProjectFromP4` — read binary from user (fallback factory), stream to Pico
- [x] Additional SPI commands:
  - `0xB2 ListProjects` — scan `/user/projects/` + `/factory/projects/`, return JSON list
  - `0xB3 DeleteProject` — remove project from `/user/projects/`
  - `0xB4 SavePicoConfig` — receive sequencer config binary, atomicWrite to `/user/config/sequencer.bin`
  - `0xB5 LoadPicoConfig` — read sequencer config, stream to Pico
- [x] Implement chunked SPI binary transfer via `transmitBinary()` — 2041 bytes/frame, fingerprint + ACK
- [x] Create `/user/projects/` directory on first save (mkdir in SaveProjectToP4 handler)
- [x] Implement temp-file + rename write pattern for atomic saves (`atomicWrite()` helper)
  - FAT32 rename cannot overwrite — must `remove()` destination before `rename()`
- [ ] Project metadata (`project.json`): name, date, firmware version, format version
- [ ] Migrate file naming from `projectXXX.bin` to `{id}/song.psng` (audit naming convention)

### Pico repo

- [x] Add command IDs `0xB0`–`0xB5` in `SpiAPI.h` (enum `RequestType_t`)
- [x] Add SPI methods:
  - `SaveProjectToP4(slotName, data, size)` — chunked binary send
  - `LoadProjectFromP4(slotName, data, maxSize, actualSize)` — chunked binary receive
  - `ListProjects()` — request JSON list from P4
  - `DeleteProject(slotName)` — delete from P4 user dir
  - `SavePicoConfig(data, size)` — send config binary to P4
  - `LoadPicoConfig(data, maxSize, actualSize)` — receive config from P4
  - `transmitBinaryData()` / `receiveBinaryData()` — private transport helpers
- [x] Rewire `project_saveto_fs()` → `spi_api.SaveProjectToP4()` (was `storage->writeFile()`)
- [x] Rewire `project_loadfrom_fs()` → `spi_api.LoadProjectFromP4()` (was `storage->readFile()`)
- [x] Make Pico SD card init conditional: `picoStorage = nullptr`, added `isInitialized()` to `SdCardHW`
- [x] Create `SpiStorage` class (`IStorageInterface`) routing config I/O to P4 SD via SPI
- [x] Wire `sequi.storage = spiStorage` — config load/save goes through SPI
- [x] Boot flow: `loadOrInitConfig()` → `LoadPicoConfig()` from P4; if fails, hardcoded defaults
- [x] Modify project list scanning — replace SD card `EnumFiles` with `SpiAPI::ListProjects()`
- [x] Update `project_load.cpp` / `project_save.cpp` screens to use P4-based file list
- [x] Add delete project UI flow (load screen → "Delete" option → confirm → SPI 0xB3)
- [x] Add `BACKGROUNDUPDATE_TYPE_DELETEPROJECT` handler in background loop

### Verification (Phase 1)

- [x] **Save a project** on Pico OLED → file written to P4 `/user/projects/` via SPI ✅ 2026-04-06
- [x] **Load the saved project** → sequence plays back correctly ✅ 2026-04-06
- [x] Project list on OLED shows projects from P4 (both `/user/` and `/factory/`)
- [x] Delete a project from OLED → file removed from P4 SD
- [x] Device boots and operates with NO Pico SD card inserted ✅ 2026-04-07
- [x] Sequencer config (MIDI routing, display prefs) persists across reboots via P4 ✅ 2026-04-07
  - LoadPicoConfig: 64 bytes from `/user/config/sequencer.bin`
  - SavePicoConfig: atomicWrite back to P4 on boot
- [x] Power-cycle during save does not corrupt existing projects (atomic write) ✅ 2026-04-07
- [ ] SPI transfer of 60 KB project completes in < 200 ms

---

## Phase 2 — Track Default Templates

**Goal:** 3-5 global track default templates (factory + user-created), selectable from project menu.
Per-project: user chooses which template to apply, or defines custom track setup.

**Status:** COMPLETE (core) ✅ — SPI commands done, UI complete, factory templates deployed

*Depends on Phase 1 (project storage on P4 must work) ✅*

### P4 repo

- [x] Add SPI commands for track default template management:
  - `0xB6 ListTrackDefaults` — scan overlay trackdefaults, return JSON array with factory/user flags
  - `0xB7 GetTrackDefault(name)` — return JSON content of a named template via overlay
  - `0xB8 SaveTrackDefault(name, json)` — write to `/user/trackdefaults/{name}.json`
  - `0xB9 DeleteTrackDefault(name)` — remove from `/user/trackdefaults/` (factory immutable)
- [x] Modify existing `GetTrackDefaultPresets` (0xA5) handler:
  - Accept optional template name in `string_param_3`
  - If empty, read active template name from `/user/config/active-trackdefault.txt`
  - Falls back to `"default"` if neither provided
  - Resolve template via overlay, return JSON
- [x] Create additional factory templates in `sdcard_image/factory/trackdefaults/`
  - `default.json` — full drum machine + synth setup ✅
  - `minimal.json` — fewer drums, more synth focus ✅
  - `sampler.json` — all rompler tracks for sample-based production ✅

### Pico repo

- [x] Add SPI wrapper methods: `ListTrackDefaults()`, `GetTrackDefault()`, `SaveTrackDefault()`, `DeleteTrackDefault()`
- [x] Add `activeTrackDefault[24]` to `SEQCONFIGFILE`, bump version to 105
  - Default value: `"default"`
  - Old v104 configs auto-rejected (size mismatch), defaults applied
- [x] Modify `GetTrackDefaultPresets()` to accept optional `templateName` parameter
- [x] Modify `fetchAndInitTrackDefaults()` to accept optional template name
- [x] Add `BACKGROUNDUPDATE_TYPE_APPLYTRACKDEFAULT` handler — calls `fetchAndInitTrackDefaults()` + `initializeMacroPresetOnTrack()` for all tracks
- [x] Add `applyTrackDefault()` to `IHostInterface` / `PicoHost`
- [x] Add **"Track setup"** menu item to project screen (item 5, before System settings)
- [x] Create `tracksetup.hpp` / `tracksetup.cpp` — template list screen:
  - `enter()` fetches template list from P4 via `trackdefault_updatelist()`
  - Shows list of available templates with factory indicator "(F)"
  - Select → options: "Back", "Apply", "Set as default"
  - "Apply" calls `host->applyTrackDefault(name)` for current project
  - "Set as default" saves to config + applies
- [x] Rename top-left menu from "Project" to **"Home"**
- [x] Show factory templates with (F) indicator (vs. user templates)
- [x] "Set as device default" option (writes to Pico config, auto-saved)
- [ ] Track setup editor screen — per-track machine view / modify
- [ ] "Save as new template" — capture current track assignment as user template
- [x] Modify "Clear Project" flow to apply active track default template
  - `project_reset()` passes `sequi.config.activeTrackDefault` to `fetchAndInitTrackDefaults()`
- [ ] Store template name in project metadata when saving

### Verification (Phase 2)

- [x] Project menu shows "Track setup" entry
- [x] Home screen renamed from "Project" to "Home" with reordered items
- [x] Selecting opens template list with names from P4 (3 factory: default, minimal, sampler)
- [x] Selecting a template applies machines + presets to all tracks
- [ ] "Save as new" creates a user template visible in the list (Phase 11)
- [x] Factory templates cannot be deleted (no delete UI; P4-side protection on 0xB9)
- [x] New Song flow applies active template (`project_reset()` uses `activeTrackDefault`)
- [x] "Set as device default" persists across reboots (writes to config, auto-saved)

---

## Phase 2.5 — Home Menu Redesign & 16-Slot Projects

**Goal:** Professional Home menu with visual groups, project context in title, 16 project slots mapped to step buttons, and consistent naming.

**Status:** First iteration COMPLETE ✅

*Depends on Phase 2 (track default templates must be working)*

### Design (see `docs/home-menu-design-proposal.md` in Pico repo)

```
── [Slot N / Untitled] ──────
  Save                          ← most common action first (hardware convention)
  Load
  New
  - - - - - - - - - - - - - -   ← dashed separator (non-selectable)
  Kit
  Track setup
  Save preset
  - - - - - - - - - - - - - -   ← dashed separator (non-selectable)
  Settings
```

### Changes (all in Pico repo)

- [x] Home menu: visual groups with dashed separators, cursor skips separator items
- [x] Title shows project context: "Slot N" (loaded) or "Untitled" (fresh)
- [x] Menu order: Save → Load → New (most common first)
- [x] Shorter labels: "Save", "Load", "New", "Kit", "Settings"
- [x] `MAX_PROJECT_SLOTS` 5 → 16, mapped to step buttons
- [x] Save/Load screens: step button LEDs (bright=occupied, dim=empty)
- [x] Save/Load screens: step button press = quick slot select
- [x] "Clear project?" → "New project?" 
- [x] "System menu" → "Settings" (consistent naming)
- [x] Design proposal doc updated with rationale and complete menu tree
- [ ] Context-aware warnings for destructive actions in saved projects (future)
- [ ] Project naming / renaming (future)

---

## Phase 3 — Self-Contained Projects (Preset Snapshots)

**Goal:** Projects restore the correct sounds when loaded. Parameter values are preserved exactly as saved.

**Status:** Phase 3a complete (sound restoration + parameter values). Phase 3b (file-level snapshots) deferred.

*Depends on Phase 1. Independent of Phase 2.*

### Phase 3a — Sound Restoration (Complete)

**Bug fix:** `project_loadfrom_fs()` previously called `LoadTrackMacroDefinition` (0xAA) which only
configured the synth engine with factory-default parameter values. Projects loaded but sounded wrong.

**Fix:** Now calls `LoadTrackSoundPreset` (0xA4) per track using the stored `soundPresetId` from the
project binary, which properly configures the synth engine and applies preset parameter values.

**Enhancement:** New SPI command `SetTrackParamValues` (0xBA) sends the exact saved parameter values
(from the project binary blob) to the P4 after loading the preset. This overrides preset defaults with
the user's exact tweaked values from save time.

#### P4 repo
- [x] Add SPI command `0xBA SetTrackParamValues(trackIndex, count, values[])` — batch-set track
      parameter values via `SoundProcessorManager::SetTrackParameter()`

#### Pico repo
- [x] Fix `project_loadfrom_fs()`: use `LoadTrackSoundPreset` (0xA4) instead of `LoadTrackMacroDefinition` (0xAA)
- [x] After preset load, send saved parameter values via `SetTrackParamValues` (0xBA)
- [x] Fallback to `LoadTrackMacroDefinition` for legacy projects without `soundPresetId`

### Phase 3b — File-Level Preset Snapshots (Deferred)

**Goal:** Preset and macro definition JSON files are snapshotted into project folders at save time.
Projects are resilient to firmware changes that modify preset files.

*Note: Command codes 0xB6/0xB7 from the original plan are used by Phase 2 (ListTrackDefaults/GetTrackDefault). New commands would start at 0xBB+.*

#### P4 repo
- [ ] On `SaveProjectToP4` (0xB0), extend handler:
  - For each track: save preset + macro def JSON to `/user/projects/{id}/snapshots/`
- [ ] On `LoadProjectFromP4` (0xB1), extend handler:
  - Compare saved snapshots with current presets
  - Return a "mismatch" flag per track if preset has changed
- [ ] Content-addressed kit store (future)

#### Pico repo
- [ ] On load: check mismatch flags; if any:
  - Show OLED warning: "Presets changed since save"
  - Options: "Load saved" / "Use current"

### Verification (Phase 3a)

- [x] Load project → P4 synth engine configured with correct preset (not just macro defaults)
- [x] Load project → P4 parameter values match saved state (not preset defaults)
- [x] Legacy projects without `soundPresetId` → falls back to `LoadTrackMacroDefinition`
- [x] Both Pico and P4 build successfully

---

## ~~Phase 4 — Migration, Polish & Backup Foundation~~ (REMOVED)

> Phase 4 has been removed from scope. With only 5 beta users and no legacy Pico SD
> projects to migrate, auto-migration is unnecessary. REST storage API may be
> revisited as a future WebUI feature.

---

## Phase 5 — WebUI Config Path Adaptation

**Goal:** Fix WebUI JavaScript to use the new overlay subdir names. Currently the backend expects
`presets/`, `macros/`, `patches/` but the WebUI JS still sends the legacy `macrosoundpresets/`,
`macrodefinitions/` paths — resulting in 404 errors for all config save/load operations.

**Status: COMPLETE** ✅ — Committed `a1662e40` on 2026-04-07

**Priority: HIGH** — The WebUI preset/macro editor was broken against the new backend.

*Depends on Phase 0 (overlay paths must be deployed). Independent of all other phases.*

> **Reference:** `docs/webui-storage-handover.md` documents the backend contract.

### Path renaming required

| Old JS path prefix | New overlay subdir | Occurrences |
|---------------------|--------------------|-------------|
| `macrosoundpresets/` | `presets/` | 18 (4 files) |
| `macrodefinitions/` | `macros/` | 6 (4 files) |
| **Total** | | **24** |

### Files to change (all in `sdcard_image/www/js/`)

| File | `macrosoundpresets` refs | `macrodefinitions` refs | Notes |
|------|-------------------------|------------------------|-------|
| `designer.js` | 4 | 2 | Preset save/load, macro def upload |
| `macro-bundle.js` | 9 | 3 | Macro editor: preset + def CRUD |
| `performer.js` | 4 | 1 | Live preset load/save |
| `app-bundle.js` | 1 (comment) | 0 | Comment only — update for accuracy |

### Tasks

- [x] Search-replace `'macrosoundpresets/'` → `'presets/'` in all JS files
- [x] Search-replace `'macrodefinitions/'` → `'macros/'` in all JS files
- [x] Update any stale comments referencing old paths (`/sdcard/data/`, `sp/`)
- [x] Regenerate .gz bundles for app-bundle.js and macro-bundle.js
- [ ] Verify in browser: preset save/load works, macro editor works
- [x] Fix SampleAPI.cpp `getconfig` handler for files without subdir (synthdefinitions.json)

### Verification (Phase 5)

- [ ] WebUI loads preset list (getconfig with `presets/` subdir)
- [ ] Saving a preset from designer writes to `/user/presets/`
- [ ] Macro definition editor loads and saves correctly
- [ ] No 404 errors in browser console for config operations
- [ ] Factory presets still visible via overlay (read from `/factory/presets/`)

---

## Phase 6 — Generic Storage REST API

**Goal:** Expose P4 SD card file operations via HTTP for backup, restore, project browsing, and
storage statistics. This is the foundation for WebUI backup/restore and any future data management.

**Status:** NOT STARTED

**Priority: MEDIUM** — Prerequisite for WebUI backup/restore (Phase 7) and storage indicator.

*Depends on Phase 0 (overlay zones). Independent of Phases 5, 3b.*

> **Reference:** Original audit §8.4 specifies these endpoints.

### New endpoints (register in `RestServer.cpp`)

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/v2/storage/info` | SD card total/used/free bytes, per-zone breakdown |
| GET | `/api/v2/storage/list?path=&recursive=` | Directory listing with file sizes + timestamps |
| GET | `/api/v2/storage/file?path=` | Raw file download (binary or JSON) |
| POST | `/api/v2/storage/file?path=` | Upload/overwrite file (multipart or raw body) |
| DELETE | `/api/v2/storage/file?path=` | Delete file |
| POST | `/api/v2/storage/mkdir?path=` | Create directory |
| POST | `/api/v2/storage/copy?from=&to=` | Server-side file copy |
| POST | `/api/v2/storage/reload` | Flush caches, re-scan overlay |

### Security constraints

- [ ] **Read-only `/factory/`** — reject POST/DELETE to any path under `/factory/`
- [ ] **Path traversal protection** — reject `..` components, normalize paths
- [ ] **Reject writes to `/system/`** — reserved for firmware use
- [ ] **Size limit on uploads** — enforce reasonable per-file max (e.g., 1 MB)
- [ ] Wrap all write operations in `lockStorage()` / `unlockStorage()`

### Tasks

- [ ] Create `main/StorageAPI.cpp` / `.hpp` — REST handlers for the new endpoints
- [ ] Register routes in `RestServer.cpp`
- [ ] Implement `getSDCardInfo()` — `FATFS` stats for total/used/free
- [ ] Implement recursive directory listing with file metadata (size, timestamp)
- [ ] Implement file GET/POST/DELETE with overlay-aware path resolution
- [ ] Implement mkdir, copy, reload
- [ ] Add unit test: path traversal rejection (`../../etc/passwd`, leading `/`)
- [ ] Integration test: upload file → list → download → verify content match

### Verification (Phase 6)

- [ ] `curl /api/v2/storage/info` returns SD card stats
- [ ] `curl /api/v2/storage/list?path=user/presets/` returns JSON file list
- [ ] Upload + download roundtrip preserves file content exactly
- [ ] Cannot write to `/factory/` (403 Forbidden)
- [ ] Cannot path-traverse outside SD root (400 Bad Request)

---

## Phase 7 — WebUI Backup & Restore

**Goal:** Browser-side backup and restore of all user data via the Storage REST API.
No archive creation on the MCU — JSZip runs in the browser, streaming files over HTTP.

**Status:** NOT STARTED

**Priority: MEDIUM** — Protects user data across firmware updates.

*Depends on Phase 6 (Storage REST API must exist).*

> **Reference:** Original audit §7 specifies the browser-side ZIP approach.

### Backup flow

1. WebUI fetches `/api/v2/storage/list?path=user/&recursive=true`
2. For each file: `GET /api/v2/storage/file?path=user/{filepath}`
3. JSZip assembles all files into `dada-tbd-backup-{date}.zip`
4. Browser triggers download

### Restore flow

1. User uploads ZIP via file picker
2. JSZip extracts in browser
3. For each file: `POST /api/v2/storage/file?path=user/{filepath}`
4. `POST /api/v2/storage/reload` to re-scan overlay
5. Confirmation: "Restored N files, M bytes"

### Tasks

- [ ] Add JSZip library to WebUI (`sdcard_image/www/js/jszip.min.js`)
- [ ] Create backup panel UI (modal overlay in sample-manager or new page)
- [ ] Implement backup: recursive list → parallel file downloads → ZIP assembly
- [ ] Implement restore: ZIP extract → confirm overwrite → parallel uploads → reload
- [ ] Add "Factory reset" option: `DELETE /api/v2/storage/file?path=user/` (recursive) → reload
- [ ] Progress indicator during backup/restore (file count, byte progress)
- [ ] Add backup reminder prompt before firmware updates (web flasher integration)

### WebUI Storage Indicator

- [ ] Add storage usage widget (total/used/free) to sample-manager header
- [ ] Per-zone breakdown: projects, presets, macros, patches, kits, samples
- [ ] Color-coded bar: green < 70%, yellow 70-90%, red > 90%

### Verification (Phase 7)

- [ ] Full backup creates valid ZIP with all user files
- [ ] Restore from ZIP recreates user directory tree
- [ ] Factory reset clears `/user/` and restores factory-only state
- [ ] Storage indicator updates after backup/restore operations
- [ ] Backup/restore works with large data sets (100+ files, 10+ MB total)

---

## Phase 8 — WebUI Overlay-Aware Browsing

**Goal:** WebUI displays factory vs. user content distinctly. Users can reset individual
items to factory defaults. Sample pool, preset list, and macro list show provenance.

**Status:** NOT STARTED

**Priority: LOW** — Quality-of-life improvement, not blocking any workflow.

*Depends on Phase 5 (config paths) and Phase 6 (storage REST API for metadata).*

> **Reference:** Original audit §8.2 describes overlay-aware browsing.

### Tasks

- [ ] Preset list: show factory badge "(F)" for items from `/factory/presets/`
- [ ] Macro list: show factory badge for items from `/factory/macros/`
- [ ] Sample pool: distinguish factory vs. user-uploaded samples
- [ ] "Reset to factory" context menu: delete user override → overlay falls through to factory
- [ ] "Duplicate" option: copy factory item to user layer for editing
- [ ] Indicate overridden items: user file shadows a factory file with same name

### Verification (Phase 8)

- [ ] Factory presets show "(F)" badge, user presets do not
- [ ] "Reset to factory" removes user override, factory version reappears
- [ ] Editing a factory preset auto-copies to user layer (no factory mutation)

---

## Phase 9 — Project Metadata & File-Level Snapshots

**Goal:** Complete Phase 3b. Projects include metadata and snapshotted preset/macro JSON
files for full self-containment and resilience to firmware changes.

**Status:** NOT STARTED

**Priority: LOW** — Useful for project portability, but parameter values (Phase 3a) cover the
most common "sounds wrong after load" scenario.

*Depends on Phase 3a (sound restoration must work). Phase 1 (SPI project storage) complete.*

### Project metadata (`project.json`)

Each project directory gets a `project.json`:
```json
{
  "name": "My Song",
  "created": "2026-04-10T12:00:00Z",
  "modified": "2026-04-10T14:30:00Z",
  "firmwareVersion": "1.2.0",
  "formatVersion": 1,
  "templateName": "default",
  "tracks": [
    { "slot": 0, "presetId": "808-kick", "macroDefId": "PicoSeqRack" },
    ...
  ]
}
```

### Tasks

#### P4 repo
- [ ] On `SaveProjectToP4` (0xB0): create/update `project.json` alongside `song.psng`
- [ ] On `SaveProjectToP4`: snapshot each track's preset JSON + macro def JSON into
      `/user/projects/{id}/snapshots/{trackN}-preset.json`, `{trackN}-macrodef.json`
- [ ] On `LoadProjectFromP4` (0xB1): compare snapshot hashes with current files,
      return per-track mismatch flags in response
- [ ] Migrate project directory naming: `projectXXX.bin` → `{id}/song.psng` + `project.json`

#### Pico repo
- [ ] Send track metadata (presetId, macroDefId per track) with save command
- [ ] On load: if mismatch flags present, show OLED warning:
      "Presets changed since save" → "Load saved" / "Use current"
- [ ] Store/display project name on OLED (from `project.json`)
- [ ] Project naming/renaming screen

### Verification (Phase 9)

- [ ] Saved project directory contains `project.json` + snapshots
- [ ] Loading detects when preset has been modified since save
- [ ] User can choose saved vs. current preset on mismatch
- [ ] Project name displays on Pico OLED

---

## Phase 10 — Content-Addressed Kit Store

**Goal:** Kits (sample/wavetable collections) are stored as immutable, content-addressed
snapshots. Projects reference kits by hash for reproducibility.

**Status:** NOT STARTED

**Priority: LOW** — Advanced feature for kit versioning and deduplication.

*Depends on Phase 6 (Storage REST API) for kit management WebUI.*

> **Reference:** Original audit §4.5–4.6 specifies the content-addressed store.

### Architecture

```
/user/kits/
├── store/
│   └── {xxh128-hash}/       ← immutable snapshot directory
│       ├── manifest.json    ← file list + per-file hashes
│       └── samples/         ← actual audio files
├── heads/
│   └── {kitname}.json       ← mutable pointer: { "hash": "...", "name": "..." }
└── incoming/                ← temp staging for uploads
```

### Tasks

- [ ] Implement XXH128 hashing for kit directories (use existing xxHash in codebase)
- [ ] On kit upload/save: hash content → store in `/user/kits/store/{hash}/`
- [ ] Create `manifest.json` per kit snapshot (file list, sizes, individual hashes)
- [ ] Implement mutable heads: `/user/kits/heads/{name}.json` → `{ "hash": "..." }`
- [ ] Garbage collection: scan project references → mark used hashes → delete unreferenced
- [ ] SPI commands for kit hash lookup: `GetKitHash(name)` → hash string
- [ ] Store kit hash in `project.json` for reproducibility
- [ ] WebUI: kit editor shows current hash, history of snapshots, diff view
- [ ] WebUI: kit GC button with "will free N MB" estimate

### Verification (Phase 10)

- [ ] Modifying a kit creates new hash, old snapshot preserved
- [ ] Two identical kits produce same hash (deduplication)
- [ ] GC removes unreferenced snapshots, preserves referenced ones
- [ ] Project load verifies kit hash matches

---

## Phase 11 — Polish & Extras

**Goal:** Final quality items from the original audit that don't fit neatly into other phases.

**Status:** NOT STARTED

**Priority: LOW** — Nice-to-haves for production readiness.

### Tasks

- [ ] Factory demo projects: create 3-5 showcase projects in `/factory/projects/`
  - Different genres/styles demonstrating machine capabilities
  - Include matching presets and kit data
- [ ] Pre-deploy backup prompt in web flasher (`tbd-flasher-p4.js`)
  - Before firmware flash: "Back up your data? [Download backup] [Skip]"
  - Uses backup API from Phase 7
- [ ] Track setup editor screen on Pico OLED
  - Per-track view: machine name, preset name, volume
  - Modify machine assignment per track
- [ ] "Save as new template" on Pico OLED
  - Capture current track assignment as user template in `/user/trackdefaults/`
- [ ] SPI transfer CRC — end-to-end integrity check on binary payloads
- [ ] Song file CRC32 in footer — detect corruption on load
- [ ] Data format versioning framework
  - `version.json` tracks schema version per data type
  - Sequential migration functions for format upgrades
  - Auto-migrate on boot if version mismatch detected

---

## Key Technical Considerations

1. **SPI chunked transfer** — ✅ VERIFIED. Current frame is 2048 bytes with 7-byte header = 2041 payload. A 60 KB project needs ~30 frames. `transmitData()` / `receiveData()` handle multi-frame correctly.

2. **Track default template format** — ✅ WORKING. Template JSON served via overlay resolution. 0xA5 handler accepts optional template name parameter.

3. **Concurrent access** — ✅ IMPLEMENTED. Storage mutex (`lockStorage()`/`unlockStorage()` in `StorageOverlay.hpp`) protects all file write operations in `SpiAPI.cpp` (5 write blocks) and `SampleAPI.cpp` (3 write points). Per-class `arrayMutex` protects in-memory arrays in `MacroSoundPresetDataModel` and `MacroDeviceDefinitionDataModel` from concurrent SPI + HTTP access.

4. **Atomic writes** — ✅ IMPLEMENTED. `atomicWrite()` in `SpiAPI.cpp` uses temp-file + rename pattern. FAT32 rename cannot overwrite — must `remove()` before `rename()`. All SPI write operations use `atomicWrite()`, wrapped in storage mutex.

5. **WebUI path breakage** — ⚠️ CRITICAL. Backend overlay expects `presets/`, `macros/` subdir names but WebUI JS still sends `macrosoundpresets/`, `macrodefinitions/`. No backward-compat shim exists — results in 404 errors. Must fix in Phase 5 before any WebUI testing.

6. **Storage REST API security** — Phase 6 endpoints must enforce: path traversal rejection, `/factory/` read-only, `/system/` write-protected, upload size limits. All writes through `lockStorage()`.

---

## Relevant Files Reference

### P4 repo (`dadamachines-ctag-tbd`)

| File | Purpose |
|------|---------|
| `main/StorageOverlay.hpp` | Overlay resolution, migration, directory helpers |
| `main/SpiAPI.hpp` / `.cpp` | SPI command handlers, commands 0xB0–0xBA |
| `main/SpiProtocol.h` | Low-level SPI frame structures |
| `main/SPManager.cpp` | Boot sequence, `initOverlay()` call |
| `main/MacroAPI.cpp` | Preset/macro REST handlers |
| `main/SampleAPI.cpp` | Sample + config file I/O (overlay-migrated ✅) |
| `main/RestServer.cpp` | HTTP handler registration |
| `main/StorageAPI.cpp` | **NEW (Phase 6):** Generic storage REST handlers |
| `docs/webui-storage-handover.md` | Backend changes documented for WebUI developer |
| `sdcard_image/` | Factory SD card image layout |
| `sdcard_image/www/js/designer.js` | WebUI designer — preset/macro config paths |
| `sdcard_image/www/js/macro-bundle.js` | WebUI macro editor — preset/macro config paths |
| `sdcard_image/www/js/performer.js` | WebUI performer — preset config paths |
| `sdcard_image/www/js/app-bundle.js` | WebUI main bundle |
| `sdcard_image/www/js/sample-manager.js` | WebUI sample manager |
| `sdcard_image/www/js/shared.js` | WebUI shared utilities |
| `create_sd_archive.sh` | SD card ZIP creation for releases |

### Pico repo (`tbd-pico-seq3`)

| File | Purpose |
|------|---------|
| `src/SpiAPI.h` / `.cpp` | SPI command methods (0xB0–0xBA) |
| `src/SpiProtocol.h` | Command ID constants |
| `src/PicoHost.h` / `.cpp` | `saveProject()` / `loadProject()` rewiring |
| `src/PicoStorage.h` / `.cpp` | Make SD-optional, keep for bootloader |
| `src/SdCardHW.cpp` | Conditional SD init |
| `examples/main.cpp` | `fetchAndInitTrackDefaults()`, boot flow |
| `lib/sequencerui/screens/project.cpp` | Home menu (renamed from "Project") |
| `lib/sequencerui/screens/project_load.cpp` | SPI-based project list |
| `lib/sequencerui/screens/project_save.cpp` | SPI-based save |
| `lib/sequencerui/screens/tracksetup.cpp` | Track default template browser |

---

## Scope Boundaries

**Included (Phases 0–11):**
- SD restructure & overlay resolution (P4 backend) ✅
- SPI project storage, no-Pico-SD operation ✅
- Track default templates (SPI + Pico UI) ✅
- Home menu redesign, 16 project slots ✅
- Sound restoration & parameter values (Phase 3a) ✅
- Thread safety (storage mutex + array mutexes) ✅
- WebUI config path adaptation (Phase 5) — **CRITICAL, next**
- Generic Storage REST API (Phase 6)
- WebUI Backup & Restore (Phase 7)
- WebUI overlay-aware browsing (Phase 8)
- Project metadata & file-level snapshots (Phase 9)
- Content-addressed kit store (Phase 10)
- Factory demo projects, CRC integrity, data versioning (Phase 11)

**Excluded:**
- Pico SD auto-migration (only 5 beta users, not worth the complexity)
- Pico SD backup / directory mirror (no Pico SD required)
- Real-time sample streaming from P4 SD (out of scope for sequencer)
- Multi-user / authentication (single-device, local network only)

---

## Progress Summary

> Updated 2026-04-XX

| Phase | Name | Status | Priority |
|-------|------|--------|----------|
| 0 | SD Restructure & Overlay | ✅ COMPLETE | — |
| 0.5 | ctagSPDataModel Crash Fix | ✅ COMPLETE | — |
| 1 | SPI Project Storage | ✅ COMPLETE | — |
| 2 | Track Default Templates | ✅ COMPLETE (core) | — |
| 2.5 | Home Menu & 16 Slots | ✅ COMPLETE | — |
| 3a | Sound Restoration | ✅ COMPLETE | — |
| 3b | File-Level Snapshots | → Moved to Phase 9 | LOW |
| ~~4~~ | ~~Migration & Backup~~ | REMOVED | — |
| 5 | WebUI Config Paths | ✅ COMPLETE | — |
| 6 | Storage REST API | NOT STARTED | MEDIUM |
| 7 | WebUI Backup & Restore | NOT STARTED | MEDIUM |
| 8 | WebUI Overlay Browsing | NOT STARTED | LOW |
| 9 | Project Metadata & Snapshots | NOT STARTED | LOW |
| 10 | Content-Addressed Kit Store | NOT STARTED | LOW |
| 11 | Polish & Extras | NOT STARTED | LOW |

### Dependency graph

```
Phase 0 ─┬─ Phase 1 ── Phase 3a ✅
          │      └──── Phase 2 ── Phase 2.5 ✅
          │
          ├─ Phase 5 (WebUI paths) ◄── NEXT: fixes broken WebUI
          │
          ├─ Phase 6 (Storage REST API)
          │      ├──── Phase 7 (Backup/Restore)
          │      ├──── Phase 8 (Overlay Browsing)
          │      └──── Phase 10 (Kit Store)
          │
          └─ Phase 9 (Metadata/Snapshots) ← depends on Phase 3a
                 └──── Phase 11 (Polish)
```
