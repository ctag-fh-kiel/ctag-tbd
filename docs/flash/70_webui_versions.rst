********************
WebUI Versions
********************

The TBD-16 WebUI can be updated independently of the firmware using the
built-in updater at `http://192.168.4.1/webui-update.html <http://192.168.4.1/webui-update.html>`__.

How to Update
=============

1. Open `http://192.168.4.1/webui-update.html <http://192.168.4.1/webui-update.html>`__ in your browser
   (Chrome, Edge, or Opera — device must be connected via USB).
2. The page will automatically check for online updates.
   If a newer version is available, click **Install Update**.
3. Alternatively, download a ``.zip`` package from below and drag it onto the
   updater page.
4. Click **Apply Update** and wait for all files to upload.
5. Hard-refresh the main page (``Cmd+Shift+R`` / ``Ctrl+Shift+R``) to see changes.

.. tip::
   The current WebUI version is shown in the footer of both the main page
   and the macro/preset manager. Click the version link to open the updater.

Version History
===============

.. list-table::
   :header-rows: 1
   :widths: 10 12 58 20

   * - Version
     - Date
     - Description
     - Download
   * - **0.5.1**
     - 2026-04-13
     - NRPM parameters (possan), OTA PSRAM buffer for reliable firmware
       uploads, updater UX fixes (compatibility warning, Pico sidecar
       version detection, OTA retry logic). Includes updated factory
       macro definitions and presets.
     - `zip <https://dadamachines.github.io/dada-tbd-firmware/webui-updates/webui-update-v0.5.1.zip>`__
   * - **0.4.6**
     - 2026-04-01
     - Fix Pico version comparison for dev builds. Non-semver versions
       (bare git hash/number) were incorrectly treated as newer than
       official releases. BOOTSEL step kept visible as picoboot3 fallback.
     - `zip <https://dadamachines.github.io/dada-tbd-firmware/webui-updates/webui-update-v0.4.6.zip>`__
   * - **0.4.5**
     - 2026-04-01
     - Version bump for coordinated v0.4.5 release (P4 + Pico + WebUI).
       Includes Pico firmware update via System Updater and all prior v0.4.1
       features.
     - `zip <https://dadamachines.github.io/dada-tbd-firmware/webui-updates/webui-update-v0.4.5.zip>`__
   * - **0.4.1**
     - 2026-04-01
     - System Updater: added Pico firmware update section with CDN check,
       download, and SD card deploy. Page renamed from "WebUI Updater" to
       "System Updater". New ``uploadsystem`` REST endpoint for writing to
       ``/sdcard/system/``. Pico version exposed in ``getAppInfo`` API.
     - `zip <https://dadamachines.github.io/dada-tbd-firmware/webui-updates/webui-update-v0.4.1.zip>`__
   * - **0.4.0**
     - 2026-03-23
     - Full data migration: ``.jsn`` → ``.json`` rename for all 114 sound
       processor files, ``favs.json``, ``spm-config.json``. All macro
       definitions and sound presets included. CI pipeline and repo cleanup.
       **Required for v0.4.x firmware** — Path A users must install this.
     - `zip <https://dadamachines.github.io/dada-tbd-firmware/webui-updates/webui-update-v0.4.0.zip>`__
   * - **0.3.5**
     - 2026-03-19
     - Volume multiplier control with factory PIN unlock. Boot Defaults
       dialog now shows macro definition hint per track. Targeted single-
       definition reload (fixes crash on save). Export now includes live
       knob values; bulk import implemented.
     - `zip <https://dadamachines.github.io/dada-tbd-firmware/webui-updates/webui-update-v0.3.5.zip>`__
   * - **0.3.4**
     - 2026-03-13
     - New WebUI Updater with light theme matching Shoelace design,
       reinstall support (re-apply current version for factory reset),
       and expandable version history to install older releases.
     - `zip <https://dadamachines.github.io/dada-tbd-firmware/webui-updates/webui-update-v0.3.4.zip>`__
   * - **0.3.3**
     - 2026-03-12
     - New full rompler macro definition and preset. Designer fixes.
       All macro definitions and sound presets included.
     - `zip <https://dadamachines.github.io/dada-tbd-firmware/webui-updates/webui-update-v0.3.3.zip>`__
   * - **0.3.2**
     - 2026-03-12
     - Factory preset protection (lock icons, readonly inputs, hidden
       Save/Delete for factory presets). Preset value safety (trim to param
       count, Math.round for firmware compliance). Auto-UI type selection
       when adding CC mappings.
     - `zip <https://dadamachines.github.io/dada-tbd-firmware/webui-updates/webui-update-v0.3.2.zip>`__
   * - **0.3.1**
     - 2026-03-12
     - Online update workflow test. Version bump only — no functional changes.
     - `zip <https://dadamachines.github.io/dada-tbd-firmware/webui-updates/webui-update-v0.3.1.zip>`__
   * - **0.3.0**
     - 2026-03-12
     - Pre-release with WebUI update workflow. Full UI refresh including
       plugin manager, sample manager, macro/preset designer, and updater page.
     - `zip <https://dadamachines.github.io/dada-tbd-firmware/webui-updates/webui-update-v0.3.0.zip>`__
