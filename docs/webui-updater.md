# WebUI Updater

Update the TBD-16 WebUI (HTML, JS, CSS) and macro definitions/presets without a full SD card erase + re-deploy.

## Problem

Previously, every WebUI change required:
1. Building the full SD card archive
2. Switching to MSC mode
3. Erasing the entire SD card
4. Extracting the new archive
5. Setting hash files
6. Switching back to ota_0

This workflow is slow, requires the device physically connected, and wipes all user data (presets, samples) unless backed up.

## Solution

The WebUI Updater writes individual files directly to the SD card over HTTP using the existing REST API. No MSC mode, no erase, no hash changes — just targeted file replacement.

## Architecture

### Components

| Component | Path | Purpose |
|-----------|------|---------|
| Updater page | `sdcard_image/www/webui-update.html` | Self-contained HTML page with ZIP parser, manifest reader, upload UI |
| Upload endpoint | `SampleAPI.cpp` → `uploadwww` | New API action: writes files to `/sdcard/www/` |
| Config endpoint | `SampleAPI.cpp` → `uploadconfig` | Existing API action: writes files to `/sdcard/data/` |
| Version file | `sdcard_image/data/webui-version.json` | Tracks installed WebUI version |
| Package script | `create_webui_update.sh` | Builds update .zip with manifest.json |

### API Endpoints

**Upload WebUI files** (new):
```
POST /api/v2/samples?action=uploadwww&path=<relative-path>
Body: raw file bytes
```
Writes to `/sdcard/www/<relative-path>`. Has path traversal protection (`..` rejected).

**Upload data files** (existing):
```
POST /api/v2/samples?action=uploadconfig&path=<relative-path>
Body: raw file bytes
```
Writes to `/sdcard/data/<relative-path>`.

**Read version**:
```
GET /api/v2/samples?getconfig=webui-version.json
```
Returns the installed WebUI version JSON.

### Update Package Format

A `.zip` file containing:
```
manifest.json           ← required, describes the update
www/index.html.gz       ← files to write (paths match SD card structure)
www/js/app-bundle.js.gz
data/macrodefinitions/my-macro.json
...
```

**manifest.json** structure:
```json
{
  "version": "1.0.1",
  "date": "2026-03-12",
  "description": "Fixed rompler macro creation",
  "files": [
    "www/index.html.gz",
    "www/js/app-bundle.js.gz",
    "data/macrodefinitions/my-macro.json"
  ]
}
```

Rules:
- `www/` files → uploaded via `uploadwww` action (path prefix stripped)
- `data/` files → uploaded via `uploadconfig` action (path prefix stripped)
- All `www/` assets must be `.gz` (that's what the server serves)
- `data/` files are plain JSON (not gzipped)
- The `files` array must list every file in the zip (excluding manifest.json)

### SD Card Hash Interaction

The WebUI Updater does **not** modify `tbd-sd-card-hash.txt` or `.version`. This means:
- No destructive re-extraction on reboot
- Updated files persist across reboots
- A future full firmware deploy (with `create_sd_archive.sh`) will overwrite files updated this way — this is expected and correct

### Version Tracking

- `sdcard_image/data/webui-version.json` stores the current version
- Both `index.html` and `preset-macro-manager.html` show the WebUI version in the footer
- The version links to `/webui-update.html` for easy access
- After a successful update, the version file is updated on the device automatically

## User Workflow

1. Open `http://192.168.4.1/webui-update.html` in a browser
2. Drop the update `.zip` onto the page (or click to browse)
3. Review the manifest: version, description, file list
4. Click **Apply Update**
5. Watch progress — each file uploads individually with status indicators
6. On success, reload the main page to see changes

## Developer Workflow

### Creating an Update Package

1. Make your changes to WebUI source files (JS, HTML, CSS) and/or data files
2. Build the bundles:
   ```bash
   cd sdcard_image/www && bash build-webui.sh && cd ../..
   ```
3. Bump the version in `sdcard_image/data/webui-version.json`
4. Run the package script:
   ```bash
   bash create_webui_update.sh 1.0.1 "Description of changes" \
     www/index.html.gz \
     www/js/app-bundle.js.gz \
     www/js/macro-bundle.js.gz \
     www/preset-macro-manager.html.gz \
     data/webui-version.json
   ```
5. Output: `build/webui-update-v1.0.1.zip`

### Which Files to Include

| Changed | Include in package |
|---------|-------------------|
| `index.html` | `www/index.html.gz` |
| Any JS in `app-bundle` (shared.js, plugin-manager.js, sample-manager.js, app.js, display-hints.js) | `www/js/app-bundle.js.gz` |
| Any JS in `macro-bundle` (shared.js, performer.js, designer.js, track-defaults.js, preset-macro-app.js) | `www/js/macro-bundle.js.gz` |
| `preset-macro-manager.html` | `www/preset-macro-manager.html.gz` |
| `webui-update.html` | `www/webui-update.html.gz` |
| `css/app.css` | `www/css/app.css.gz` |
| Macro definitions | `data/macrodefinitions/<name>.json` |
| Macro presets | `data/macrosoundpresets/<name>.json` |
| Track defaults | `data/trackdefaults.json` |
| Version file | `data/webui-version.json` (always include) |

## Online Auto-Update

The updater page can check for updates hosted on GitHub Pages and install them with one click.

### How It Works

1. User opens `http://192.168.4.1/webui-update.html`
2. Page fetches installed version from the device API
3. Page fetches `https://dadamachines.github.io/ctag-tbd/_static/updates/latest.json`
4. If the remote version is newer → shows "Update Available" banner
5. User clicks **Install Update** → browser downloads the zip from GitHub Pages, parses it, uploads files to device
6. Manual drag-drop upload remains available as a fallback (for offline use)

### Hosting Structure

```
docs/_static/updates/
  latest.json                         ← points to the latest version + download URL
  webui-update-v1.0.1.zip             ← update package
  webui-update-v1.1.0.zip             ← newer update package
```

**latest.json** format:
```json
{
  "version": "1.1.0",
  "date": "2026-03-15",
  "description": "New features and fixes",
  "url": "https://dadamachines.github.io/ctag-tbd/_static/updates/webui-update-v1.1.0.zip",
  "size": 98304
}
```

### Publishing an Online Update

The `create_webui_update.sh` script automatically copies the zip to `docs/_static/updates/` and updates `latest.json`. To make it live:

1. Create the update package as usual (see Developer Workflow above)
2. Commit the docs changes: `git add docs/_static/updates/ && git commit`
3. Push and ensure GitHub Pages rebuilds
4. The device updater page will find the new version on next load

### Typical Full WebUI Update

When most WebUI files changed:
```bash
bash create_webui_update.sh 1.1.0 "Full WebUI refresh" \
  www/index.html.gz \
  www/js/app-bundle.js.gz \
  www/js/macro-bundle.js.gz \
  www/preset-macro-manager.html.gz \
  www/webui-update.html.gz \
  www/css/app.css.gz \
  data/webui-version.json
```

### Adding New Macro Definitions

```bash
bash create_webui_update.sh 1.0.2 "New rompler macros" \
  data/macrodefinitions/ro-allnewrompler.json \
  data/macrosoundpresets/allnewrompler-def.json \
  data/webui-version.json
```
