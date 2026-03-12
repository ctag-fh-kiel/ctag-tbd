#!/bin/bash
# Build script for WebUI — creates JS bundles + .gz versions for ESP32 deployment.
# The ESP32 RestServer serves .gz files exclusively (Content-Encoding: gzip).
#
# Usage:  cd sdcard_image/www && ./build-webui.sh
#
# This script:
#   1. Concatenates JS source files into two bundles:
#      - js/app-bundle.js   (index.html — Plugin & Sample Manager)
#      - js/macro-bundle.js (preset-macro-manager.html — Preset & Macro Manager)
#   2. Gzips all assets for production deployment
#
# Note: The full build pipeline (create_sd_archive.sh) also gzips these files.
#       This script is for quick local testing of gzipped assets.

set -e
cd "$(dirname "$0")"

# ── Step 1a: Create app-bundle.js (index.html) ──
# Order matters — dependency chain: Sortable (vendor) → webaudio-controls →
# shared → display-hints → plugin-manager → sample-manager → app (shell, boots last)
BUNDLE_SOURCES=(
  js/Sortable.min.js
  js/webaudio-controls.js
  js/shared.js
  js/display-hints.js
  js/plugin-manager.js
  js/sample-manager.js
  js/app.js
)
BUNDLE_OUT="js/app-bundle.js"

echo "Building app-bundle.js (index.html)..."
> "$BUNDLE_OUT"
for src in "${BUNDLE_SOURCES[@]}"; do
  if [ -f "$src" ]; then
    echo "// ── $(basename "$src") ───" >> "$BUNDLE_OUT"
    cat "$src" >> "$BUNDLE_OUT"
    echo "" >> "$BUNDLE_OUT"
    printf "  + %-30s\n" "$src"
  else
    echo "  WARN: $src not found, skipping"
  fi
done

bundle_size=$(wc -c < "$BUNDLE_OUT" | tr -d ' ')
echo "  → $BUNDLE_OUT ($bundle_size bytes)"
echo ""

# ── Step 1b: Create macro-bundle.js (preset-macro-manager.html) ──
# Order: Sortable (vendor) → shared → display-hints → performer → designer
# → preset-macro-app (shell, boots last)
MACRO_BUNDLE_SOURCES=(
  js/Sortable.min.js
  js/shared.js
  js/factory-manifest.js
  js/display-hints.js
  js/performer.js
  js/designer.js
  js/track-defaults.js
  js/preset-macro-app.js
)
MACRO_BUNDLE_OUT="js/macro-bundle.js"

echo "Building macro-bundle.js (preset-macro-manager.html)..."
> "$MACRO_BUNDLE_OUT"
for src in "${MACRO_BUNDLE_SOURCES[@]}"; do
  if [ -f "$src" ]; then
    echo "// ── $(basename "$src") ───" >> "$MACRO_BUNDLE_OUT"
    cat "$src" >> "$MACRO_BUNDLE_OUT"
    echo "" >> "$MACRO_BUNDLE_OUT"
    printf "  + %-30s\n" "$src"
  else
    echo "  WARN: $src not found, skipping"
  fi
done

macro_bundle_size=$(wc -c < "$MACRO_BUNDLE_OUT" | tr -d ' ')
echo "  → $MACRO_BUNDLE_OUT ($macro_bundle_size bytes)"
echo ""

# ── Step 2: Gzip all production assets ──
# Only the files that the ESP32 actually serves need .gz versions.
# Individual JS source files are NOT gzipped — only the bundle is served.
GZIP_FILES=(
  index.html
  preset-macro-manager.html
  webui-update.html
  js/app-bundle.js
  js/macro-bundle.js
  js/shoelace-bundle.js
  shoelace/themes/dark.css
  shoelace/themes/light.css
  css/app.css
)

echo "Creating gzip assets..."
for f in "${GZIP_FILES[@]}"; do
  if [ -f "$f" ]; then
    gzip -k -f -9 "$f"
    orig=$(wc -c < "$f" | tr -d ' ')
    comp=$(wc -c < "$f.gz" | tr -d ' ')
    ratio=$((100 - comp * 100 / orig))
    printf "  %-40s %6s → %6s  (%d%% smaller)\n" "$f" "$orig" "$comp" "$ratio"
  else
    echo "  SKIP: $f (not found)"
  fi
done

echo ""
echo "Done. Bundles + gzipped assets ready for ESP32 deployment."
