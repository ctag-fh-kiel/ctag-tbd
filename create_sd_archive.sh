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

cd "${SOURCE_DIR}"
zip -r "${SD_CARD_ZIP}" \
    spiffs_image/data \
    spiffs_image/www \
    sample_rom/tbdsamples \
    -x '*.DS_Store' '*/__pycache__/*'

echo "Generating hash file..."
"${XXH128SUM}" "${SD_CARD_ZIP}" | awk '{print $1}' > "${SD_CARD_HASH}"

echo "Adding .version file to archive..."
cp "${SD_CARD_HASH}" "${VERSION_FILE}"
cd "${BUILD_DIR}"
zip -u "${SD_CARD_ZIP}" .version
rm -f "${VERSION_FILE}"

echo "SD card archive created: ${SD_CARD_ZIP}"
echo "Hash file created: ${SD_CARD_HASH}"

