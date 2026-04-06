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
| `main/SampleAPI.cpp` | P4 | TODO marker only (needs WebUI coordination) |
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

**Status:** IN PROGRESS — SPI commands done, basic UI started

*Depends on Phase 1 (project storage on P4 must work) ✅*

### P4 repo

- [x] Add SPI commands for track default template management:
  - `0xB6 ListTrackDefaults` — scan overlay trackdefaults, return JSON array of template names
  - `0xB7 GetTrackDefault(name)` — return JSON content of a named template via overlay
  - `0xB8 SaveTrackDefault(name, json)` — write to `/user/trackdefaults/{name}.json`
  - `0xB9 DeleteTrackDefault(name)` — remove from `/user/trackdefaults/` (factory immutable)
- [x] Modify existing `GetTrackDefaultPresets` (0xA5) handler:
  - Accept optional template name in `string_param_3`
  - If empty, read active template name from `/user/config/active-trackdefault.txt`
  - Falls back to `"default"` if neither provided
  - Resolve template via overlay, return JSON
- [ ] Create additional factory templates in `sdcard_image/factory/trackdefaults/`
  - `default.json` exists ✅
  - Optionally: 1-2 additional templates (techno, ambient, minimal)

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
  - Shows list of available templates
  - Select → "Apply?" confirmation → `host->applyTrackDefault(name)`
- [ ] Rename top-left menu from "Project" to **"Home"**
- [ ] Show factory templates with lock icon (vs. user templates)
- [ ] "Set as device default" option (writes to Pico config)
- [ ] Track setup editor screen — per-track machine view / modify
- [ ] "Save as new template" — capture current track assignment as user template
- [ ] Modify "Clear Project" flow to apply active track default template
- [ ] Store template name in project metadata when saving

### Verification (Phase 2)

- [x] Project menu shows "Track setup" entry
- [ ] Selecting opens template list with names from P4
- [ ] Selecting a template applies machines + presets to all tracks
- [ ] "Save as new" creates a user template visible in the list
- [ ] Factory templates cannot be deleted
- [ ] New Song flow applies active template
- [ ] "Set as device default" persists across reboots

---

## Phase 3 — Self-Contained Projects (Preset Snapshots)

**Goal:** Preset and macro definitions are snapshotted into project folders at save time. Projects are resilient to firmware changes.

**Status:** Not started

*Depends on Phase 1. Independent of Phase 2.*

### P4 repo

- [ ] Add SPI commands:
  - `0xB6 GetProjectPresetSnapshot(trackIndex)` — return preset + macro def JSON for a track
  - `0xB7 GetActiveKitHash` — return XXH128 of active kit binary
- [ ] On `SaveProjectToP4` (0xB0), extend handler:
  - For each track: save preset + macro def JSON to `/user/projects/{id}/snapshots/`
  - Record kit hash in `project.json`
- [ ] On `LoadProjectFromP4` (0xB1), extend handler:
  - Compare saved snapshots with current presets
  - Return a "mismatch" flag per track if preset has changed
- [ ] Implement content-addressed kit store:
  - `/user/kits/store/{hash}/` — immutable kit snapshots
  - `/user/kits/heads/{name}.json` — mutable head pointers
- [ ] Kit garbage collection endpoint (REST + SPI)

### Pico repo

- [ ] On save: request snapshot data from P4 per track, include in save bundle
- [ ] On load: check mismatch flags; if any:
  - Show OLED warning: "Presets changed since save"
  - Options: "Load saved" / "Use current"
- [ ] Add kit hash display in project info screen

### Verification (Phase 3)

- [ ] Save project → `/user/projects/{id}/snapshots/` contains preset JSONs
- [ ] Save project → `project.json` contains `kitHash`
- [ ] Modify preset → load project → OLED shows mismatch warning
- [ ] Kit modification creates new hash; old projects still reference old hash
- [ ] GC removes unreferenced kit store entries

---

## Phase 4 — Migration, Polish & Backup Foundation

**Goal:** Auto-migrate existing Pico SD projects, add REST storage API, prepare for WebUI backup.

**Status:** Not started

*Depends on Phase 1*

### P4 repo

- [ ] Implement generic storage REST API (6 endpoints from audit Section 8.4):
  - `GET /api/v2/storage/info`
  - `GET /api/v2/storage/list?path=`
  - `GET /api/v2/storage/file?path=`
  - `POST /api/v2/storage/file?path=`
  - `DELETE /api/v2/storage/file?path=`
  - `POST /api/v2/storage/mkdir?path=`
- [ ] Security: reject path traversal (`../`), reject writes to `/factory/` and `/system/`
- [ ] Pre-deploy backup prompt in web flasher

### Pico repo

- [ ] Auto-migration on first boot after update:
  - Detect `/lastconfig.bin` or `/project*.bin` on Pico SD (if card present)
  - OLED: "Migrating projects to device storage..."
  - Stream each project via `SaveProjectToP4()`
  - Migrate `lastconfig.bin` via `SavePicoConfig()`
  - OLED: "Migration complete. Pico SD card can be removed."
  - Write migration-complete flag to P4 config
  - Do NOT delete files from Pico SD

### Verification (Phase 4)

- [ ] Insert old Pico SD with projects → boot → migration runs automatically
- [ ] All projects appear in P4 `/user/projects/` after migration
- [ ] Original files remain on Pico SD (non-destructive)
- [ ] REST API serves `/user/` files over HTTP
- [ ] REST API rejects path traversal and writes to `/factory/`

---

## Key Technical Considerations

1. **SPI chunked transfer** — Current frame is 2048 bytes with 7-byte header = 2041 payload. A 60 KB project needs ~30 frames. Verify `transmitData()` / `receiveData()` handle multi-frame at this scale. Test with 60 KB payload early in Phase 1.

2. **Track default template format** — Current `trackdefaults.json` has `kit` field + per-track `sampleSlice`/`sampleBank`. New template format wraps this in metadata (`name`, `description`, `factory`). The 0xA5 handler must strip metadata or Pico must parse extended format.

3. **Concurrent access** — When WebUI (REST) and Pico (SPI) both access `/user/projects/`, use a mutex around file operations. P4 is single-threaded for SPI but multi-threaded for HTTP.

4. **Atomic writes** — Write to temp file, then `rename()`. FAT32 rename is atomic on ESP-IDF VFS. Critical for project saves during power-cycle.

---

## Relevant Files Reference

### P4 repo (`dadamachines-ctag-tbd`)

| File | Purpose |
|------|---------|
| `main/StorageOverlay.hpp` | Overlay resolution, migration, directory helpers |
| `main/SpiAPI.hpp` / `.cpp` | SPI command handlers, new commands 0xB0–0xC0 |
| `main/SpiProtocol.h` | Low-level SPI frame structures |
| `main/SPManager.cpp` | Boot sequence, `initOverlay()` call |
| `main/MacroAPI.cpp` | Preset/macro REST handlers |
| `main/SampleAPI.cpp` | Sample file I/O (TODO: overlay migration) |
| `main/RestServer.cpp` | HTTP handler registration |
| `sdcard_image/` | Factory SD card image layout |
| `create_sd_archive.sh` | SD card ZIP creation for releases |

### Pico repo (`tbd-pico-seq3`)

| File | Purpose |
|------|---------|
| `src/SpiAPI.h` / `.cpp` | SPI command methods (add 0xB0–0xC0) |
| `src/SpiProtocol.h` | Command ID constants |
| `src/PicoHost.h` / `.cpp` | `saveProject()` / `loadProject()` rewiring |
| `src/PicoStorage.h` / `.cpp` | Make SD-optional, keep for bootloader |
| `src/SdCardHW.cpp` | Conditional SD init |
| `examples/main.cpp` | `fetchAndInitTrackDefaults()`, boot flow, migration |
| `lib/sequencerui/screens/project.cpp` | Menu rename → "Home", add "Track Setup" |
| `lib/sequencerui/screens/project_load.cpp` | SPI-based project list |
| `lib/sequencerui/screens/project_save.cpp` | SPI-based save |
| `lib/sequencerui/screens/trackdefault_select.cpp` | NEW: template browser |
| `lib/sequencerui/screens/trackdefault_edit.cpp` | NEW: template editor |

---

## Scope Boundaries

**Included:** Phases 0–4 above, both repos, on-device template management, no-Pico-SD operation, auto-migration, REST storage API foundation.

**Excluded (future work):**
- WebUI Backup & Restore panel (JSZip browser-side)
- WebUI project browser
- Factory demo projects (content creation)
- Pico SD backup (directory mirror)
- Content-addressed kit store GC via WebUI
- WebUI overlay-aware sample browser (badges, "reset to factory")
