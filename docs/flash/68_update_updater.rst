:orphan:

****************************
Update WebUI Updater
****************************

The **WebUI Updater** (``webui-update.html``) is the page on your TBD-16
that installs WebUI update packages. The updater checks for new versions
online and lets you install them with one click — all from your device.

How to Update the WebUI
=======================

1. Connect your computer to the TBD-16's WiFi network (or USB NCM).
2. Open ``http://192.168.4.1/webui-update.html`` in your browser.
3. The updater page auto-checks for new versions.
   If an update is available, click **Install Update**.
4. Wait for all files to upload (about 30 seconds over WiFi).
5. Hard-refresh the main page (``Cmd+Shift+R`` / ``Ctrl+Shift+R``) to see changes.

That's it — no firmware flash, no SD card erase, no MSC mode needed.

.. tip::
   The current WebUI version is shown in the footer of both the main page
   and the macro/preset manager.

Manual Update (Offline)
=======================

If your device doesn't have internet access (no WiFi AP bridge), you can
update manually:

1. Download the latest ``.zip`` package from `WebUI Versions <70_webui_versions.html>`_.
2. Connect to the TBD-16 and open ``http://192.168.4.1/webui-update.html``.
3. Drag and drop the ``.zip`` onto the updater page (or use the manual upload button).
4. Click **Apply Update** and wait for all files to upload.
5. Hard-refresh the main page (``Cmd+Shift+R`` / ``Ctrl+Shift+R``).

What's in the Latest Updater
=============================

The WebUI Updater (v0.3.4+) includes:

- **Light theme** matching the rest of the TBD-16 WebUI
- **Reinstall current version** — reset your WebUI files to a known-good state
  even when there is no newer update available
- **Install older versions** — roll back to any previously released version
  from an expandable version history
- **Improved layout** — cleaner header, version cards, and status messages
- **Manual ZIP upload** — drag and drop any ``.zip`` update package for
  offline installs
