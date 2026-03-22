---
name: tbd-deploy
description: 'Deploy TBD-16 firmware and/or WebUI to ESP32-P4 device. USE FOR: full firmware+WebUI deploy; firmware-only flash; WebUI-only update; fresh SD card erase+reimage; building WebUI bundles; creating SD card archive; flashing firmware via esptool; switching OTA partitions; verifying device after deploy. DO NOT USE FOR: writing or editing WebUI code; debugging firmware C++ code; modifying ESP-IDF configuration. TRIGGER WORDS: deploy, flash, update, SD card, MSC mode, OTA, build WebUI, erase SD, reimage, fresh deploy, push to device.'
argument-hint: 'Specify deploy type: "full", "firmware-only", "webui-only", or "fresh-sd" (erase+reimage)'
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

### ALWAYS do these:

1. **ALWAYS flash with `esptool.py`** directly — never `idf.py flash`.
2. **ALWAYS use `--chip esp32p4`** (not `esp32`).
3. **ALWAYS flash ALL 4 images** at the correct P4 addresses.
4. **ALWAYS use bootloader address `0x2000`** (not `0x1000` — that's ESP32, not P4).
5. **ALWAYS detect the port** with `ls /dev/cu.usb*` before flashing.
6. **ALWAYS eject SD** (`diskutil eject "/Volumes/NO NAME"`) before switching OTA partitions.

## Key Facts

| Item | Value |
|------|-------|
| Chip | ESP32-P4 |
| Serial port | Detect with `ls /dev/cu.usb*` |
| Device IP | `192.168.4.1` |
| Host IP | `192.168.4.2` |
| SD card mount | `/Volumes/NO NAME` (macOS) |
| ESP-IDF setup | `. ~/esp/esp-idf/export.sh` |
| Hash tool | `xxh128sum` (install: `brew install xxhash`) |
| Server | Only serves `.gz` files — always deploy gzipped assets |
| `tusb_msc.bin` | Already on ota_1 — never re-flash unless doing clean install |
| Flash scripts | `bin/flash.sh`, `bin/flash-aem.sh`, `bin/flash_ota_1.sh` |

## ESP32-P4 Flash Address Map

| Address | Image | Notes |
|---------|-------|-------|
| `0x2000` | `bootloader/bootloader.bin` | P4 bootloader (NOT `0x1000` like ESP32) |
| `0x8000` | `partition_table/partition-table.bin` | Partition table |
| `0xd000` | `ota_data_initial.bin` | Resets OTA state to boot from ota_0 |
| `0x10000` | `dada-tbd.bin` | Main firmware (ota_0 partition) |

## The Correct Flash Command

```bash
# Detect port
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1)

# Flash ALL 4 images from the build directory
esptool.py --chip esp32p4 -p "$PORT" -b 460800 \
  --before=default_reset --after=hard_reset \
  write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \
  0x2000  build/bootloader/bootloader.bin \
  0x10000 build/dada-tbd.bin \
  0x8000  build/partition_table/partition-table.bin \
  0xd000  build/ota_data_initial.bin
```

Or use the script: `bin/flash.sh`

## Choose a Deploy Type

| Type | When | Firmware flash? | SD card erase? |
|------|------|-----------------|----------------|
| **Full** | Firmware + WebUI changed | Yes (esptool) | No (copy files) |
| **Firmware-only** | Only C++ code changed | Yes (esptool) | No |
| **WebUI-only** | Only JS/HTML/CSS changed | No | No (copy `.gz` files) |
| **Fresh SD** | Corrupted card, clean slate, new card | No | Yes (erase all) |

---

## Procedure: Firmware-Only Flash

Use when only C++ firmware code changed. No SD card or WebUI changes.

### Step 1 — Build firmware

```bash
cd <workspace_root>
. ~/esp/esp-idf/export.sh
idf.py build
```

`idf.py build` is fine — it's `idf.py flash` that is forbidden.

### Step 2 — Flash with esptool (ALL 4 images)

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

Or: `bin/flash.sh`

### Step 3 — Verify (wait 15s for boot + USB NCM)

```bash
sleep 15
ifconfig | grep "inet 192.168.4"    # expect 192.168.4.2
curl -s -o /dev/null -w "%{http_code}" http://192.168.4.1/   # expect 200
```

---

## Procedure: Full Deployment (Firmware + WebUI)

Use when both firmware (C++) and WebUI (JS/HTML) have changed.

### Step 1 — Build firmware

```bash
cd <workspace_root>
. ~/esp/esp-idf/export.sh
idf.py build
```

### Step 2 — Build WebUI

```bash
cd sdcard_image/www && bash build-webui.sh && cd ../..
```

### Step 3 — Flash firmware with esptool (ALL 4 images)

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

**DO NOT use `idf.py flash` — use `esptool.py` directly as shown above.**

### Step 4 — Wait for boot, switch to MSC mode

```bash
sleep 10
otatool.py --port "$PORT" switch_ota_partition --name ota_1
```

Wait ~10s for `/Volumes/NO NAME` to mount. Verify: `ls "/Volumes/NO NAME/"`.

### Step 5 — Copy WebUI files

```bash
cp sdcard_image/www/index.html.gz "/Volumes/NO NAME/www/index.html.gz"
cp sdcard_image/www/js/app-bundle.js.gz "/Volumes/NO NAME/www/js/app-bundle.js.gz"
cp sdcard_image/www/js/shoelace-bundle.js.gz "/Volumes/NO NAME/www/js/shoelace-bundle.js.gz"
cp sdcard_image/www/shoelace/themes/dark.css.gz "/Volumes/NO NAME/www/shoelace/themes/dark.css.gz"
```

### Step 6 — Update hash files

```bash
cp build/tbd-sd-card-hash.txt "/Volumes/NO NAME/tbd-sd-card-hash.txt"
cp build/tbd-sd-card-hash.txt "/Volumes/NO NAME/.version"
```

### Step 7 — Eject and switch back

```bash
sync && diskutil eject "/Volumes/NO NAME"
sleep 3
otatool.py --port "$PORT" switch_ota_partition --name ota_0
```

### Step 8 — Verify

```bash
sleep 15
ifconfig | grep "inet 192.168.4"    # expect 192.168.4.2
curl -s -o /dev/null -w "%{http_code}" http://192.168.4.1/   # expect 200
```

---

## Procedure: WebUI-Only Update

Use when only WebUI files (JS, HTML, CSS) changed. No firmware flash needed.

### Step 1 — Build WebUI bundles

```bash
cd <workspace_root>
cd sdcard_image/www && bash build-webui.sh && cd ../..
```

Produces `app-bundle.js`, `macro-bundle.js`, gzipped versions of all assets.

### Step 2 — Detect serial port and switch to MSC mode

```bash
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1)
otatool.py --port "$PORT" switch_ota_partition --name ota_1
```

Wait ~10s for `/Volumes/NO NAME` to mount.

### Step 3 — Copy updated files

```bash
cp sdcard_image/www/js/app-bundle.js.gz "/Volumes/NO NAME/www/js/app-bundle.js.gz"
cp sdcard_image/www/index.html.gz "/Volumes/NO NAME/www/index.html.gz"
```

Copy only the files that changed. Do NOT touch `.version` or `dada-tbd-sd-hash.txt`.

### Step 4 — Eject and switch back

```bash
sync && diskutil eject "/Volumes/NO NAME"
sleep 3
otatool.py --port "$PORT" switch_ota_partition --name ota_0
```

### Step 5 — Verify

```bash
sleep 15
ifconfig | grep "inet 192.168.4"    # expect 192.168.4.2
curl -s -o /dev/null -w "%{http_code}" http://192.168.4.1/   # expect 200
```

---

## Procedure: Fresh SD Card Deploy (Erase + Reimage)

Use when SD card is corrupted, you want a guaranteed clean slate, or setting up a new card.

### Step 1 — Build WebUI bundles

```bash
cd <workspace_root>
cd sdcard_image/www && bash build-webui.sh && cd ../..
```

### Step 2 — Generate SD card archive + hash

```bash
which xxh128sum    # if not found: brew install xxhash
bash create_sd_archive.sh \
  <workspace_root> \
  <workspace_root>/build \
  "$(which xxh128sum)"
```

Produces `build/dada-tbd-sd.zip` and `build/dada-tbd-sd-hash.txt`.

### Step 3 — Switch to MSC mode

```bash
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1)
otatool.py --port "$PORT" switch_ota_partition --name ota_1
```

Wait ~10-12s. Verify: `ls "/Volumes/NO NAME/"`.

### Step 4 — Erase SD card

```bash
setopt rmstarsilent        # required — zsh blocks rm * otherwise
rm -rf "/Volumes/NO NAME/"*
rm -rf "/Volumes/NO NAME/".* 2>/dev/null || true
sync
```

Verify empty: `ls -la "/Volumes/NO NAME/"`.

### Step 5 — Extract archive

```bash
unzip -o build/dada-tbd-sd.zip -d "/Volumes/NO NAME/"
```

### Step 6 — Set hash files (BOTH must match)

```bash
cp build/dada-tbd-sd-hash.txt "/Volumes/NO NAME/dada-tbd-sd-hash.txt"
cp build/dada-tbd-sd-hash.txt "/Volumes/NO NAME/.version"
```

### Step 7 — Eject and switch back

```bash
sync && diskutil eject "/Volumes/NO NAME"
sleep 3
otatool.py --port "$PORT" switch_ota_partition --name ota_0
```

### Step 8 — Verify

```bash
sleep 15
ifconfig | grep "inet 192.168.4"    # expect 192.168.4.2
curl -s -o /dev/null -w "%{http_code}" http://192.168.4.1/   # expect 200
```

---

## Procedure: Clean Install (Erase Flash + Fresh SD)

For a brand new device or to start completely from scratch.

### Step 1 — Erase flash and write firmware

```bash
bin/flash-aem.sh
```

### Step 2 — Write tusb_msc.bin to ota_1

```bash
bin/flash_ota_1.sh
```

### Step 3 — Follow "Fresh SD Card Deploy" procedure above

---

## Troubleshooting

| Problem | Cause | Fix |
|---------|-------|-----|
| OTA reboot loop / device stuck | Used `idf.py flash` | Re-flash with esptool including `ota_data_initial.bin` at `0xd000` |
| "Storage mounted: No" | OTA state corrupted | Flash all 4 images with esptool |
| SpiAPI unknown request errors | Wrong partition or version mismatch | Flash with esptool including `ota_data_initial.bin` |
| Port not found | Different USB port or not connected | `ls /dev/cu.usb*` |
| `/Volumes/NO NAME` doesn't mount | MSC boot takes longer | Wait 15-20s, `ls /Volumes/` |
| `zsh: sure you want to delete` | zsh safety on `rm *` | `setopt rmstarsilent` first |
| Hash mismatch → re-extraction at boot | `.version` ≠ `dada-tbd-sd-hash.txt` | Copy same hash to both |
| 404 on WebUI | Missing `.gz` files | Verify all `.gz` assets on SD |
| No network after switch | Boot still in progress | Wait 20-30s, re-check `ifconfig` |
| Device stuck in MSC mode | Forgot switch back | `otatool.py --port $PORT switch_ota_partition --name ota_0` |
