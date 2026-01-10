#!/bin/bash
set -e

SOURCE_DIR="$1"
BUILD_DIR="$2"
XXH128SUM="$3"

SD_CARD_ZIP="${BUILD_DIR}/tbd-sd-card.zip"
SD_CARD_HASH="${BUILD_DIR}/tbd-sd-card-hash.txt"
VERSION_FILE="${BUILD_DIR}/.version"

echo "Creating SD card archive..."
rm -f "${SD_CARD_ZIP}"

# Create temporary directory structure
TEMP_DIR="${BUILD_DIR}/temp_zip_content"
rm -rf "${TEMP_DIR}"
mkdir -p "${TEMP_DIR}"

# Copy folders to temp directory with target names
cp -r "${SOURCE_DIR}/spiffs_image/data" "${TEMP_DIR}/data"
cp -r "${SOURCE_DIR}/spiffs_image/www" "${TEMP_DIR}/www"
cp -r "${SOURCE_DIR}/sample_rom/tbdsamples" "${TEMP_DIR}/tbdsamples"

# Create zip from temp directory
cd "${TEMP_DIR}"
zip -r "${SD_CARD_ZIP}" \
    data \
    www \
    tbdsamples \
    -x '*.DS_Store' '*/__pycache__/*'

# Clean up temp directory
cd "${BUILD_DIR}"
rm -rf "${TEMP_DIR}"

echo "Generating hash file..."
"${XXH128SUM}" "${SD_CARD_ZIP}" | awk '{print $1}' > "${SD_CARD_HASH}"

echo "Adding .version file to archive..."
cp "${SD_CARD_HASH}" "${VERSION_FILE}"
cd "${BUILD_DIR}"
zip -u "${SD_CARD_ZIP}" .version
rm -f "${VERSION_FILE}"

echo "SD card archive created: ${SD_CARD_ZIP}"
echo "Hash file created: ${SD_CARD_HASH}"

