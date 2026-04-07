# WebUI Developer Handover — Storage Architecture Refactor (Phase 0–3)

> **Branch:** `feature-test/storage-arch-refactor` (both repos)
> **Date:** June 2026
> **Audience:** WebUI developer who needs to adapt the WebUI to backend storage changes.

---

## Executive Summary

The P4 SD card has been restructured from a flat `/data/` directory into a **factory/user overlay** system. All REST API endpoint paths remain unchanged (`/api/v2/*`), but the **backend now reads from two locations** (user → factory fallback) and **always writes to `/user/`**. `SampleAPI.cpp` has been migrated to overlay. Legacy `/data/` directory is no longer created.

---

## What Changed on Disk

### Old Layout (before refactor)

```
/sdcard/
├── data/
│   ├── spm-config.json         ← device config
│   ├── favs.json               ← favorites
│   ├── synthdefinitions.json   ← synth defs
│   ├── trackdefaults.json      ← single file
│   ├── macrosoundpresets/      ← sound presets
│   ├── macrodefinitions/       ← macro definitions
│   └── sp/                     ← plugin patches (mp-*.json)
├── tbdsamples/                 ← WAV sample files (unchanged)
└── www/                        ← WebUI static assets (unchanged)
```

### New Layout (after refactor)

```
/sdcard/
├── factory/                    ← immutable, ships with firmware
│   ├── presets/                ← was: data/macrosoundpresets/
│   ├── macros/                 ← was: data/macrodefinitions/
│   ├── patches/                ← was: data/sp/
│   ├── trackdefaults/          ← was: data/trackdefaults.json (now per-file)
│   ├── config/                 ← (new)
│   └── synthdefinitions.json   ← was: data/synthdefinitions.json
├── user/                       ← mutable, all writes go here
│   ├── projects/               ← NEW — binary project files (via SPI only)
│   ├── presets/                ← user-modified presets
│   ├── macros/                 ← user-modified macros
│   ├── patches/                ← user-modified plugin patches
│   ├── trackdefaults/          ← user-created templates
│   ├── kits/                   ← NEW — sample kit configs
│   └── config/
│       ├── device.json         ← was: data/spm-config.json
│       ├── favorites.json      ← was: data/favs.json
│       └── sequencer.bin       ← NEW — Pico config (binary, SPI only)
├── system/
│   ├── cache/
│   └── webui-version.json      ← NEW — WebUI version tracking
├── samples/                    ← was: tbdsamples/ (audio WAV files)
└── www/                        ← unchanged
```

### Overlay Resolution Rules

1. **Reads:** Check `/user/{dir}/` first, then `/factory/{dir}/`. First match wins.
2. **Writes:** Always go to `/user/{dir}/`.
3. **Directory listings:** Merge both dirs, user files shadow same-named factory files.
4. **Deletes:** Only from `/user/`. Factory files are immutable (firmware provides them).

---

## REST API Impact

### No Endpoint URLs Changed

All endpoints remain at `/api/v2/*`. The overlay is transparent to the WebUI JS code — the server resolves paths internally via `StorageOverlay.hpp`.

### Endpoints That Work Correctly with Overlay

| Endpoint | Action | Overlay Status |
|----------|--------|----------------|
| `GET /api/v2/macros?action=getall` | List all presets + macros | ✅ Uses `read_all_json_overlay()` — merges factory + user |
| `GET /api/v2/macros?action=get_trackdefaults` | List track default templates | ✅ Uses `listMergedDir(DIR_TRACKDEFAULTS)` |
| `POST /api/v2/macros?action=save_trackdefaults` | Save user template | ✅ Writes to `/user/trackdefaults/` |
| `GET /api/v2/plugins?action=getParams` | Plugin parameters | ✅ Reads via overlay (`resolveFile`) |
| `POST /api/v2/plugins?action=savePreset` | Save preset | ✅ Writes to `/user/patches/` |
| `GET /api/v2/device?action=getConfig` | Device config | ✅ Reads from `/user/config/device.json` |
| `POST /api/v2/device?action=setConfig` | Write config | ✅ Writes to `/user/config/device.json` |
| `POST /api/v2/device?action=storeFavorite` | Save favorite | ✅ Writes to `/user/config/favorites.json` |

### SampleAPI Config Upload/Download — FIXED

`SampleAPI.cpp` now uses overlay for all config operations:

| Action | Behavior |
|--------|----------|
| `?getconfig=presets/foo.json` | ✅ Reads via `resolveFile(subdir, filename)` — user → factory fallback |
| `?action=uploadconfig&path=macros/foo.json` | ✅ Writes to `/user/macros/foo.json` |
| `?action=deleteconfig&path=macros/foo.json` | ✅ Deletes from `/user/` only (factory immutable) |
| Config file listing | ✅ `scan_overlay_configs()` merges factory + user, deduplicates |

**Path format:** `?getconfig=` and `?uploadconfig=` now use overlay subdir paths:
- `presets/filename.json` — sound presets
- `macros/filename.json` — macro definitions
- `patches/filename.json` — plugin patches
- `config/filename.json` — device config
- `trackdefaults/filename.json` — track default templates

**Note:** The `?getconfig=` URL parameter path must include the overlay subdir prefix.

---

## New Backend Capabilities

### New DeviceAPI Actions (already working)

| Action | Purpose |
|--------|---------|
| `getAudioHealth` | Audio engine health stats (underruns, etc.) |
| `resetAudioHealth` | Reset audio health counters |
| `getAppInfo` | Info about Pico app (name, version, flags) |
| `getAll` | Bulk fetch: config + IO caps + favorites + health + app info in one call |

### New SPI-Only Features (no REST endpoint yet)

These features are Pico-UI-only via SPI. No REST/WebUI access currently exists:

| Feature | SPI Code | Notes |
|---------|----------|-------|
| Save/Load/List/Delete projects | 0xB0–0xB3 | Binary project files in `/user/projects/` |
| Save/Load sequencer config | 0xB4–0xB5 | Binary config in `/user/config/sequencer.bin` |
| Manage track default templates | 0xB6–0xB9 | JSON templates in `{factory,user}/trackdefaults/` |
| Set track parameter values | 0xBA | Batch param restore on project load |

If a WebUI project browser is planned, REST endpoints wrapping these would need to be created.

---

## WebUI JS Paths — No Changes Needed

The current JS code (`app-bundle.js`, `macro-bundle.js`, etc.) uses only `/api/v2/*` endpoints — never direct SD card paths. The overlay resolution is server-side, so **existing JS code continues to work** for all endpoints that are correctly overlay-enabled.

The one exception: some JS files contain **comments** referencing the old `/sdcard/data/trackdefaults.json` path. These are just comments, not functional code.

---

## File Naming Convention Change

| Old Name | New Name |
|----------|----------|
| `macrosoundpresets/` | `presets/` |
| `macrodefinitions/` | `macros/` |
| `sp/` (plugin patches) | `patches/` |
| `spm-config.json` | `device.json` |
| `favs.json` | `favorites.json` |

The `?getconfig=` and `?uploadconfig=` actions now expect overlay subdir paths (e.g. `macros/foo.json` instead of `macrodefinitions/foo.json`). WebUI JS code must be updated to use the new subdir names.

---

## Summary of Required WebUI Work

| Priority | Task | Details |
|----------|------|---------|
| 🟡 Medium | Update `?getconfig=` URL params | Must use overlay subdir prefix: `presets/`, `macros/`, `patches/`, `config/` |
| 🟡 Medium | Update `?uploadconfig=` URL params | Same subdir prefix format as `?getconfig=` |
| 🟢 Low | Add project browser UI | REST endpoints for project list/save/load don't exist yet |
| 🟢 Low | Clean up stale comments | Remove old `/sdcard/data/` path references from JS comments |
| ✅ Done | SampleAPI overlay migration | Config upload/download/delete/list all use overlay |
| ✅ Done | Sample dir rename | `/tbdsamples/` → `/samples/` |
| ✅ Done | Legacy `/data/` removal | No longer created in SD archive |
| ✅ Done | Thread safety | SPI shared state protected by FreeRTOS mutex |
