#!/bin/bash
# Write tusb_msc.bin to ota_1 partition on ESP32-P4
# This only needs to be done ONCE per device (tusb_msc.bin persists across firmware updates).
# Usage: ./flash_ota_1.sh [PORT]

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TUSB_BIN="$SCRIPT_DIR/tusb_msc.bin"

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

if [ ! -f "$TUSB_BIN" ]; then
    echo "ERROR: tusb_msc.bin not found at $TUSB_BIN"
    exit 1
fi

echo "Writing tusb_msc.bin to ota_1 partition via $PORT ..."
otatool.py --port "$PORT" write_ota_partition --name ota_1 --input "$TUSB_BIN"
echo "Done. tusb_msc.bin written to ota_1."
echo ""
echo "To enter MSC (SD card USB) mode:"
echo "  otatool.py --port $PORT switch_ota_partition --name ota_1"
echo ""
echo "To return to normal firmware:"
echo "  otatool.py --port $PORT switch_ota_partition --name ota_0"
