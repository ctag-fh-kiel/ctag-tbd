#!/bin/bash
# Erase flash and write fresh firmware to ESP32-P4 (clean install)
# WARNING: This erases the ENTIRE flash. You will need to re-image the SD card after.
# Usage: ./flash-aem.sh [PORT]

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/../build"

# Auto-detect or use provided port
if [ -n "$1" ]; then
    PORT="$1"
else
    PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1)
    if [ -z "$PORT" ]; then
        echo "ERROR: No USB device found. Connect the ESP32-P4 and try again."
        exit 1
    fi
    echo "Auto-detected port: $PORT"
fi

# Verify build artifacts exist
for f in bootloader/bootloader.bin ctag-tbd.bin partition_table/partition-table.bin ota_data_initial.bin; do
    if [ ! -f "$BUILD_DIR/$f" ]; then
        echo "ERROR: Missing $BUILD_DIR/$f — run 'idf.py build' first."
        exit 1
    fi
done

echo "=== Step 1/2: Erasing entire flash ==="
esptool.py --chip esp32p4 -p "$PORT" -b 460800 erase_flash

echo "=== Step 2/2: Writing firmware ==="
esptool.py --chip esp32p4 -p "$PORT" -b 460800 \
    --before=default_reset --after=hard_reset \
    write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \
    0x2000  "$BUILD_DIR/bootloader/bootloader.bin" \
    0x10000 "$BUILD_DIR/ctag-tbd.bin" \
    0x8000  "$BUILD_DIR/partition_table/partition-table.bin" \
    0xd000  "$BUILD_DIR/ota_data_initial.bin"

echo ""
echo "Done. Flash erased and firmware written."
echo "NEXT STEPS:"
echo "  1. Write tusb_msc.bin to ota_1:  ./flash_ota_1.sh $PORT"
echo "  2. Re-image SD card (fresh-sd deploy) — see SKILL.md or create_sd_archive.sh"
