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

- **CI pipeline** is already configured on the `ci/cdn-pipeline` branch.
  The `publish-cdn` job pushes directly to the CDN repo using the token —
  no intermediate dispatch workflow needed.

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

## The complete workflow

Your `.github/workflows/build_firmware.yml` on the `ci/cdn-pipeline`
branch builds `possan_rev_c` on every push and publishes to the CDN on
tagged releases. **The tag name determines the CDN channel:**

| Tag pattern | CDN channel | Flash page | Example |
|-------------|-------------|------------|---------|
| `v0.5.0` (clean semver) | `stable` | Stable Channel | `git tag v0.5.0` |
| `v0.5.0-staging` | `staging` | Beta Channel | `git tag v0.5.0-staging` |
| `v0.5.0-staging.2` | `staging` | Beta Channel | `git tag v0.5.0-staging.2` |
| `v0.5.0-ft-launchpad` | `feature-test-launchpad` | Beta Channel (dropdown) | `git tag v0.5.0-ft-launchpad` |

This convention is **identical for both repos** (P4 and Pico). Same tag
format, same channel derivation, same version number when coordinated.

### Channel derivation logic (in CI)

```bash
# v0.5.0           → stable
# v0.5.0-staging   → staging
# v0.5.0-staging.2 → staging
# v0.5.0-ft-foo    → feature-test-foo
if echo "$VERSION" | grep -qE '^v[0-9]+\.[0-9]+\.[0-9]+$'; then
  CHANNEL="stable"
elif echo "$VERSION" | grep -qE -- '-staging'; then
  CHANNEL="staging"
elif echo "$VERSION" | grep -qoE -- '-ft-(.+)'; then
  CHANNEL="feature-test-$(echo "$VERSION" | sed -E 's/.*-ft-//')"
else
  CHANNEL="stable"
fi
```

### The CI workflow file

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
          name: groovebox-firmware
          path: |
            .pio/build/possan_rev_c/firmware.bin
            .pio/build/possan_rev_c/firmware.uf2
          overwrite: true

  # ────────────────────────────────────────────────────────────
  # Publish to dadamachines firmware CDN on tagged releases.
  # Pushes the .uf2 directly to the CDN repo using PICO_CDN_TOKEN.
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
          name: groovebox-firmware
          path: pico-artifact

      - name: Compute SHA-256
        if: steps.check.outputs.skip != 'true'
        id: sha
        run: |
          SHA=$(sha256sum pico-artifact/firmware.uf2 | cut -d' ' -f1)
          echo "sha256=${SHA}" >> "$GITHUB_OUTPUT"
          echo "SHA-256: $SHA"

      - name: Push to CDN
        if: steps.check.outputs.skip != 'true'
        env:
          CDN_TOKEN: ${{ secrets.PICO_CDN_TOKEN }}
          VERSION: ${{ github.ref_name }}
          SHA256: ${{ steps.sha.outputs.sha256 }}
        run: |
          set -euo pipefail

          # ── Derive channel from tag name ──
          if echo "$VERSION" | grep -qE '^v[0-9]+\.[0-9]+\.[0-9]+$'; then
            CHANNEL="stable"
          elif echo "$VERSION" | grep -qE -- '-staging'; then
            CHANNEL="staging"
          elif echo "$VERSION" | grep -qoE -- '-ft-(.+)' >/dev/null 2>&1; then
            CHANNEL="feature-test-$(echo "$VERSION" | sed -E 's/.*-ft-//')"
          else
            CHANNEL="stable"
          fi

          APP_ID="groovebox"
          CDN_REPO="dadamachines/dada-tbd-firmware"

          echo "Channel: $CHANNEL  Version: $VERSION"

          git clone --depth 1 \
            "https://x-access-token:${CDN_TOKEN}@github.com/${CDN_REPO}.git" cdn
          cd cdn

          git config user.name "github-actions[bot]"
          git config user.email "github-actions[bot]@users.noreply.github.com"

          # Place .uf2 in channel pico directory
          mkdir -p "${CHANNEL}/pico"
          cp -f ../pico-artifact/firmware.uf2 \
            "${CHANNEL}/pico/dada-tbd-pico.uf2"
          cp -f ../pico-artifact/firmware.uf2 \
            "${CHANNEL}/pico/dada-tbd-16-${VERSION}-pico.uf2"

          # App catalog — stable releases only
          if [ "$CHANNEL" = "stable" ]; then
            mkdir -p "apps/${APP_ID}"
            cp -f ../pico-artifact/firmware.uf2 \
              "apps/${APP_ID}/${APP_ID}-${VERSION#v}.uf2"
          fi

          # Update latest.json pico fields
          if [ -f "${CHANNEL}/latest.json" ]; then
            TMP=$(mktemp)
            jq --arg pico "${CHANNEL}/pico/dada-tbd-16-${VERSION}-pico.uf2" \
               --arg ver "${VERSION}" \
               '.files.pico = $pico | .picoVersion = $ver' \
               "${CHANNEL}/latest.json" > "$TMP" \
               && mv "$TMP" "${CHANNEL}/latest.json"
          fi

          echo "${VERSION}" > "${CHANNEL}/pico/pico-version.txt"

          git add -A
          git diff --cached --quiet \
            && echo "No changes to commit" && exit 0
          git commit -m \
            "Pico app: ${APP_ID} ${VERSION} → ${CHANNEL} [sha256:${SHA256:0:12}]"
          git push
```

### How it works

The `publish-cdn` job pushes directly to the CDN repo —
no intermediate `receive-pico-app.yml` dispatch needed. This avoids the
cross-repo artifact download problem (GitHub's `GITHUB_TOKEN` can't
download artifacts from private repos).

**Separation of concerns:**
- **P4 repo** (`dadamachines/ctag-tbd`) delivers P4 firmware + SD card
  image + hash to `{channel}/p4/` on the CDN
- **Pico repo** (`possan/tbd-pico-seq3`) delivers the Pico .uf2 to
  `{channel}/pico/` on the CDN

Neither repo touches the other's files. Both patch their own fields
into `{channel}/latest.json`. The flash page reads the combined
manifest and flashes both.

### Building locally

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

## Release workflows

### Stable release (production)

A stable release requires **both** repos to tag the same version.
The P4 firmware, Pico firmware, SD card image, and WebUI must all match.

**Before tagging:**

1. **Build locally** — `pio run -e possan_rev_c`
2. **Flash to hardware** — test with the P4 firmware build from
   `dadamachines/ctag-tbd` that will be tagged with the same version
3. **Smoke test** — boot, sequencer, SPI communication, MIDI, WebUI
   parameter control — all working together
4. **Agree on the version tag with Johannes** — both repos use the same
   semver tag (e.g. `v0.5.0`)

**Tag both repos:**

```bash
# possan (Groovebox repo):
git tag v0.5.0
git push origin v0.5.0
# → CI builds → publishes to stable/pico/ on CDN

# dadamachines (P4 repo) — Johannes does this within minutes:
git tag v0.5.0 && git push origin v0.5.0
# → CI builds → publishes to stable/p4/ on CDN
```

**Verify CDN convergence:**
```bash
curl -s https://dadamachines.github.io/dada-tbd-firmware/stable/latest.json \
  | python3 -c "import sys,json; d=json.load(sys.stdin); \
    print(f'P4: {d[\"tag\"]}  Pico: {d.get(\"picoVersion\",\"—\")}')"
# Both should show v0.5.0
```

### Staging release (Beta Channel)

Use staging when you and Johannes are developing a new feature that
needs testing on the Beta Channel flash page. Both repos tag with
`-staging` suffix:

```bash
# possan (Groovebox repo):
git tag v0.5.0-staging
git push origin v0.5.0-staging
# → CI builds → publishes to staging/pico/ on CDN

# dadamachines (P4 repo) — Johannes pushes to staging branch:
git checkout staging && git merge feature/new-thing && git push origin staging
# → staging-release.yml → publishes to staging/p4/ on CDN
```

The Beta Channel flash page (`20_staging_channel.rst`) discovers Pico
firmware at: `staging/pico/dada-tbd-16-{tag}-pico.uf2`

**Staging iteration:** If you need multiple staging builds, use a
revision suffix:

```bash
git tag v0.5.0-staging.2
git push origin v0.5.0-staging.2
```

### Feature test release

For isolated feature testing on a per-feature Beta Channel:

```bash
# possan:
git tag v0.5.0-ft-launchpad
git push origin v0.5.0-ft-launchpad
# → publishes to feature-test-launchpad/pico/ on CDN

# dadamachines:
git push origin feature-test/launchpad
# → feature-test-release.yml → publishes to feature-test-launchpad/p4/
```

The Beta Channel flash page shows feature-test channels in its dropdown.

---

## Quick reference

| What you want | Command | CDN target |
|---------------|---------|------------|
| Stable release | `git tag v0.5.0 && git push origin v0.5.0` | `stable/pico/` + `apps/groovebox/` |
| Staging build | `git tag v0.5.0-staging && git push origin v0.5.0-staging` | `staging/pico/` |
| Feature test | `git tag v0.5.0-ft-foo && git push origin v0.5.0-ft-foo` | `feature-test-foo/pico/` |

After tagging, the CDN pico directory gets:
- `dada-tbd-pico.uf2` — unversioned alias (latest for that channel)
- `dada-tbd-16-{tag}-pico.uf2` — versioned name (what flash pages request)
- `pico-version.txt` — version string

---

## Normal pushes (no tag)

Nothing changes for normal development. Pushes without a `v*` tag still
build and upload the artifact — the `publish-cdn` job is skipped because
of the `if: startsWith(github.ref, 'refs/tags/v')` condition.

---

## How to verify it worked

After tagging, check:

1. **Your repo:** `github.com/possan/tbd-pico-seq3/actions` — both
   `build` and `publish-cdn` jobs should be green
2. **CDN repo:** `github.com/dadamachines/dada-tbd-firmware` — look for
   a commit from `github-actions[bot]` with the version and channel

Then verify the file is served (replace `{channel}` with `stable`,
`staging`, or `feature-test-{name}`):

```bash
curl -sI "https://dadamachines.github.io/dada-tbd-firmware/{channel}/pico/dada-tbd-pico.uf2" | head -5
```
You should see `HTTP/2 200` with a non-zero `content-length`.

---

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| `publish-cdn` job skipped | Tag doesn't start with `v` | Use `v0.4.2`, not `0.4.2` |
| "PICO_CDN_TOKEN not set" notice | Secret missing or misspelled | Check repo Settings → Secrets → `PICO_CDN_TOKEN` |
| Push to CDN fails with 403 | Token expired or permissions changed | Ask dadamachines to regenerate `PICO_CDN_TOKEN` |
| CDN commit appears but Pages not updated | Pages deploy was cancelled by concurrency | Re-run the Deploy Pages workflow in CDN repo Actions tab |
| Build fails with missing SPI pin | Wrong environment | Make sure CI builds `possan_rev_c`, not `pi2350` |

---

## Questions?

Reach out to @nevvkid (Johannes) — he manages the CDN infrastructure
and the `PICO_CDN_TOKEN` secret.
