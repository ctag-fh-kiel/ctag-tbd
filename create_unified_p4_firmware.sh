#!/bin/bash

# Configuration
ESPTOOL_PATH="/Users/jlo/esp/esptool-macos/esptool"
SOURCE_DIR="/Users/jlo/esp/esptool-macos/P4/per"
OUTPUT_DIR="/Users/jlo/Documents/GitHub/dadamachines-ctag-tbd/docs/flash/_static/firmware/p4"
OUTPUT_FILENAME="ctag-tbd-unified.bin"

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Run esptool merge_bin
# Note: Offsets and parameters derived from user's flash command:
# --flash_mode dio --flash_freq 80m --flash_size 16MB 
# 0x2000 bootloader.bin
# 0x8000 partition-table.bin
# 0xd000 ota_data_initial.bin
# 0x10000 ctag-tbd.bin

"$ESPTOOL_PATH" --chip esp32p4 merge_bin \
    -o "$OUTPUT_DIR/$OUTPUT_FILENAME" \
    --flash_mode dio \
    --flash_freq 80m \
    --flash_size 16MB \
    0x2000 "$SOURCE_DIR/bootloader.bin" \
    0x8000 "$SOURCE_DIR/partition-table.bin" \
    0xd000 "$SOURCE_DIR/ota_data_initial.bin" \
    0x10000 "$SOURCE_DIR/ctag-tbd.bin"

echo "Unified firmware created at: $OUTPUT_DIR/$OUTPUT_FILENAME"
