# Setting Up Auto-Publish to the dadamachines CDN

This guide walks you through adding automatic CDN publishing to your
Pico app repo (`tbd-pico-seq3`). Once set up, tagging a release pushes
your `.uf2` to the dadamachines firmware CDN automatically — no manual
file copying.

---

## What's already done

- **`PICO_CDN_TOKEN`** is installed as a secret in your repo. This is a
  fine-grained GitHub PAT scoped to `dadamachines/dada-tbd-firmware`
  with Contents: write permission. You don't need to create or manage it.

- **CDN `receive-pico-app.yml`** is live in the CDN repo. It receives
  dispatch events, verifies SHA-256, and commits the `.uf2` to the CDN.

---

## Build environment: `possan_rev_c`

The shipping TBD-16 hardware is **Rev C**. Your repo has four PlatformIO
environments in `platformio.ini`:

| Environment | Purpose | Use for CI / shipping? |
|-------------|---------|----------------------|
| `pi2350` | Generic RP2350 (no hardware revision flag) | **No** — wrong SPI pins, no SD reset, wrong LED layout |
| `nevvkid` | Johannes's dev environment (CMSIS-DAP debug) | No — dev only |
| `possan_rev_b` | Rev B hardware (your old board) | No — superseded by Rev C |
| **`possan_rev_c`** | **Rev C shipping hardware** | **Yes — use this for CI and releases** |

Key differences in `possan_rev_c` vs `pi2350`:

- **`-DREV_C`** — enables correct SPI pin config (7-arg `DaDa_SPI`
  constructor with extra ready pin), SD card reset (GPIO 17), SPI flow
  control (`WaitUntilP4IsReady()` instead of blind delay), and Rev C
  LED layout (20 LEDs, no MIDDLE LED)
- **Source filter** — excludes `Ui.cpp`, `Midi.cpp`, `MidiP4.cpp` (the
  generic implementations) because your Groovebox has its own
  replacements (`MidiP4-2.cpp`, sequencer UI)
- **USB serial disabled** — `DISABLE_USB_SERIAL`,
  `PIO_FRAMEWORK_ARDUINO_ENABLE_CDC=0`, `CFG_TUSB_RHPORT0_MODE=2`
- **PSRAM** — 8 MB, CS pin 19 (same as `pi2350`)
- **DaDa_SPI v1.0.5** (same as `pi2350`)

Building `pi2350` for shipping hardware produces a binary with wrong SPI
pins, missing SD card reset, wrong LED count, and no SPI flow control.

---

## What you need to do

### 1. Update your CI to build `possan_rev_c`

Your current `.github/workflows/build_firmware.yml` builds the `pi2350`
environment. Change it to build `possan_rev_c`:

```diff
      - name: Build Firmware
-       run: pio run -e pi2350
+       run: pio run -e possan_rev_c

      - name: Upload Firmware
        uses: actions/upload-artifact@v4
        with:
          name: tbd_frontend
          path: |
-           .pio/build/pi2350/firmware.bin
-           .pio/build/pi2350/firmware.uf2
+           .pio/build/possan_rev_c/firmware.bin
+           .pio/build/possan_rev_c/firmware.uf2
          overwrite: true
```

### 2. Add the `publish-cdn` job

Add this job **after** your existing `build` job (same indentation level
as `build:`):

```yaml
  # ────────────────────────────────────────────────────────────
  # Publish to dadamachines firmware CDN on tagged releases.
  # Requires PICO_CDN_TOKEN secret (already installed).
  # Normal pushes (no tag) skip this job entirely.
  # ────────────────────────────────────────────────────────────
  publish-cdn:
    needs: build
    if: startsWith(github.ref, 'refs/tags/v')
    runs-on: ubuntu-latest
    steps:
      - name: Check for CDN token
        id: check
        run: |
          if [ -z "${{ secrets.PICO_CDN_TOKEN }}" ]; then
            echo "skip=true" >> "$GITHUB_OUTPUT"
            echo "::notice::PICO_CDN_TOKEN not set — skipping CDN publish"
          else
            echo "skip=false" >> "$GITHUB_OUTPUT"
          fi

      - name: Download build artifact
        if: steps.check.outputs.skip != 'true'
        uses: actions/download-artifact@v4
        with:
          name: tbd_frontend
          path: pico-artifact

      - name: Compute SHA-256
        if: steps.check.outputs.skip != 'true'
        id: sha
        run: |
          SHA=$(sha256sum pico-artifact/firmware.uf2 | cut -d' ' -f1)
          echo "sha256=${SHA}" >> "$GITHUB_OUTPUT"
          echo "SHA-256: $SHA"

      - name: Dispatch to CDN
        if: steps.check.outputs.skip != 'true'
        uses: peter-evans/repository-dispatch@v3
        with:
          token: ${{ secrets.PICO_CDN_TOKEN }}
          repository: dadamachines/dada-tbd-firmware
          event-type: pico-app-update
          client-payload: >-
            {
              "channel": "stable",
              "app_id": "groovebox",
              "version": "${{ github.ref_name }}",
              "run_id": "${{ github.run_id }}",
              "sha256": "${{ steps.sha.outputs.sha256 }}",
              "repository": "${{ github.repository }}",
              "artifact_name": "tbd_frontend"
            }
```

> **Important:** `app_id` is hardcoded to `"groovebox"` — this is the
> app slug used in the CDN. Don't change it.

### 3. Your complete workflow should look like this

```yaml
name: build tbd firmware

on: [push]

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4
      - uses: actions/cache@v4
        with:
          path: |
            ~/.cache/pip
            ~/.platformio/.cache
          key: ${{ runner.os }}-pio
      - uses: actions/setup-python@v5
        with:
          python-version: '3.11'
      - name: Install PlatformIO Core
        run: pip install --upgrade platformio

      - name: Build Firmware
        run: pio run -e possan_rev_c

      - name: Upload Firmware
        uses: actions/upload-artifact@v4
        with:
          name: tbd_frontend
          path: |
            .pio/build/possan_rev_c/firmware.bin
            .pio/build/possan_rev_c/firmware.uf2
          overwrite: true

  publish-cdn:
    needs: build
    if: startsWith(github.ref, 'refs/tags/v')
    runs-on: ubuntu-latest
    steps:
      - name: Check for CDN token
        id: check
        run: |
          if [ -z "${{ secrets.PICO_CDN_TOKEN }}" ]; then
            echo "skip=true" >> "$GITHUB_OUTPUT"
            echo "::notice::PICO_CDN_TOKEN not set — skipping CDN publish"
          else
            echo "skip=false" >> "$GITHUB_OUTPUT"
          fi

      - name: Download build artifact
        if: steps.check.outputs.skip != 'true'
        uses: actions/download-artifact@v4
        with:
          name: tbd_frontend
          path: pico-artifact

      - name: Compute SHA-256
        if: steps.check.outputs.skip != 'true'
        id: sha
        run: |
          SHA=$(sha256sum pico-artifact/firmware.uf2 | cut -d' ' -f1)
          echo "sha256=${SHA}" >> "$GITHUB_OUTPUT"
          echo "SHA-256: $SHA"

      - name: Dispatch to CDN
        if: steps.check.outputs.skip != 'true'
        uses: peter-evans/repository-dispatch@v3
        with:
          token: ${{ secrets.PICO_CDN_TOKEN }}
          repository: dadamachines/dada-tbd-firmware
          event-type: pico-app-update
          client-payload: >-
            {
              "channel": "stable",
              "app_id": "groovebox",
              "version": "${{ github.ref_name }}",
              "run_id": "${{ github.run_id }}",
              "sha256": "${{ steps.sha.outputs.sha256 }}",
              "repository": "${{ github.repository }}",
              "artifact_name": "tbd_frontend"
            }
```

### 4. Building locally

```bash
# Build the shipping firmware:
pio run -e possan_rev_c

# Output files:
#   .pio/build/possan_rev_c/firmware.bin   (raw binary)
#   .pio/build/possan_rev_c/firmware.uf2   (UF2 for BOOTSEL drag-and-drop)

# Flash via BOOTSEL (hold BOOTSEL, plug USB, release):
pio run -e possan_rev_c -t upload

# Flash via debug probe (if connected):
# Uncomment debug_tool and upload_protocol in platformio.ini [env:possan_rev_c]
pio run -e possan_rev_c -t upload
```

---

## Release coordination with dadamachines

A release (e.g. v0.4.2) requires **both** repos to tag the same version.
The P4 firmware, Pico firmware, SD card image, and WebUI must all match.

### Before tagging

1. **Build locally** — `pio run -e possan_rev_c`
2. **Flash to hardware** — test with the P4 firmware build from
   `dadamachines/ctag-tbd` that will be tagged with the same version
3. **Smoke test** — boot, sequencer, SPI communication, MIDI, WebUI
   parameter control, all working together
4. **Agree on the version tag with Johannes** — both repos use the same
   semver tag (e.g. `v0.4.2`)

### Tagging the release

```bash
git tag v0.4.2
git push origin v0.4.2
```

Johannes tags the P4 repo within minutes. Both dispatches arrive at the
CDN independently — the Stable Channel flash page shows both versions.

### After tagging

Verify both dispatches landed:
```bash
curl -s https://dadamachines.github.io/dada-tbd-firmware/stable/latest.json \
  | python3 -c "import sys,json; d=json.load(sys.stdin); \
    print(f'P4: {d[\"tag\"]}  Pico: {d.get(\"picoVersion\",\"—\")}')"
```

Both should show `v0.4.2`.

---

## How to publish a release

When you're ready to push a Groovebox update to the CDN:

```bash
git tag v0.4.2
git push origin v0.4.2
```

What happens:
1. **Your CI** builds `possan_rev_c` (the Rev C shipping firmware)
2. **`publish-cdn` job** fires (because the tag starts with `v`)
3. It computes the SHA-256 of `firmware.uf2` and dispatches to the CDN
4. **CDN `receive-pico-app.yml`** downloads your artifact, verifies the
   SHA-256 matches, and commits the `.uf2` to the CDN
5. GitHub Pages deploys — the binary is now live at:
   - `dadamachines.github.io/dada-tbd-firmware/stable/pico/dada-tbd-pico.uf2`
   - `dadamachines.github.io/dada-tbd-firmware/stable/pico/dada-tbd-16-v0.4.2-pico.uf2`
   - `dadamachines.github.io/dada-tbd-firmware/apps/groovebox/groovebox-0.4.2.uf2`

The Stable Channel flash page automatically picks up the new `.uf2`.

---

## Normal pushes (no tag)

Nothing changes for normal development. Pushes without a `v*` tag still
build and upload the artifact — the `publish-cdn` job is skipped because
of the `if: startsWith(github.ref, 'refs/tags/v')` condition.

---

## How to verify it worked

After tagging, check these two Actions pages:

1. **Your repo:** `github.com/possan/tbd-pico-seq3/actions` — both
   `build` and `publish-cdn` jobs should be green
2. **CDN repo:** `github.com/dadamachines/dada-tbd-firmware/actions` —
   look for a "Receive Pico App" run triggered by your dispatch

Then verify the file is served:
```bash
curl -sI https://dadamachines.github.io/dada-tbd-firmware/stable/pico/dada-tbd-pico.uf2 | head -5
```
You should see `HTTP/2 200` with a non-zero `content-length`.

---

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| `publish-cdn` job skipped | Tag doesn't start with `v` | Use `v0.4.2`, not `0.4.2` |
| "PICO_CDN_TOKEN not set" notice | Secret missing or misspelled | Check repo Settings → Secrets → `PICO_CDN_TOKEN` |
| CDN run shows "SHA-256 mismatch" | Artifact was re-built between jobs | Re-tag and push (delete old tag first) |
| CDN run shows 404 downloading artifact | Cross-repo artifact access issue | Ask dadamachines to check — may need token adjustment |
| CDN run doesn't appear at all | Dispatch failed silently | Check the `publish-cdn` job logs for errors in the Dispatch step |
| Build fails with missing SPI pin | Wrong environment | Make sure CI builds `possan_rev_c`, not `pi2350` |

---

## Questions?

Reach out to @nevvkid (Johannes) — he manages the CDN infrastructure
and the `PICO_CDN_TOKEN` secret.
