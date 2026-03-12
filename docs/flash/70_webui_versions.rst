********************
WebUI Versions
********************

The TBD-16 WebUI can be updated independently of the firmware using the
built-in updater at ``http://192.168.4.1/webui-update.html``.

Each version below includes the update package and a summary of changes.

.. list-table:: WebUI Version History
   :header-rows: 1
   :widths: 15 15 70

   * - Version
     - Date
     - Description
   * - **0.3.2**
     - 2026-03-12
     - Factory preset protection (lock icons, readonly inputs, hidden
       Save/Delete for factory presets). Preset value safety (trim to param
       count, Math.round for firmware compliance). Auto-UI type selection
       when adding CC mappings. All macro definitions & sound presets included.
   * - **0.3.1**
     - 2026-03-12
     - Online update workflow test. Version bump only — no functional changes.
   * - **0.3.0**
     - 2026-03-12
     - Pre-release with WebUI update workflow. Full UI refresh including
       plugin manager, sample manager, macro/preset designer, and updater page.

How to Update
=============

1. Open ``http://192.168.4.1/webui-update.html`` in your browser
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

Downloads
=========

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Version
     - Download
   * - v0.3.2
     - `webui-update-v0.3.2.zip <../_static/updates/webui-update-v0.3.2.zip>`_
   * - v0.3.1
     - `webui-update-v0.3.1.zip <../_static/updates/webui-update-v0.3.1.zip>`_
   * - v0.3.0
     - `webui-update-v0.3.0.zip <../_static/updates/webui-update-v0.3.0.zip>`_
