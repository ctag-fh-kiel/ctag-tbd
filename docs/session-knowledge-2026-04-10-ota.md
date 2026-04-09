# Session Knowledge — 2026-04-10 — OTA Firmware Update

## Summary
Implemented real OTA (Over-The-Air) firmware update capability for the TBD-16 P4 firmware, accessible via the WebUI's System Configuration dialog.

## Changes Made

### 1. Partition Table (`partitions_example.csv`)
- Changed `ota_1` from **1MB** to **5MB** — now matches `ota_0` (5MB each)
- Start offset **unchanged** at `0x510000` — web flasher compatibility preserved
- Layout: nvs(16K) + otadata(8K) + phy_init(4K) + ota_0(5MB) + ota_1(5MB)
- ~10.1MB used of 16MB flash, 5.9MB free
- Build confirms: "Smallest app partition is 0x500000 bytes. 35% free"

### 2. OTA REST API Backend (`main/OtaAPI.hpp` + `main/OtaAPI.cpp`)
- **GET `/api/v2/ota`** — Returns JSON with running partition, next update partition name, and max size
- **POST `/api/v2/ota`** — Receives firmware binary, validates magic byte, streams to OTA partition in 4K chunks
- Uses ESP-IDF OTA APIs: `esp_ota_begin()` → `esp_ota_write()` → `esp_ota_end()` → `esp_ota_set_boot_partition()`
- Disables audio plugin processing during flash to avoid contention
- Validates firmware header magic (`ESP_IMAGE_HEADER_MAGIC = 0xE9`) on first chunk
- Logs progress every ~256KB
- Error handling: aborts OTA and re-enables audio on any failure

### 3. Handler Registration (`main/RestServer.cpp`)
- Added `#include "OtaAPI.hpp"` and registered GET/POST handlers for `/api/v2/ota`
- Handler count: 9 → 11 of 20 slots
- OTA handlers registered before the static file wildcard catch-all (which must be last)

### 4. WebUI Integration (`sdcard_image/www/js/app.js`)
- Replaced stub "not yet implemented" toast with full OTA workflow:
  1. Queries GET `/api/v2/ota` for partition info
  2. Shows confirm dialog with running/target partition, max size
  3. Opens file picker for `.bin` firmware files
  4. Client-side size validation before upload
  5. XHR upload with progress bar (updates showLoading text)
  6. On success: offers to POST `{action: "reboot"}` to `/api/v2/device`
  7. 5-minute timeout for large firmware files

### 5. WebUI Bundle Rebuild (`sdcard_image/www/build-webui.sh`)
- Rebuilt `app-bundle.js` + gzipped assets

## Design Decisions

### Why no SPIFFS stage?
The old ctag-tbd OTA had a 4-stage process (initiate → SPIFFS → app → commit) because SPIFFS held the web UI. TBD-16 serves files from SD card, so only the firmware binary needs flashing. Single POST endpoint suffices.

### Why disable audio during flash?
Flash write operations on ESP32-P4 cause bus contention. The old `OTAManager` killed the audio task entirely; our approach is lighter — `DisablePluginProcessing()` pauses the DSP callback while keeping the task alive, then re-enables on completion or error.

### Web flasher compatibility
The web flasher (`tbd-flasher-p4.js`) hardcodes `ota1Addr = 0x510000`. Changing ota_1 **size** doesn't affect the **start offset**, so the flasher continues to work unchanged.

## Architecture Note
The OTA writes to the **inactive** OTA partition. The running firmware is never overwritten. After flashing, `esp_ota_set_boot_partition()` marks the new partition for next boot. The device must reboot to activate it. If the new firmware fails to boot, ESP-IDF's rollback mechanism can revert to the previous partition.

## Files Changed
- `partitions_example.csv` — ota_1 size 1M → 5M
- `main/OtaAPI.hpp` — NEW
- `main/OtaAPI.cpp` — NEW
- `main/RestServer.cpp` — added OTA handler registration
- `sdcard_image/www/js/app.js` — firmware update button wiring
- `sdcard_image/www/js/app-bundle.js` — rebuilt bundle
- `sdcard_image/www/js/app-bundle.js.gz` — rebuilt gzip
