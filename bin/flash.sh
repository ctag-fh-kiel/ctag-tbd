#!/bin/bash
# Flash TBD-16 firmware to ESP32-P4
# Usage: ./flash.sh [PORT]
#   PORT defaults to auto-detect via ls /dev/cu.usbmodem*

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
        echo "Usage: $0 [PORT]"
        exit 1
    fi
    echo "Auto-detected port: $PORT"
fi

# Verify build artifacts exist
for f in bootloader/bootloader.bin dada-tbd.bin partition_table/partition-table.bin ota_data_initial.bin; do
    if [ ! -f "$BUILD_DIR/$f" ]; then
        echo "ERROR: Missing $BUILD_DIR/$f — run 'idf.py build' first."
        exit 1
    fi
done

echo "Flashing TBD-16 firmware via $PORT ..."
esptool.py --chip esp32p4 -p "$PORT" -b 460800 \
    --before=default_reset --after=hard_reset \
    write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \
    0x2000  "$BUILD_DIR/bootloader/bootloader.bin" \
    0x10000 "$BUILD_DIR/dada-tbd.bin" \
    0x8000  "$BUILD_DIR/partition_table/partition-table.bin" \
    0xd000  "$BUILD_DIR/ota_data_initial.bin"

echo "Done. Device will boot from ota_0."
