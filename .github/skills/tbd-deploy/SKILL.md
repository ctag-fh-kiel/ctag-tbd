---
name: tbd-deploy
description: 'Deploy TBD-16 firmware and/or WebUI to ESP32-P4 device. USE FOR: full firmware+WebUI deploy; WebUI-only update; fresh SD card erase+reimage; building WebUI bundles; creating SD card archive; flashing firmware via esptool; switching OTA partitions; verifying device after deploy. DO NOT USE FOR: writing or editing WebUI code; debugging firmware C++ code; modifying ESP-IDF configuration. TRIGGER WORDS: deploy, flash, update, SD card, MSC mode, OTA, build WebUI, erase SD, reimage, fresh deploy, push to device.'
argument-hint: 'Specify deploy type: "full", "webui-only", or "fresh-sd" (erase+reimage)'
---

# TBD-16 Deployment

Deploy firmware and/or WebUI to an ESP32-P4 TBD-16 device over USB.

## When to Use

- User says "deploy", "flash", "push to device", "update the device"
- User wants to build WebUI and copy to SD card
- User wants a fresh/clean SD card (erase + reimage)
- User wants to flash new firmware
- After making WebUI code changes that need testing on hardware

## Key Facts

- Serial port: detect with `ls /dev/cu.usb*` (commonly `/dev/cu.usbmodem1201` or `/dev/cu.usbmodem11301`)
- Device IP: `192.168.4.1`, Host IP: `192.168.4.2`
- SD card mount point: `/Volumes/NO NAME` (macOS)
- ESP-IDF setup: `. ~/esp/esp-idf/export.sh`
- xxh128sum: `$(which xxh128sum)` — install with `brew install xxhash` if missing
- Workspace root: the folder containing `CMakeLists.txt`, `sdcard_image/`, `build/`
- Server only serves `.gz` files — always deploy gzipped assets
- `tusb_msc.bin` is already on ota_1 — never flash it with esptool

## NEVER Do These

1. **NEVER** use `idf.py flash` — causes OTA reboot loops
2. **NEVER** flash `tusb_msc.bin` with esptool to `0x10000` — overwrites main firmware
3. **NEVER** leave device in ota_1 (MSC mode) — it won't run main firmware
4. **NEVER** make `.version` and `tbd-sd-card-hash.txt` different — triggers destructive re-extraction at boot
5. **NEVER** forget to eject before switching partitions — corrupts FAT filesystem
6. **NEVER** skip the switch back to ota_0 after copying files
7. **NEVER** `rm -rf` on SD card without `setopt rmstarsilent` first — zsh will prompt and block

## Choose a Deploy Type

| Type | When | Firmware flash? | SD card erase? |
|------|------|-----------------|----------------|
| **Full** | Firmware + WebUI changed | Yes (esptool) | No (copy files) |
| **WebUI-only** | Only JS/HTML/CSS changed | No | No (copy `.gz` files) |
| **Fresh SD** | Corrupted card, clean slate, new card | No | Yes (erase all) |

---

## Procedure: WebUI-Only Update

Use when only WebUI files (JS, HTML, CSS) changed. No firmware flash needed.

### Step 1 — Build WebUI bundles

```bash
cd <workspace_root>
cd sdcard_image/www && bash build-webui.sh && cd ../..
```

Produces `app-bundle.js`, `macro-bundle.js`, gzipped versions of all assets.

### Step 2 — Detect serial port

```bash
ls /dev/cu.usb*
```

Use the port found (e.g. `/dev/cu.usbmodem1201`).

### Step 3 — Switch to MSC mode

```bash
otatool.py --port <PORT> switch_ota_partition --name ota_1
```

Wait ~10s for `/Volumes/NO NAME` to mount. Verify with `ls "/Volumes/NO NAME/"`.

### Step 4 — Copy updated files

```bash
cp sdcard_image/www/js/app-bundle.js.gz "/Volumes/NO NAME/www/js/app-bundle.js.gz"
cp sdcard_image/www/index.html.gz "/Volumes/NO NAME/www/index.html.gz"
```

Copy only the files that changed. Do NOT touch `.version` or `tbd-sd-card-hash.txt`.

### Step 5 — Eject and switch back

```bash
sync && diskutil eject "/Volumes/NO NAME"
sleep 3
otatool.py --port <PORT> switch_ota_partition --name ota_0
```

### Step 6 — Verify

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

### Step 2 — Locate xxh128sum

```bash
which xxh128sum    # if not found: brew install xxhash
```

### Step 3 — Generate SD card archive + hash

```bash
bash create_sd_archive.sh \
  <workspace_root> \
  <workspace_root>/build \
  "$(which xxh128sum)"
```

Produces `build/tbd-sd-card.zip` and `build/tbd-sd-card-hash.txt`.

### Step 4 — Switch to MSC mode

```bash
otatool.py --port <PORT> switch_ota_partition --name ota_1
```

Wait ~10-12s. Verify mount: `ls "/Volumes/NO NAME/"`.

### Step 5 — Erase SD card

```bash
setopt rmstarsilent        # required — zsh blocks rm * otherwise
rm -rf "/Volumes/NO NAME/"*
rm -rf "/Volumes/NO NAME/".* 2>/dev/null || true
sync
```

Verify empty: `ls -la "/Volumes/NO NAME/"` (only `.Spotlight-V100` may remain — that's macOS, harmless).

### Step 6 — Extract archive

```bash
cd <workspace_root>/build
unzip -o tbd-sd-card.zip -d "/Volumes/NO NAME/"
```

### Step 7 — Set hash files (BOTH must match)

```bash
cp tbd-sd-card-hash.txt "/Volumes/NO NAME/tbd-sd-card-hash.txt"
cp tbd-sd-card-hash.txt "/Volumes/NO NAME/.version"
```

### Step 8 — Eject and switch back

```bash
cd <workspace_root>
sync && diskutil eject "/Volumes/NO NAME"
sleep 3
otatool.py --port <PORT> switch_ota_partition --name ota_0
```

### Step 9 — Verify

```bash
sleep 15
ifconfig | grep "inet 192.168.4"    # expect 192.168.4.2
curl -s -o /dev/null -w "%{http_code}" http://192.168.4.1/   # expect 200
```

### Step 10 — Post-deploy validation (optional)

```bash
cd tests/apitest
bash test-device-api.sh 192.168.4.1
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

### Step 3 — Flash firmware (ALL 4 images)

```bash
cd build
esptool.py --chip esp32p4 -p <PORT> -b 460800 \
  --before=default_reset --after=hard_reset \
  write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \
  0x2000 bootloader/bootloader.bin \
  0x10000 ctag-tbd.bin \
  0x8000 partition_table/partition-table.bin \
  0xd000 ota_data_initial.bin
cd ..
```

### Step 4 — Wait for boot, switch to MSC mode

```bash
sleep 10
otatool.py --port <PORT> switch_ota_partition --name ota_1
```

Wait ~10s for `/Volumes/NO NAME`.

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
diskutil eject "/Volumes/NO NAME"
sleep 3
otatool.py --port <PORT> switch_ota_partition --name ota_0
```

### Step 8 — Verify

```bash
sleep 15
ifconfig | grep "inet 192.168.4"
curl -s -o /dev/null -w "%{http_code}" http://192.168.4.1/
```

## Troubleshooting

| Problem | Cause | Fix |
|---------|-------|-----|
| Port not found | Device on different USB port or not connected | `ls /dev/cu.usb*` to find actual port |
| `/Volumes/NO NAME` doesn't mount | MSC boot takes longer | Wait 15-20s, try `ls /Volumes/` |
| `zsh: sure you want to delete` | zsh safety prompt on `rm *` | Run `setopt rmstarsilent` first |
| Hash mismatch at boot → re-extraction | `.version` ≠ `tbd-sd-card-hash.txt` | Copy same file to both locations |
| 404 on WebUI | Missing `.gz` files | Verify all `.gz` assets copied to SD |
| No network after switch | Boot still in progress | Wait 20-30s, re-check `ifconfig` |
| Device stuck in MSC mode | Forgot to switch back to ota_0 | Run `otatool.py ... switch_ota_partition --name ota_0` |
