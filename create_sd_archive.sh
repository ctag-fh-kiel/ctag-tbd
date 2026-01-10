#!/bin/bash
set -e

SOURCE_DIR="$1"
BUILD_DIR="$2"
XXH128SUM="$3"

SD_CARD_TAR="${BUILD_DIR}/tbd-sd-card.tar"
SD_CARD_HASH="${BUILD_DIR}/tbd-sd-card-hash.txt"
VERSION_FILE="${BUILD_DIR}/.version"

echo "Creating SD card archive with deflate compression..."
rm -f "${SD_CARD_TAR}"

# Create temporary directory structure
TEMP_DIR="${BUILD_DIR}/temp_archive_content"
rm -rf "${TEMP_DIR}"
mkdir -p "${TEMP_DIR}"

# Copy folders to temp directory with target names
cp -r "${SOURCE_DIR}/spiffs_image/data" "${TEMP_DIR}/data"
cp -r "${SOURCE_DIR}/spiffs_image/www" "${TEMP_DIR}/www"
cp -r "${SOURCE_DIR}/sample_rom/tbdsamples" "${TEMP_DIR}/tbdsamples"

# Create tar archive (uncompressed, just for structure)
cd "${TEMP_DIR}"
tar -cf "${SD_CARD_TAR}" \
    data \
    www \
    tbdsamples

# Clean up temp directory
cd "${BUILD_DIR}"
rm -rf "${TEMP_DIR}"

# Compress the tar with raw deflate (compatible with esp_rom tinfl)
echo "Compressing with raw deflate..."
gzip -c -n "${SD_CARD_TAR}" > "${SD_CARD_TAR}.gz"
# Extract just the deflate stream (remove gzip header/trailer)
python3 -c "
import sys
with open('${SD_CARD_TAR}.gz', 'rb') as f:
    data = f.read()
    # Skip gzip header (10 bytes min) and find deflate start
    # gzip format: 10 byte header, deflate data, 8 byte trailer
    deflate_data = data[10:-8]
    with open('${SD_CARD_TAR}.deflate', 'wb') as out:
        out.write(deflate_data)
"
rm -f "${SD_CARD_TAR}.gz"

echo "Generating hash file..."
"${XXH128SUM}" "${SD_CARD_TAR}.deflate" | awk '{print $1}' > "${SD_CARD_HASH}"

echo "Adding .version file..."
cp "${SD_CARD_HASH}" "${VERSION_FILE}"

echo "SD card archive created: ${SD_CARD_TAR}.deflate"
echo "Original tar: ${SD_CARD_TAR}"
echo "Hash file created: ${SD_CARD_HASH}"

