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

# Copy data folder
echo "Copying data..."
cp -r "${SOURCE_DIR}/spiffs_image/data" "${TEMP_DIR}/data"

# Copy and gzip www files
echo "Copying and gzipping www files..."
mkdir -p "${TEMP_DIR}/www"
cd "${SOURCE_DIR}/spiffs_image/www"
find . -type f | while read file; do
    # Create directory structure in temp
    mkdir -p "${TEMP_DIR}/www/$(dirname "$file")"

    # Gzip the file with .gz extension
    gzip -c "$file" > "${TEMP_DIR}/www/${file}.gz"
    echo "  Gzipped: $file -> ${file}.gz"
done
cd - > /dev/null

# Copy tbdsamples
echo "Copying tbdsamples..."
cp -r "${SOURCE_DIR}/sample_rom/tbdsamples" "${TEMP_DIR}/tbdsamples"

# Create backup of data folder (pre-created backup)
echo "Creating pre-created backup (dbup)..."
cp -r "${TEMP_DIR}/data" "${TEMP_DIR}/dbup"

# Create zip from temp directory
echo "Creating zip archive..."
cd "${TEMP_DIR}"
zip -r "${SD_CARD_ZIP}" \
    data \
    www \
    tbdsamples \
    dbup \
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
echo "Contents:"
echo "  - /data (user data)"
echo "  - /www (gzipped web files with .gz extension)"
echo "  - /tbdsamples (audio samples)"
echo "  - /dbup (pre-created backup of /data)"

