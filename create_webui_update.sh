#!/bin/bash
# Create a WebUI update package (.zip) for OTA-style WebUI updates.
#
# Usage:
#   ./create_webui_update.sh <version> <description> [files...]
#
# Example:
#   ./create_webui_update.sh 1.0.1 "Macro/preset fixes" \
#     www/index.html.gz \
#     www/js/app-bundle.js.gz \
#     www/js/macro-bundle.js.gz \
#     www/preset-macro-manager.html.gz \
#     www/webui-update.html.gz \
#     data/macrodefinitions/ro-allnewrompler.json \
#     data/macrosoundpresets/allnewrompler-def.json
#
# File paths are relative to sdcard_image/. The script:
#   1. Validates all listed files exist
#   2. Creates manifest.json with version info
#   3. Packages everything into a .zip (stored, no compression — files are already .gz)
#   4. Output: build/webui-update-v<version>.zip
#
# For www/ files: include the .gz extension (that's what's on the SD card).
# For data/ files: include as plain JSON (no .gz).
#
set -e

if [ $# -lt 3 ]; then
  echo "Usage: $0 <version> <description> <file1> [file2] ..."
  echo ""
  echo "Paths are relative to sdcard_image/ (e.g., www/js/app-bundle.js.gz)"
  exit 1
fi

VERSION="$1"
DESCRIPTION="$2"
shift 2
FILES=("$@")

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SOURCE_DIR="${SCRIPT_DIR}/sdcard_image"
BUILD_DIR="${SCRIPT_DIR}/build"
OUTPUT="${BUILD_DIR}/webui-update-v${VERSION}.zip"

mkdir -p "${BUILD_DIR}"

# Validate all files exist
echo "Validating files..."
for f in "${FILES[@]}"; do
  if [ ! -f "${SOURCE_DIR}/${f}" ]; then
    echo "ERROR: File not found: sdcard_image/${f}"
    exit 1
  fi
  printf "  ✓ %s\n" "$f"
done

# Create temp directory
TEMP_DIR="${BUILD_DIR}/temp_webui_update"
rm -rf "${TEMP_DIR}"
mkdir -p "${TEMP_DIR}"

# Build manifest.json
echo ""
echo "Creating manifest.json..."
{
  echo "{"
  echo "  \"version\": \"${VERSION}\","
  echo "  \"date\": \"$(date +%Y-%m-%d)\","
  echo "  \"description\": \"${DESCRIPTION}\","
  echo "  \"files\": ["
  for i in "${!FILES[@]}"; do
    comma=","
    if [ $i -eq $((${#FILES[@]} - 1)) ]; then comma=""; fi
    echo "    \"${FILES[$i]}\"${comma}"
  done
  echo "  ]"
  echo "}"
} > "${TEMP_DIR}/manifest.json"

cat "${TEMP_DIR}/manifest.json"

# Copy files into temp directory preserving paths
echo ""
echo "Copying files..."
for f in "${FILES[@]}"; do
  mkdir -p "${TEMP_DIR}/$(dirname "$f")"
  cp "${SOURCE_DIR}/${f}" "${TEMP_DIR}/${f}"
  size=$(wc -c < "${SOURCE_DIR}/${f}" | tr -d ' ')
  printf "  %s (%s bytes)\n" "$f" "$size"
done

# Create zip (stored, no compression — www files are already .gz)
echo ""
echo "Creating update package..."
cd "${TEMP_DIR}"
zip -r -0 "${OUTPUT}" manifest.json "${FILES[@]}"

rm -rf "${TEMP_DIR}"

zip_size=$(wc -c < "${OUTPUT}" | tr -d ' ')
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Update package: ${OUTPUT}"
echo "  Version:        ${VERSION}"
echo "  Files:          ${#FILES[@]}"
echo "  Size:           ${zip_size} bytes"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Upload this zip via the WebUI Updater at http://192.168.4.1/webui-update.html"

# ── Publish to CDN repo for online updates ──
CDN_DIR="${SCRIPT_DIR}/../dada-tbd-firmware/webui-updates"
DOCS_DIR="${SCRIPT_DIR}/docs/_static/updates"
ZIP_URL="https://dadamachines.github.io/dada-tbd-firmware/webui-updates/webui-update-v${VERSION}.zip"
if [ -d "${CDN_DIR}" ]; then
  echo ""
  echo "Publishing to CDN repo (dada-tbd-firmware/webui-updates/)..."
  cp "${OUTPUT}" "${CDN_DIR}/"
  echo "  ✓ Copied zip to ${CDN_DIR}/"
  echo ""
  echo "After pushing the CDN repo, the update will be available at:"
  echo "  ${ZIP_URL}"
else
  echo ""
  echo "⚠ CDN repo not found at ${CDN_DIR}"
  echo "  Clone dadamachines/dada-tbd-firmware next to this repo, then copy:"
  echo "  cp ${OUTPUT} <cdn-repo>/webui-updates/"
fi

# ── Update latest.json (docs site — pointer to CDN, fetched by on-device updater) ──
if [ -d "${DOCS_DIR}" ]; then
  echo ""
  echo "Updating docs/_static/updates/latest.json (pointer to CDN)..."
  LATEST="${DOCS_DIR}/latest.json"
  if [ -f "${LATEST}" ] && command -v jq >/dev/null 2>&1; then
    # Preserve history: move current top-level entry into versions array
    OLD_VERSION=$(jq -r '.version // empty' "${LATEST}")
    if [ -n "${OLD_VERSION}" ] && [ "${OLD_VERSION}" != "${VERSION}" ]; then
      jq --arg ver "${VERSION}" \
         --arg date "$(date +%Y-%m-%d)" \
         --arg desc "${DESCRIPTION}" \
         --arg url "${ZIP_URL}" \
         --argjson size "${zip_size}" \
         '{
           version: $ver,
           date: $date,
           description: $desc,
           url: $url,
           size: $size,
           versions: ([{version: .version, date: .date, url: .url, description: .description}] + (.versions // []))
         }' "${LATEST}" > "${LATEST}.tmp" && mv "${LATEST}.tmp" "${LATEST}"
      echo "  ✓ Moved v${OLD_VERSION} into versions history"
    else
      # Same version — just update in place
      jq --arg date "$(date +%Y-%m-%d)" \
         --arg desc "${DESCRIPTION}" \
         --arg url "${ZIP_URL}" \
         --argjson size "${zip_size}" \
         '.date = $date | .description = $desc | .url = $url | .size = $size' \
         "${LATEST}" > "${LATEST}.tmp" && mv "${LATEST}.tmp" "${LATEST}"
    fi
  else
    # No jq or no existing file — write fresh
    cat > "${LATEST}" <<EOF
{
  "version": "${VERSION}",
  "date": "$(date +%Y-%m-%d)",
  "description": "${DESCRIPTION}",
  "url": "${ZIP_URL}",
  "size": ${zip_size}
}
EOF
  fi
  echo "  ✓ Updated latest.json → v${VERSION}"
  # Also update CDN copy of latest.json
  if [ -d "${CDN_DIR}" ]; then
    cp "${DOCS_DIR}/latest.json" "${CDN_DIR}/latest.json"
    echo "  ✓ Updated latest.json in both docs and CDN"
  else
    echo "  ✓ Updated latest.json in docs (CDN copy needs manual update)"
  fi
fi
