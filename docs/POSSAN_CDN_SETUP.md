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

## What you need to do

### 1. Add the `publish-cdn` job to your workflow

Open `.github/workflows/build_firmware.yml` and add this job **after**
your existing `build` job (same indentation level as `build:`):

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

### 2. Your complete workflow should look like this

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
        run: pio run -e pi2350

      - name: Upload Firmware
        uses: actions/upload-artifact@v4
        with:
          name: tbd_frontend
          path: |
            .pio/build/pi2350/firmware.bin
            .pio/build/pi2350/firmware.uf2
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

---

## How to publish a release

When you're ready to push a Groovebox update to the CDN:

```bash
git tag v0.4.2
git push origin v0.4.2
```

What happens:
1. **Your CI** builds the firmware (same as any push)
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

---

## Questions?

Reach out to @nevvkid (Johannes) — he manages the CDN infrastructure
and the `PICO_CDN_TOKEN` secret.
