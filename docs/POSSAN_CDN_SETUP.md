# Setting Up Auto-Publish to the dadamachines CDN

This guide walks you through the automatic CDN publishing for the
Pico app repo (`tbd-pico-seq3`). Pushing to specific branches or
tagging a release pushes your `.uf2` to the dadamachines firmware CDN
automatically — no manual file copying.

---

## What's already done

- **`PICO_CDN_TOKEN`** is installed as a secret in the repo.

- **CI pipeline** is configured on the `dada-tbd-master` branch with
  5 workflow files that mirror the P4 repo exactly.

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

The CI pipeline uses 5 separate workflow files in `.github/workflows/`,
mirroring the P4 repo structure exactly:

| Workflow | Trigger | Purpose |
|----------|---------|---------|
| `build-firmware.yml` | Called by other workflows | Reusable PlatformIO build job |
| `ci.yml` | PR / push to `dada-tbd-master` | Build-check only (no release, no CDN) |
| `create-release.yml` | Push `v*` tag | Stable release → GitHub Release → CDN stable channel |
| `staging-release.yml` | Push to `staging` branch | Pre-release → CDN staging channel |
| `feature-test-release.yml` | Push to `feature-test/*` branch | Pre-release → CDN per-feature channel |

### CDN channel mapping

| Trigger | CDN channel | Flash page | Example |
|---------|-------------|------------|---------|
| Tag `v0.5.0` | `stable` | Stable Channel | `git tag v0.5.0 && git push origin v0.5.0` |
| Push to `staging` branch | `staging` | Beta Channel | `git push origin staging` |
| Push to `feature-test/launchpad` | `feature-test-launchpad` | Beta Channel (dropdown) | `git push origin feature-test/launchpad` |

This convention is **identical for both repos** (P4 and Pico). Same
branch names, same workflow structure, same channel mapping.

### How CDN publishing works

Each release workflow (stable, staging, feature-test) includes a
`publish-cdn` job that pushes directly to the CDN repo — no intermediate
dispatch needed. This avoids the cross-repo artifact download problem
(GitHub's `GITHUB_TOKEN` can't download artifacts from private repos).

The `publish-cdn` job:
1. Downloads the build artifact
2. Computes SHA-256
3. Clones the CDN repo with `PICO_CDN_TOKEN`
4. Places `.uf2` files in `{channel}/pico/`
5. Patches `latest.json` pico fields
6. Commits and pushes

**Separation of concerns:**
- **P4 repo** (`dadamachines/ctag-tbd`) delivers P4 firmware + SD card
  image to `{channel}/p4/` on the CDN
- **Pico repo** (`possan/tbd-pico-seq3`) delivers the Pico `.uf2` to
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
needs testing on the Beta Channel flash page. Both repos push to their
`staging` branch:

```bash
# possan (Groovebox repo):
git checkout staging
git merge dada-tbd-master   # or cherry-pick specific commits
git push origin staging
# → staging-release.yml → publishes to staging/pico/ on CDN

# dadamachines (P4 repo) — Johannes does the same:
git checkout staging && git merge dada-tbd-master && git push origin staging
# → staging-release.yml → publishes to staging/p4/ on CDN
```

The Beta Channel flash page (`20_staging_channel.rst`) discovers Pico
firmware at: `staging/pico/dada-tbd-16-{tag}-pico.uf2`

### Feature test release

For isolated feature testing on a per-feature Beta Channel:

```bash
# possan:
git checkout -b feature-test/launchpad
# ... make changes ...
git push origin feature-test/launchpad
# → feature-test-release.yml → publishes to feature-test-launchpad/pico/ on CDN

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
| Staging build | `git push origin staging` | `staging/pico/` |
| Feature test | `git push origin feature-test/foo` | `feature-test-foo/pico/` |

After tagging, the CDN pico directory gets:
- `dada-tbd-pico.uf2` — unversioned alias (latest for that channel)
- `dada-tbd-16-{tag}-pico.uf2` — versioned name (what flash pages request)
- `pico-version.txt` — version string

---

## Normal pushes (no tag, not a release branch)

Nothing changes for normal development. Pushes to `dada-tbd-master`
trigger `ci.yml` which does a build-check only — no release, no CDN
publish. PRs against `dada-tbd-master` also trigger a build-check.

---

## How to verify it worked

After a release, check:

1. **Your repo:** `github.com/possan/tbd-pico-seq3/actions` — the
   relevant workflow should be green
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
| CI didn't trigger | Push was not to `dada-tbd-master`, `staging`, or `feature-test/*` | Check branch name matches a workflow trigger |
| Stable release didn't publish | Tag doesn't start with `v` | Use `v0.4.2`, not `0.4.2` |
| Push to CDN fails with 403 | Token expired or permissions changed | Ask dadamachines to regenerate the CDN token |
| CDN commit appears but Pages not updated | Pages deploy was cancelled by concurrency | Re-run the Deploy Pages workflow in CDN repo Actions tab |
| Build fails with missing SPI pin | Wrong environment | Make sure CI builds `possan_rev_c`, not `pi2350` |

---

## Questions?

Reach out to @nevvkid (Johannes) — he manages the CDN infrastructure
and the `PICO_CDN_TOKEN` secret.
