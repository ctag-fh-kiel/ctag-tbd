#!/bin/bash

# Configuration
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
OUTPUT_DIR="$SCRIPT_DIR/docs/_static/firmware/p4"
OUTPUT_FILENAME="${1:-dada-tbd-unified.bin}"

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Run esptool merge_bin
# Offsets from build/flash_args:
# --flash_mode dio --flash_freq 80m --flash_size 16MB
# 0x2000 bootloader/bootloader.bin
# 0x8000 partition_table/partition-table.bin
# 0xd000 ota_data_initial.bin
# 0x10000 dada-tbd.bin

esptool.py --chip esp32p4 merge_bin \
    -o "$OUTPUT_DIR/$OUTPUT_FILENAME" \
    --flash_mode dio \
    --flash_freq 80m \
    --flash_size 16MB \
    0x2000 "$BUILD_DIR/bootloader/bootloader.bin" \
    0x8000 "$BUILD_DIR/partition_table/partition-table.bin" \
    0xd000 "$BUILD_DIR/ota_data_initial.bin" \
    0x10000 "$BUILD_DIR/dada-tbd.bin"

echo "Unified firmware created at: $OUTPUT_DIR/$OUTPUT_FILENAME"
