---
name: tbd-deploy
description: 'Deploy TBD-16 firmware and/or WebUI to ESP32-P4 device. USE FOR: full firmware+WebUI deploy; firmware-only flash; WebUI-only update; fresh SD card erase+reimage; clean install; building WebUI bundles; creating SD card archive; flashing firmware via esptool; switching OTA partitions; verifying device after deploy. DO NOT USE FOR: writing or editing WebUI code; debugging firmware C++ code; modifying ESP-IDF configuration. TRIGGER WORDS: deploy, flash, update, SD card, MSC mode, OTA, build WebUI, erase SD, reimage, fresh deploy, push to device, clean install.'
argument-hint: 'Specify deploy type: "full", "firmware-only", "webui-only", "fresh-sd" (erase+reimage), or "clean-install" (erase flash + fresh SD)'
---

# TBD-16 Deployment (ESP32-P4)

Deploy firmware and/or WebUI to an ESP32-P4 TBD-16 device over USB.

## CRITICAL RULES — READ FIRST

### NEVER do these:

1. **NEVER use `idf.py flash`** — it writes OTA data that causes reboot loops. The device will cycle between partitions and fail to boot. This is the #1 cause of "bricked" devices.
2. **NEVER use `idf.py flash monitor`** — same problem. Use esptool + separate monitor.
3. **NEVER use `idf.py -p PORT flash`** — same problem in any form.
4. **NEVER flash `tusb_msc.bin` with esptool to `0x10000`** — overwrites main firmware.
5. **NEVER leave device in ota_1 (MSC mode)** — it won't run main firmware.
6. **NEVER make `.version` and `dada-tbd-sd-hash.txt` different** — triggers destructive re-extraction at boot.
7. **NEVER forget to eject SD before switching partitions** — corrupts FAT filesystem.
8. **NEVER skip the switch back to ota_0** after MSC file operations.
9. **NEVER `rm -rf` on SD card without `setopt rmstarsilent`** — zsh will prompt and block.
10. **NEVER use helper scripts** (`bin/flash.sh`, `bin/flash-aem.sh`, `bin/flash_ota_1.sh`) — always use inline commands from this skill so every step is visible and debuggable.

### ALWAYS do these:

1. **ALWAYS flash with `esptool.py`** directly — never `idf.py flash`.
2. **ALWAYS use `--chip esp32p4`** (not `esp32`).
3. **ALWAYS flash ALL 4 images** at the correct P4 addresses.
4. **ALWAYS use bootloader address `0x2000`** (not `0x1000` — that's ESP32, not P4).
5. **ALWAYS detect the port** with `ls /dev/cu.usbmodem*` before flashing.
6. **ALWAYS eject SD** (`sync && diskutil eject "/Volumes/NO NAME"`) before switching OTA partitions.
7. **ALWAYS chain `source ~/esp/esp-idf/export.sh &&`** before ANY `esptool.py` or `otatool.py` command. The env is NOT preserved across terminal calls. Never assume it persists.
8. **ALWAYS wait 25-30s** after switching to MSC (ota_1) before accessing `/Volumes/NO NAME`. "Permission denied" = still mounting — wait more. Never retry instantly.
9. **ALWAYS re-write `tusb_msc.bin` to ota_1** after `erase_flash` — it wipes the entire flash including ota_1 where `tusb_msc.bin` lives.

## AGENT EXECUTION RULES

These rules prevent the mistakes that occurred during real deploy sessions:

1. **Source ESP-IDF in EVERY command** — each terminal invocation is a fresh shell. Always prefix with `source ~/esp/esp-idf/export.sh 2>/dev/null &&`. Never rely on a previous terminal having sourced it.
2. **Detect port ONCE per procedure** — run `ls /dev/cu.usbmodem*` at the start, then hardcode the result in subsequent commands.
3. **Chain related SD operations** — combine erase + extract + hash into fewer terminal calls to avoid timing issues.
4. **Use generous timeouts** — `diskutil eject` can take 20+ seconds after large writes. Use 30s+ timeouts.
5. **Wait 25s after OTA switch** — the SD mount takes 20-30s in practice, not 10-12s.
6. **Verify SD mount before access** — after waiting, run `ls "/Volumes/NO NAME/"`. If "Permission denied", wait 10s more. If "No such file", MSC is broken.
7. **Never build from stale artifacts** — when deploying a new branch, always rebuild firmware, WebUI, AND regenerate the SD archive+hash to include new plugins/files.

## Key Facts

| Item | Value |
|------|-------|
| Chip | ESP32-P4 |
| Serial port | Detect with `ls /dev/cu.usbmodem*` |
| Device IP | `192.168.4.1` |
| Host IP | `192.168.4.2` |
| SD card mount | `/Volumes/NO NAME` (macOS) |
| ESP-IDF setup | `source ~/esp/esp-idf/export.sh` (every terminal!) |
| Hash tool | `xxh128sum` (install: `brew install xxhash`) |
| Server | Only serves `.gz` files — always deploy gzipped assets |
| `tusb_msc.bin` | Located at `bin/tusb_msc.bin` — lives on ota_1, destroyed by `erase_flash` |
| Hardware config | TBD-16 (default). For alternate configs see `HARDWARE_CONFIGURATIONS.md` |
| ESP-IDF version | 5.5.3 (`espressif/idf:v5.5.3` in CI) |

## ESP32-P4 Flash Address Map

| Address | Image | Notes |
|---------|-------|-------|
| `0x2000` | `bootloader/bootloader.bin` | P4 bootloader (NOT `0x1000` like ESP32) |
| `0x8000` | `partition_table/partition-table.bin` | Partition table |
| `0xd000` | `ota_data_initial.bin` | Resets OTA state to boot from ota_0 |
| `0x10000` | `dada-tbd.bin` | Main firmware (ota_0 partition) |

## Choose a Deploy Type

| Type | When | Firmware flash? | SD card erase? | Erase flash? |
|------|------|-----------------|----------------|--------------|
| **Full** | Firmware + WebUI changed | Yes (esptool) | No (copy files) | No |
| **Firmware-only** | Only C++ code changed | Yes (esptool) | No | No |
| **WebUI-only** | Only JS/HTML/CSS changed | No | No (copy `.gz` files) | No |
| **Fresh SD** | Corrupted card, clean slate | No | Yes (erase all) | No |
| **Clean Install** | New device, unrecoverable state | Yes (erase all) | Yes (erase all) | Yes |

---

## Procedure: Firmware-Only Flash

Use when only C++ firmware code changed. No SD card or WebUI changes.

### Step 1 — Build firmware

```bash
source ~/esp/esp-idf/export.sh 2>/dev/null && idf.py build
```

`idf.py build` is fine — it's `idf.py flash` that is forbidden.

### Step 2 — Flash with esptool (ALL 4 images)

```bash
source ~/esp/esp-idf/export.sh 2>/dev/null && \
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1) && \
esptool.py --chip esp32p4 -p "$PORT" -b 460800 \
  --before=default_reset --after=hard_reset \
  write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \
  0x2000  build/bootloader/bootloader.bin \
  0x10000 build/dada-tbd.bin \
  0x8000  build/partition_table/partition-table.bin \
  0xd000  build/ota_data_initial.bin
```

### Step 3 — Verify (wait 20s for boot + USB NCM)

```bash
sleep 20 && ifconfig | grep "inet 192.168.4" && \
curl -s -o /dev/null -w "HTTP %{http_code}\n" http://192.168.4.1/
```

Expect: `192.168.4.2` and `HTTP 200`.

---

## Procedure: Full Deployment (Firmware + WebUI)

Use when both firmware (C++) and WebUI (JS/HTML) have changed.

### Step 1 — Build firmware

```bash
source ~/esp/esp-idf/export.sh 2>/dev/null && idf.py build
```

### Step 2 — Build WebUI

```bash
cd sdcard_image/www && bash build-webui.sh && cd ../..
```

### Step 3 — Flash firmware with esptool (ALL 4 images)

```bash
source ~/esp/esp-idf/export.sh 2>/dev/null && \
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1) && \
esptool.py --chip esp32p4 -p "$PORT" -b 460800 \
  --before=default_reset --after=hard_reset \
  write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \
  0x2000  build/bootloader/bootloader.bin \
  0x10000 build/dada-tbd.bin \
  0x8000  build/partition_table/partition-table.bin \
  0xd000  build/ota_data_initial.bin
```

### Step 4 — Wait for boot, switch to MSC mode

```bash
sleep 10 && source ~/esp/esp-idf/export.sh 2>/dev/null && \
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1) && \
otatool.py --port "$PORT" switch_ota_partition --name ota_1
```

Wait **25-30s** for `/Volumes/NO NAME` to mount. Verify:

```bash
sleep 25 && ls "/Volumes/NO NAME/"
```

If "Permission denied" → wait 10s more. If "No such file" → MSC is broken (see Troubleshooting).

### Step 5 — Copy WebUI files + update hash files

```bash
cp sdcard_image/www/index.html.gz "/Volumes/NO NAME/www/index.html.gz" && \
cp sdcard_image/www/js/app-bundle.js.gz "/Volumes/NO NAME/www/js/app-bundle.js.gz" && \
cp sdcard_image/www/js/shoelace-bundle.js.gz "/Volumes/NO NAME/www/js/shoelace-bundle.js.gz" && \
cp sdcard_image/www/shoelace/themes/dark.css.gz "/Volumes/NO NAME/www/shoelace/themes/dark.css.gz" && \
cp build/dada-tbd-sd-hash.txt "/Volumes/NO NAME/dada-tbd-sd-hash.txt" && \
cp build/dada-tbd-sd-hash.txt "/Volumes/NO NAME/.version"
```

### Step 6 — Eject and switch back

```bash
sync && diskutil eject "/Volumes/NO NAME"
```

Wait for eject to complete (can take 15-20s). Then:

```bash
sleep 5 && source ~/esp/esp-idf/export.sh 2>/dev/null && \
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1) && \
otatool.py --port "$PORT" switch_ota_partition --name ota_0
```

### Step 7 — Verify

```bash
sleep 20 && ifconfig | grep "inet 192.168.4" && \
curl -s -o /dev/null -w "HTTP %{http_code}\n" http://192.168.4.1/
```

---

## Procedure: WebUI-Only Update

Use when only WebUI files (JS, HTML, CSS) changed. No firmware flash needed.

### Step 1 — Build WebUI bundles

```bash
cd sdcard_image/www && bash build-webui.sh && cd ../..
```

Produces `app-bundle.js`, `macro-bundle.js`, gzipped versions of all assets.

### Step 2 — Switch to MSC mode

```bash
source ~/esp/esp-idf/export.sh 2>/dev/null && \
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1) && \
otatool.py --port "$PORT" switch_ota_partition --name ota_1
```

Wait **25-30s** for `/Volumes/NO NAME` to mount. Verify:

```bash
sleep 25 && ls "/Volumes/NO NAME/"
```

### Step 3 — Copy updated files

```bash
cp sdcard_image/www/js/app-bundle.js.gz "/Volumes/NO NAME/www/js/app-bundle.js.gz" && \
cp sdcard_image/www/index.html.gz "/Volumes/NO NAME/www/index.html.gz"
```

Copy only the files that changed. Do NOT touch `.version` or `dada-tbd-sd-hash.txt`.

### Step 4 — Eject and switch back

```bash
sync && diskutil eject "/Volumes/NO NAME"
```

Wait for eject to complete. Then:

```bash
sleep 5 && source ~/esp/esp-idf/export.sh 2>/dev/null && \
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1) && \
otatool.py --port "$PORT" switch_ota_partition --name ota_0
```

### Step 5 — Verify

```bash
sleep 20 && ifconfig | grep "inet 192.168.4" && \
curl -s -o /dev/null -w "HTTP %{http_code}\n" http://192.168.4.1/
```

---

## Procedure: Fresh SD Card Deploy (Erase + Reimage)

Use when SD card is corrupted, you want a guaranteed clean slate, or setting up a new card.

### Step 1 — Build WebUI bundles

```bash
cd sdcard_image/www && bash build-webui.sh && cd ../..
```

### Step 2 — Generate SD card archive + hash

```bash
bash create_sd_archive.sh \
  <workspace_root> \
  <workspace_root>/build \
  "$(which xxh128sum)"
```

If `xxh128sum` not found: `brew install xxhash`.
Produces `build/dada-tbd-sd.zip` and `build/dada-tbd-sd-hash.txt`.

### Step 3 — Switch to MSC mode

```bash
source ~/esp/esp-idf/export.sh 2>/dev/null && \
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1) && \
otatool.py --port "$PORT" switch_ota_partition --name ota_1
```

Wait **25-30s**. Verify:

```bash
sleep 25 && ls "/Volumes/NO NAME/"
```

### Step 4 — Erase SD card, extract archive, set hash files

Combine into one sequence to avoid timing issues:

```bash
setopt rmstarsilent && \
rm -rf "/Volumes/NO NAME/"* && \
rm -rf "/Volumes/NO NAME/".* 2>/dev/null; true && \
sync && \
unzip -o build/dada-tbd-sd.zip -d "/Volumes/NO NAME/" && \
cp build/dada-tbd-sd-hash.txt "/Volumes/NO NAME/dada-tbd-sd-hash.txt" && \
cp build/dada-tbd-sd-hash.txt "/Volumes/NO NAME/.version" && \
echo "Done. Hash:" && cat "/Volumes/NO NAME/.version"
```

### Step 5 — Eject and switch back

```bash
sync && diskutil eject "/Volumes/NO NAME"
```

Wait for eject to complete (can take 20s+ after large writes). Then:

```bash
sleep 5 && source ~/esp/esp-idf/export.sh 2>/dev/null && \
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1) && \
otatool.py --port "$PORT" switch_ota_partition --name ota_0
```

### Step 6 — Verify

```bash
sleep 20 && ifconfig | grep "inet 192.168.4" && \
curl -s -o /dev/null -w "HTTP %{http_code}\n" http://192.168.4.1/
```

---

## Procedure: Clean Install (Erase Flash + Fresh SD)

For a brand new device or to start completely from scratch.

**WARNING**: `erase_flash` destroys EVERYTHING on the flash chip — including `tusb_msc.bin` on ota_1. You MUST re-write `tusb_msc.bin` before MSC mode will work again.

### Step 1 — Erase entire flash

```bash
source ~/esp/esp-idf/export.sh 2>/dev/null && \
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1) && \
esptool.py --chip esp32p4 -p "$PORT" -b 460800 erase_flash
```

### Step 2 — Write firmware (ALL 4 images)

```bash
source ~/esp/esp-idf/export.sh 2>/dev/null && \
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1) && \
esptool.py --chip esp32p4 -p "$PORT" -b 460800 \
  --before=default_reset --after=hard_reset \
  write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \
  0x2000  build/bootloader/bootloader.bin \
  0x10000 build/dada-tbd.bin \
  0x8000  build/partition_table/partition-table.bin \
  0xd000  build/ota_data_initial.bin
```

### Step 3 — Re-write tusb_msc.bin to ota_1

Wait 5s for device to reset, then:

```bash
sleep 5 && source ~/esp/esp-idf/export.sh 2>/dev/null && \
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1) && \
otatool.py --port "$PORT" write_ota_partition --name ota_1 --input bin/tusb_msc.bin
```

### Step 4 — Follow "Fresh SD Card Deploy" from Step 3 onward

Switch to MSC, erase SD, extract archive, set hashes, eject, switch back, verify.

---

## Troubleshooting

| Problem | Cause | Fix |
|---------|-------|-----|
| `command not found: esptool.py` | ESP-IDF env not sourced | Always prefix with `source ~/esp/esp-idf/export.sh 2>/dev/null &&` |
| `command not found: otatool.py` | ESP-IDF env not sourced | Same fix — source ESP-IDF before every command |
| OTA reboot loop / device stuck | Used `idf.py flash` | Re-flash with esptool including `ota_data_initial.bin` at `0xd000` |
| "Storage mounted: No" | OTA state corrupted | Flash all 4 images with esptool |
| SpiAPI unknown request errors | Wrong partition or version mismatch | Flash with esptool including `ota_data_initial.bin` |
| Port not found | Different USB port or not connected | `ls /dev/cu.usbmodem*` — try unplugging and re-plugging USB |
| `/Volumes/NO NAME` not mounted | MSC boot takes 25-30s | Wait 25-30s, then `ls /Volumes/`. Do NOT retry before waiting |
| "Permission denied" on SD | Still mounting | Wait 10-15s more — filesystem isn't ready yet |
| MSC mode not working after erase_flash | `erase_flash` destroyed `tusb_msc.bin` | Re-write: `otatool.py --port $PORT write_ota_partition --name ota_1 --input bin/tusb_msc.bin` |
| `zsh: sure you want to delete` | zsh safety on `rm *` | `setopt rmstarsilent` first |
| Hash mismatch → re-extraction at boot | `.version` ≠ `dada-tbd-sd-hash.txt` | Copy same hash to both |
| 404 on WebUI | Missing `.gz` files | Verify all `.gz` assets on SD |
| No network after switch | Boot still in progress | Wait 25-30s, re-check `ifconfig` |
| Device stuck in MSC mode | Forgot switch back | `source ~/esp/esp-idf/export.sh 2>/dev/null && otatool.py --port $PORT switch_ota_partition --name ota_0` |
| `diskutil eject` hangs | Large sync flush | Wait 20-30s — don't kill it. Large writes take time to flush |
| Plugin missing from WebUI | Stale `spm-config.json` cache or stale SD archive | Rebuild archive with `create_sd_archive.sh`, re-deploy fresh SD. Ensure no `availableProcessors` key in `spm-config.json` |

---

## Development vs Production Builds

The P4 uses ESP-IDF's log level system to control debug output at compile time.

### Log Level Architecture

| sdkconfig Setting | Effect |
|-------------------|--------|
| `CONFIG_LOG_DEFAULT_LEVEL=3` (INFO) | Runtime default — what prints at boot |
| `CONFIG_LOG_MAXIMUM_LEVEL=3` (INFO) | **Compile-time ceiling** — `ESP_LOGD()` calls are removed by preprocessor |
| `CONFIG_LOG_MAXIMUM_LEVEL=4` (DEBUG) | `ESP_LOGD()` calls are compiled in and available at runtime |

The key insight: when `CONFIG_LOG_MAXIMUM_LEVEL=3`, ALL `ESP_LOGD()` calls have **zero overhead** — they don't exist in the binary. This is the production default.

### Development Build (verbose logging)

To enable debug output for development, change `CONFIG_LOG_MAXIMUM_LEVEL` to 4 (DEBUG):

```bash
# Using idf.py menuconfig (interactive):
source ~/esp/esp-idf/export.sh 2>/dev/null && idf.py menuconfig
# Navigate: Component config → Log → Maximum log verbosity → Debug

# Or edit sdkconfig directly:
sed -i '' \
  -e 's/^CONFIG_LOG_MAXIMUM_LEVEL=3/CONFIG_LOG_MAXIMUM_LEVEL=4/' \
  -e 's/^# CONFIG_LOG_MAXIMUM_LEVEL_DEBUG is not set/CONFIG_LOG_MAXIMUM_LEVEL_DEBUG=y/' \
  sdkconfig
```

Then build and flash normally:

```bash
source ~/esp/esp-idf/export.sh 2>/dev/null && idf.py build
```

Flash with esptool as per the Firmware-Only Flash procedure above.

### Production Build (quiet, default)

Ensure `CONFIG_LOG_MAXIMUM_LEVEL=3` (the default). This compiles out all `ESP_LOGD()` calls:

```bash
# Verify current setting:
grep CONFIG_LOG_MAXIMUM_LEVEL= sdkconfig
# Should show: CONFIG_LOG_MAXIMUM_LEVEL=3

# If it was changed for dev, restore production settings:
sed -i '' \
  -e 's/^CONFIG_LOG_MAXIMUM_LEVEL=4/CONFIG_LOG_MAXIMUM_LEVEL=3/' \
  -e 's/^CONFIG_LOG_MAXIMUM_LEVEL_DEBUG=y/# CONFIG_LOG_MAXIMUM_LEVEL_DEBUG is not set/' \
  sdkconfig
```

Then rebuild and flash.

### Monitoring P4 Serial Output

The P4 outputs logs via USB Serial JTAG (secondary console) at 115200 baud:

```bash
# Using idf.py monitor (safe — it's idf.py FLASH that is forbidden, not monitor):
source ~/esp/esp-idf/export.sh 2>/dev/null && \
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1) && \
idf.py -p "$PORT" monitor

# Or using screen:
screen /dev/cu.usbmodem* 115200

# Exit screen: Ctrl-A, then K, then Y
# Exit idf.py monitor: Ctrl-]
```

### What Each Log Level Shows (after cleanup)

| Level | What prints |
|-------|-------------|
| **Production** (MAX_LEVEL=3) | Boot banner, state changes (plugin switch, project save/load, reboot), errors, boot summaries |
| **Development** (MAX_LEVEL=4) | All of production + per-request heap dumps, per-item boot details, SPI command traces, debug_task heap monitoring (every 4s), SROM diagnostics |

### IMPORTANT: Never commit sdkconfig changes for dev builds

The `sdkconfig` with `CONFIG_LOG_MAXIMUM_LEVEL=3` is the production default. If you change it for development, do NOT commit the change. Restore before committing:

```bash
git checkout -- sdkconfig
```
