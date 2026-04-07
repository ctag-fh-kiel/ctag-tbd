#!/bin/bash
set -e

SOURCE_DIR="$1"
BUILD_DIR="$2"
XXH128SUM="$3"

SD_CARD_ZIP="${BUILD_DIR}/dada-tbd-sd.zip"
SD_CARD_HASH="${BUILD_DIR}/dada-tbd-sd-hash.txt"
VERSION_FILE="${BUILD_DIR}/.version"

echo "Creating SD card archive..."
rm -f "${SD_CARD_ZIP}"

# Create temporary directory structure
TEMP_DIR="${BUILD_DIR}/temp_zip_content"
rm -rf "${TEMP_DIR}"
mkdir -p "${TEMP_DIR}"

# Copy factory/user/system overlay directories
echo "Copying factory overlay..."
cp -r "${SOURCE_DIR}/sdcard_image/factory" "${TEMP_DIR}/factory"
echo "Copying user overlay..."
cp -r "${SOURCE_DIR}/sdcard_image/user" "${TEMP_DIR}/user"
echo "Copying system overlay..."
cp -r "${SOURCE_DIR}/sdcard_image/system" "${TEMP_DIR}/system"

# Copy and gzip www files
# Only ship what the device actually needs from Shoelace:
#   shoelace/themes/  — CSS themes (dark + light, toggled at runtime)
# Everything else (components/, chunks/, assets/icons/, autoloader) is unused
# because all used components + icons are already inlined in js/shoelace-bundle.js
echo "Copying and gzipping www files..."
mkdir -p "${TEMP_DIR}/www"
cd "${SOURCE_DIR}/sdcard_image/www"
find . -type f \
    -not -path './node_modules/*' \
    -not -path './tools/*' \
    -not -path './shoelace/components/*' \
    -not -path './shoelace/chunks/*' \
    -not -path './shoelace/assets/*' \
    -not -name 'shoelace.js' \
    -not -name 'shoelace-autoloader.js' \
    -not -name '*.DS_Store' \
    -not -name '*.gz' \
    -not -name 'package.json' \
    -not -name 'package-lock.json' \
    -not -name 'build-webui.sh' \
    -not -name 'readme-api.md' \
    | while read file; do
    # Create directory structure in temp
    mkdir -p "${TEMP_DIR}/www/$(dirname "$file")"

    # Gzip the file with .gz extension
    gzip -c "$file" > "${TEMP_DIR}/www/${file}.gz"
    echo "  Gzipped: $file -> ${file}.gz"
done
cd - > /dev/null

# Copy samples
echo "Copying samples..."
cp -r "${SOURCE_DIR}/sample_rom/samples" "${TEMP_DIR}/samples"

# Create .version placeholder (will be updated with actual hash later)
echo "placeholder" > "${TEMP_DIR}/.version"

# Create zip from temp directory with reproducible settings
echo "Creating zip archive..."
cd "${TEMP_DIR}"
# Use -X to exclude extra file attributes (timestamps, etc.) for reproducible builds
# Set fixed timestamp using TZ=UTC touch
export TZ=UTC
find . -exec touch -t 202001010000.00 {} +
zip -r -X "${SD_CARD_ZIP}" \
    factory \
    user \
    system \
    www \
    samples \
    .version \
    -x '*.DS_Store' '*/__pycache__/*' '*/.gitkeep'

# Clean up temp directory
cd "${BUILD_DIR}"
rm -rf "${TEMP_DIR}"

echo "Generating hash file..."
"${XXH128SUM}" "${SD_CARD_ZIP}" | awk '{print $1}' > "${SD_CARD_HASH}"

echo "Updating .version file in archive with actual hash..."
# Extract, update with real hash, and replace in zip
HASH_VALUE=$(cat "${SD_CARD_HASH}")
echo "${HASH_VALUE}" > "${VERSION_FILE}"
# Update the .version file in the zip (this changes the zip, but .version itself contains the hash of the zip before this update)
zip -u "${SD_CARD_ZIP}" .version
rm -f "${VERSION_FILE}"

echo "SD card archive created: ${SD_CARD_ZIP}"
echo "Hash file created: ${SD_CARD_HASH}"
echo "Contents:"
echo "  - /factory (factory default patches, macros, presets)"
echo "  - /user (user config, overrides, projects)"
echo "  - /system (system metadata)"
echo "  - /www (gzipped web files with .gz extension)"
echo "  - /samples (audio samples)"

