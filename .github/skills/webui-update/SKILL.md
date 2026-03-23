---
name: webui-update
description: 'Create and manage WebUI update packages for TBD-16. USE FOR: building update .zip packages; bumping WebUI version; listing changed files for an update; adding new macro definitions/presets to an update; creating a release-ready update package; publishing updates to GitHub Pages for online auto-update; understanding the update manifest format. DO NOT USE FOR: editing WebUI source code; building firmware; flashing firmware to device; full SD card deploy (use tbd-deploy skill). TRIGGER WORDS: update package, WebUI update, create update, bump version, release WebUI, package update, webui-update.html, manifest.json, publish update, online update, latest.json.'
argument-hint: 'Describe what changed: e.g. "JS fixes only", "new rompler macro", "full WebUI refresh"'
---

# WebUI Update Package Builder

Create update packages (.zip) that users can apply via `http://192.168.4.1/webui-update.html` — no MSC mode, no SD card erase, no firmware flash needed.

## When to Use

- User says "create update package", "build an update", "package the WebUI changes"
- User says "bump the WebUI version"
- User made WebUI changes and wants to distribute them without a full deploy
- User wants to add/update macro definitions or presets for distribution
- After fixing bugs in JS/HTML/CSS files and wanting to ship the fix

## When NOT to Use

- Firmware C++ changes → use `tbd-deploy` skill instead
- Full SD card erase + reimage → use `tbd-deploy` skill (type: fresh-sd)
- Writing or editing WebUI source code (JS, HTML, CSS) — just edit files directly

## Key Facts

- Workspace root: the folder containing `CMakeLists.txt`, `sdcard_image/`, `build/`
- Package script: `create_webui_update.sh` at workspace root
- Version file: `sdcard_image/data/webui-version.json`
- Build script: `sdcard_image/www/build-webui.sh`
- Online update manifest: `docs/_static/updates/latest.json` (pointer — URLs resolve to CDN)
- CDN hosting: `https://dadamachines.github.io/dada-tbd-firmware/webui-updates/`
- CDN repo: `../dada-tbd-firmware/webui-updates/` (sibling directory)
- WebUI versions doc: `docs/flash/70_webui_versions.rst` — **must be updated every release**
- The device's updater page auto-checks `latest.json` on load to offer one-click online updates
- The updater supports **reinstall** (re-apply current version) and **older versions** (roll back)
- `latest.json` includes a `versions` array for the older-versions list (backwards-compatible)
- Updater-update docs page: `docs/flash/68_update_updater.rst` — pushes `webui-update.html.gz` to the device
- Hosted updater copy: `docs/_static/updater/webui-update.html.gz` — must be rebuilt when `webui-update.html` changes
- All `www/` assets on SD card are `.gz` — include the `.gz` extension
- All `data/` files are plain JSON — no `.gz` extension
- File paths in the manifest are relative to `sdcard_image/` (e.g., `www/js/app-bundle.js.gz`)
- Output goes to `build/webui-update-v<VERSION>.zip`
- Update packages do NOT touch the SD card hash — no destructive re-extraction

## NEVER Do These

1. **NEVER** include a `www/` file without the `.gz` extension — the server only serves `.gz` files
2. **NEVER** forget to include `data/webui-version.json` — users won't see the version update
3. **NEVER** skip `build-webui.sh` before packaging — you'll ship stale bundles
4. **NEVER** include files from `build/` — only from `sdcard_image/`
5. **NEVER** set an older version number than what's already installed
6. **NEVER** skip updating `docs/flash/70_webui_versions.rst` — every release must appear in the version history
7. **NEVER** pick a version number without checking upstream first — if upstream already has that version, you'll get merge conflicts on the PR

## File Mapping Reference

Which source files map to which package files:

| Source files (what you edit) | Bundle / Package path |
|------------------------------|----------------------|
| `www/index.html` | `www/index.html.gz` |
| `www/preset-macro-manager.html` | `www/preset-macro-manager.html.gz` |
| `www/webui-update.html` | `www/webui-update.html.gz` |
| `www/css/app.css` | `www/css/app.css.gz` |
| `www/js/shared.js` | Both `www/js/app-bundle.js.gz` AND `www/js/macro-bundle.js.gz` |
| `www/js/display-hints.js` | Both `www/js/app-bundle.js.gz` AND `www/js/macro-bundle.js.gz` |
| `www/js/plugin-manager.js` | `www/js/app-bundle.js.gz` |
| `www/js/sample-manager.js` | `www/js/app-bundle.js.gz` |
| `www/js/app.js` | `www/js/app-bundle.js.gz` |
| `www/js/factory-manifest.js` | `www/js/macro-bundle.js.gz` |
| `www/js/performer.js` | `www/js/macro-bundle.js.gz` |
| `www/js/designer.js` | `www/js/macro-bundle.js.gz` |
| `www/js/track-defaults.js` | `www/js/macro-bundle.js.gz` |
| `www/js/preset-macro-app.js` | `www/js/macro-bundle.js.gz` |
| `data/macrodefinitions/*.json` | `data/macrodefinitions/<name>.json` |
| `data/macrosoundpresets/*.json` | `data/macrosoundpresets/<name>.json` |
| `data/trackdefaults.json` | `data/trackdefaults.json` |
| `data/webui-version.json` | `data/webui-version.json` |

**Important**: `shared.js` and `display-hints.js` are in BOTH bundles. If you change either, include both `app-bundle.js.gz` and `macro-bundle.js.gz`.

---

## Procedure: Create Update Package

### Step 0 — Check upstream for version conflicts

**This step is mandatory.** Before choosing a version number, fetch upstream and check the latest version to avoid conflicts:

```bash
cd <workspace_root>
git fetch upstream
git log --oneline upstream/dada-tbd-master -3
cat <(git show upstream/dada-tbd-master:sdcard_image/data/webui-version.json)
```

The new version must be **strictly greater** than whatever upstream has. If upstream already has v0.3.3, use v0.3.4 or higher.

Also check if your branch is behind upstream. If it is, rebase first:

```bash
git rebase upstream/dada-tbd-master
```

Resolve any conflicts before proceeding. This prevents merge conflicts when opening a PR against `https://github.com/dadamachines/ctag-tbd/`.

### Step 1 — Identify changed files

Look at `git diff --name-only` or the user's description to determine what changed.

```bash
cd <workspace_root>
git diff --name-only HEAD
```

Map each changed source file to its package path using the File Mapping Reference above.

### Step 2 — Build WebUI bundles

Always rebuild bundles before packaging, even if you think nothing changed:

```bash
cd sdcard_image/www && bash build-webui.sh && cd ../..
```

This produces fresh `app-bundle.js`, `macro-bundle.js`, and all `.gz` files.

### Step 3 — Bump the version

Edit `sdcard_image/data/webui-version.json`:

```json
{
  "version": "1.0.1",
  "date": "2026-03-12",
  "description": "Brief description of what changed"
}
```

Use semantic versioning:
- **Patch** (1.0.x): Bug fixes, small CSS tweaks
- **Minor** (1.x.0): New features, new macro definitions, significant UI changes
- **Major** (x.0.0): Breaking changes, major redesigns

### Step 4 — Run the package script

```bash
bash create_webui_update.sh <VERSION> "<DESCRIPTION>" <FILE1> [FILE2] ...
```

**Example — JS bug fixes only:**
```bash
bash create_webui_update.sh 1.0.1 "Fixed macro/preset creation bugs" \
  www/js/app-bundle.js.gz \
  www/js/macro-bundle.js.gz \
  data/webui-version.json
```

**Example — Full WebUI refresh (most common):**
```bash
bash create_webui_update.sh 1.1.0 "New features and bug fixes" \
  www/index.html.gz \
  www/js/app-bundle.js.gz \
  www/js/macro-bundle.js.gz \
  www/preset-macro-manager.html.gz \
  www/webui-update.html.gz \
  www/css/app.css.gz \
  data/webui-version.json
```

**Example — New macro definitions + presets:**
```bash
bash create_webui_update.sh 1.0.2 "Added new rompler macros" \
  data/macrodefinitions/ro-allnewrompler.json \
  data/macrosoundpresets/allnewrompler-def.json \
  data/webui-version.json
```

**Example — Everything (HTML + JS + CSS + data):**
```bash
bash create_webui_update.sh 1.2.0 "Major update with new macros and UI fixes" \
  www/index.html.gz \
  www/js/app-bundle.js.gz \
  www/js/macro-bundle.js.gz \
  www/preset-macro-manager.html.gz \
  www/webui-update.html.gz \
  www/css/app.css.gz \
  data/macrodefinitions/ro-allnewrompler.json \
  data/macrosoundpresets/allnewrompler-def.json \
  data/trackdefaults.json \
  data/webui-version.json
```

**Example — Include ALL macro definitions and presets (common for major releases):**

When you want every definition and preset, dynamically expand the file list with shell subshells. There can be 70+ data files, so never try to type them all by hand:

```bash
bash create_webui_update.sh 1.3.0 "Full refresh with all macros" \
  www/js/macro-bundle.js.gz \
  data/webui-version.json \
  $(cd sdcard_image && find data/macrodefinitions -name '*.json' -type f | sort) \
  $(cd sdcard_image && find data/macrosoundpresets -name '*.json' -type f | sort)
```

You can combine this with any `www/` files too. The `find` subshells produce paths relative to `sdcard_image/`, which is exactly what the script expects.

### Step 5 — Verify the output

The script prints a summary. Check:
- All files validated (✓ marks)
- manifest.json looks correct
- Output zip created at `build/webui-update-v<VERSION>.zip`

Optionally inspect the zip:
```bash
unzip -l build/webui-update-v<VERSION>.zip
```

### Step 6 — Update the WebUI versions docs page

Add the new version to `docs/flash/70_webui_versions.rst`. This page has a single table with Version, Date, Description, and Download columns. Add a new row at the top (below the header):

```rst
   * - v<VERSION>
     - <DATE>
     - <DESCRIPTION>
     - `zip <https://dadamachines.github.io/dada-tbd-firmware/webui-updates/webui-update-v<VERSION>.zip>`__
```

This step is **required** — every release must appear in the versions page so users can see the changelog and download older packages.

### Step 7 — Report to user

Tell the user:
- The update package path: `build/webui-update-v<VERSION>.zip`
- How to apply it: upload via `http://192.168.4.1/webui-update.html`
- What's included (list the files)
- That `docs/flash/70_webui_versions.rst` was updated

---

## Procedure: Quick Decision — What Files to Include

Use this decision tree when the user just says "create an update" without specifying files:

1. **Did any `.html` file change?** → Include each changed `.html.gz`
2. **Did any JS source used by `app-bundle` change?** (shared.js, display-hints.js, plugin-manager.js, sample-manager.js, app.js, webaudio-controls.js) → Include `www/js/app-bundle.js.gz`
3. **Did any JS source used by `macro-bundle` change?** (shared.js, display-hints.js, factory-manifest.js, performer.js, designer.js, track-defaults.js, preset-macro-app.js) → Include `www/js/macro-bundle.js.gz`
4. **Did `app.css` change?** → Include `www/css/app.css.gz`
5. **Did any macro definition change?** → Include each `data/macrodefinitions/<name>.json`
6. **Did any macro preset change?** → Include each `data/macrosoundpresets/<name>.json`
7. **Did trackdefaults.json change?** → Include `data/trackdefaults.json`
8. **ALWAYS** include `data/webui-version.json`
9. **When in doubt**, include all WebUI files — the update is idempotent

---

## Procedure: Publish Update for Online Auto-Update

After creating an update package and updating the docs page (Steps 1–6 above), the `create_webui_update.sh` script automatically copies the zip to the CDN repo (`../dada-tbd-firmware/webui-updates/`) and updates `latest.json` in both docs and CDN. To make it live:

### Step 1 — Verify CDN repo

```bash
cat docs/_static/updates/latest.json
ls -la ../dada-tbd-firmware/webui-updates/*.zip
```

Confirm `latest.json` has the correct version, URL, size, and `versions` array.

The `versions` array in `latest.json` lists previously released versions for the "Older versions" feature:

```json
{
  "version": "1.2.0",
  "date": "2026-03-15",
  "url": "https://dadamachines.github.io/dada-tbd-firmware/webui-updates/webui-update-v1.2.0.zip",
  "size": 180000,
  "versions": [
    {
      "version": "1.1.0",
      "date": "2026-03-12",
      "url": "https://dadamachines.github.io/dada-tbd-firmware/webui-updates/webui-update-v1.1.0.zip",
      "description": "Previous feature release"
    }
  ]
}
```

When adding a new version, move the current `latest.json` top-level entry into the `versions` array before updating the top-level fields.

### Step 2 — Commit and push both repos

Firmware repo (latest.json pointer + docs + version file):

```bash
git add docs/_static/updates/latest.json \
  sdcard_image/data/webui-version.json \
  docs/flash/70_webui_versions.rst
git commit -m "WebUI v<VERSION> — <short description>"
git push
```

CDN repo (zip + latest.json):

```bash
cd ../dada-tbd-firmware
git add webui-updates/
git commit -m "WebUI v<VERSION> update package"
git push
```

**If `webui-update.html` was changed**, also rebuild and commit the hosted updater copy:

```bash
cd sdcard_image/www && gzip -k -f webui-update.html && cd ../..
cp sdcard_image/www/webui-update.html.gz docs/_static/updater/webui-update.html.gz
git add docs/_static/updater/webui-update.html.gz
git commit -m "Update hosted WebUI Updater copy"
```

### Step 3 — Deploy docs (if needed)

If CI/CD auto-deploys docs to GitHub Pages, you're done. Otherwise, build and deploy manually:

```bash
# Build docs (if Sphinx is set up)
make -f docs/config/Makefile html
# Then push or deploy the built output to GitHub Pages
```

The update will be available at:
`https://dadamachines.github.io/dada-tbd-firmware/webui-updates/latest.json`

### How Online Updates Work

1. User opens `http://192.168.4.1/webui-update.html`
2. Page fetches installed version from device API
3. Page fetches `latest.json` from GitHub Pages
4. If remote version > installed version, shows "Update Available" banner with one-click install
5. User clicks "Install Update" → browser downloads zip from GitHub Pages → parses it → uploads files to device via REST API
6. If versions match, user can click "Reinstall" to re-apply the current version (factory reset)
7. Expandable "Older versions" list lets users roll back to any version in the `versions` array
8. Manual drag-drop upload remains available as a fallback

---

## Troubleshooting

| Problem | Cause | Fix |
|---------|-------|-----|
| "File not found: sdcard_image/www/js/app-bundle.js.gz" | Forgot to run build-webui.sh | Run `cd sdcard_image/www && bash build-webui.sh && cd ../..` first |
| Update applied but page looks the same | Browser cached old JS (30-day immutable cache) | Hard refresh (Cmd+Shift+R) or clear browser cache |
| Version not showing in footer | `webui-version.json` missing from device | Include `data/webui-version.json` in the update |
| "Unknown path prefix" error during apply | File path doesn't start with `www/` or `data/` | Fix manifest — all paths must start with `www/` or `data/` |
| "No manifest.json found" | Zip structure wrong | manifest.json must be at root of zip, not in a subdirectory |
| Online check says "Could not check" | GitHub Pages not deployed, or no internet | Push `docs/_static/updates/latest.json` and rebuild docs site |
| Online check says "up to date" but shouldn't | `latest.json` version ≤ installed version | Update `latest.json` with the new version and re-push |
| Online install fails to download | CORS blocked or wrong URL in `latest.json` | Verify the zip URL in `latest.json` is correct and the file is deployed |
