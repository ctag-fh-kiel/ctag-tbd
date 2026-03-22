# TBD-16 Flash Scripts (ESP32-P4)

All scripts auto-detect the serial port. Override with: `./script.sh /dev/cu.usbmodemXXXX`

Requires ESP-IDF environment: `. ~/esp/esp-idf/export.sh`

## Scripts

| Script | Purpose |
|--------|----------|
| `flash.sh` | Flash firmware (bootloader + app + partition table + OTA data) |
| `flash-aem.sh` | **Erase entire flash** then write firmware (clean install) |
| `flash_ota_1.sh` | Write `tusb_msc.bin` to ota_1 (only needed once per device) |

## Flash addresses (ESP32-P4)

| Address | Image |
|---------|-------|
| `0x2000` | bootloader.bin |
| `0x8000` | partition-table.bin |
| `0xd000` | ota_data_initial.bin |
| `0x10000` | dada-tbd.bin (ota_0) |

## Pre-built binaries

| File | Purpose |
|------|----------|
| `tusb_msc.bin` | TinyUSB Mass Storage — exposes SD card via USB (lives on ota_1) |
| `usb_uac.bin` | USB Audio Class (experimental) |

## Important

- **NEVER** use `idf.py flash` — it causes OTA reboot loops
- **NEVER** flash `tusb_msc.bin` to `0x10000` — it overwrites main firmware
- Always use `esptool.py` directly with the addresses above
- Sources for tusb_msc: https://github.com/ctag-fh-kiel/tbd-usb-msc
