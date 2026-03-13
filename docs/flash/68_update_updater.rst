:orphan:

****************************
Update WebUI Updater
****************************

The **WebUI Updater** (``webui-update.html``) is the page on your TBD-16
that installs WebUI update packages. Since it can't update itself while
running, this page pushes the latest version directly to your device.

**No firmware flash, no SD card erase, no MSC mode** — just connect to
your device's WiFi and click a button.

.. dropdown:: How it Works

   1. Your browser downloads the latest ``webui-update.html.gz`` from this page
   2. It sends the file to your TBD-16 via the REST API
      (``POST /api/v2/samples?action=uploadwww``)
   3. The device writes it to the SD card, replacing the old version
   4. Hard-refresh the updater page to use the new version

   **Requirements:**

   - Your computer must be connected to the TBD-16's WiFi (``http://192.168.4.1``)
   - Any modern browser (Chrome, Firefox, Edge, Safari)
   - The device must be running normal firmware (not MSC mode)

What's New in the Latest Updater
================================

The updated WebUI Updater includes:

- **Light theme** matching the rest of the TBD-16 WebUI
- **Reinstall current version** — reset your WebUI files to a known-good state
  even when there is no newer update available
- **Install older versions** — roll back to any previously released version
  from an expandable version history
- **Improved layout** — cleaner header, version cards, and status messages

.. raw:: html

    <style>
      .updater-tool {
        max-width: 560px;
        margin: 1em 0 1.5em;
      }
      .updater-tool .ut-card {
        border: 1px solid var(--color-background-border, #d1d5db);
        border-radius: 8px;
        padding: 1.2em 1.5em;
        margin-bottom: 1em;
        background: var(--color-background-secondary, #fafafa);
      }
      .updater-tool .ut-hdr {
        font-weight: 700;
        font-size: 1em;
        margin-bottom: 0.6em;
        color: var(--color-foreground-primary, #1a1a1a);
      }
      .updater-tool .ut-desc {
        font-size: 0.88em;
        color: var(--color-foreground-secondary, #555);
        line-height: 1.55;
        margin-bottom: 0.8em;
      }
      .updater-tool button {
        padding: 0.55em 1.4em;
        border: none;
        border-radius: 5px;
        font-size: 0.88em;
        font-weight: 600;
        cursor: pointer;
        color: #fff;
        transition: opacity 0.15s;
      }
      .updater-tool button:disabled {
        opacity: 0.35;
        cursor: not-allowed;
      }
      .updater-tool button:not(:disabled):hover {
        opacity: 0.85;
      }
      .updater-tool .btn-primary { background: #2563EB; }
      .updater-tool .btn-success { background: #16A34A; }
      .updater-tool .progress-wrap {
        margin-top: 0.7em;
        background: #E5E7EB;
        border-radius: 4px;
        overflow: hidden;
        height: 20px;
        position: relative;
        display: none;
      }
      .updater-tool .progress-bar {
        height: 100%;
        background: #2563EB;
        width: 0%;
        transition: width 0.2s;
      }
      .updater-tool .progress-text {
        position: absolute;
        top: 0; left: 0; right: 0;
        text-align: center;
        line-height: 20px;
        font-size: 0.78em;
        font-weight: 600;
        color: #374151;
      }
      .updater-tool .status {
        padding: 0.55em 0.85em;
        border-radius: 4px;
        font-size: 0.85em;
        margin-top: 0.7em;
        word-break: break-word;
        background: #F3F4F6;
        color: #374151;
      }
      .updater-tool .status-ok {
        background: #DEF7EC; color: #065F46;
      }
      .updater-tool .status-err {
        background: #FEE2E2; color: #991B1B;
      }
      .updater-tool .status-info {
        background: #EFF6FF; color: #1E40AF;
      }
      .updater-tool .ip-input {
        padding: 0.4em 0.6em;
        border: 1px solid var(--color-background-border, #d1d5db);
        border-radius: 4px;
        font-size: 0.88em;
        width: 160px;
        margin-right: 0.5em;
        background: var(--color-background-primary, #fff);
        color: var(--color-foreground-primary, #1a1a1a);
      }

      /* Dark mode */
      body[data-theme="dark"] .updater-tool .ut-card {
        background: #1e293b;
        border-color: #334155;
      }
      body[data-theme="dark"] .updater-tool .ut-hdr {
        color: #f1f5f9;
      }
      body[data-theme="dark"] .updater-tool .ut-desc {
        color: #94a3b8;
      }
      body[data-theme="dark"] .updater-tool .ip-input {
        background: #1e293b;
        color: #f1f5f9;
        border-color: #475569;
      }
      body[data-theme="dark"] .updater-tool .status {
        background: #1e293b; color: #cbd5e1;
      }
      body[data-theme="dark"] .updater-tool .status-ok {
        background: #064e3b; color: #6ee7b7;
      }
      body[data-theme="dark"] .updater-tool .status-err {
        background: #450a0a; color: #fca5a5;
      }
      body[data-theme="dark"] .updater-tool .status-info {
        background: #1e3a5f; color: #93c5fd;
      }
      @media (prefers-color-scheme: dark) {
        body:not([data-theme="light"]) .updater-tool .ut-card {
          background: #1e293b; border-color: #334155;
        }
        body:not([data-theme="light"]) .updater-tool .ut-hdr { color: #f1f5f9; }
        body:not([data-theme="light"]) .updater-tool .ut-desc { color: #94a3b8; }
        body:not([data-theme="light"]) .updater-tool .ip-input {
          background: #1e293b; color: #f1f5f9; border-color: #475569;
        }
        body:not([data-theme="light"]) .updater-tool .status {
          background: #1e293b; color: #cbd5e1;
        }
        body:not([data-theme="light"]) .updater-tool .status-ok {
          background: #064e3b; color: #6ee7b7;
        }
        body:not([data-theme="light"]) .updater-tool .status-err {
          background: #450a0a; color: #fca5a5;
        }
        body:not([data-theme="light"]) .updater-tool .status-info {
          background: #1e3a5f; color: #93c5fd;
        }
      }
    </style>

    <div class="updater-tool" id="updaterTool">

      <!-- ── Device Connection ── -->
      <div class="ut-card">
        <div class="ut-hdr">1. Connect to Your Device</div>
        <div class="ut-desc">
          Make sure your computer is connected to your TBD-16's WiFi network,
          then verify the connection.
        </div>
        <div style="display:flex; align-items:center; flex-wrap:wrap; gap:0.5em;">
          <label style="font-size:0.85em; font-weight:600;">Device IP:</label>
          <input type="text" id="utDeviceIp" class="ip-input" value="192.168.4.1"
                 placeholder="192.168.4.1">
          <button id="utCheckBtn" class="btn-primary">Check Connection</button>
        </div>
        <div class="status status-info" id="utConnStatus">
          Enter your device IP and click <b>Check Connection</b>.
        </div>
      </div>

      <!-- ── Push Update ── -->
      <div class="ut-card">
        <div class="ut-hdr">2. Update the WebUI Updater</div>
        <div class="ut-desc">
          Downloads the latest <code>webui-update.html.gz</code> from this page
          and pushes it to your device's SD card via the REST API. The new updater
          includes a light theme, reinstall support, and version history.
        </div>
        <button id="utPushBtn" class="btn-success" disabled>Push Update to Device</button>
        <div class="progress-wrap" id="utProg">
          <div class="progress-bar" id="utProgBar"></div>
          <span class="progress-text" id="utProgTxt">0 %</span>
        </div>
        <div class="status" id="utPushStatus">Check the connection first.</div>
      </div>

    </div>

    <script>
    (function () {
      var $ = function (id) { return document.getElementById(id); };

      var deviceIp   = $('utDeviceIp');
      var checkBtn   = $('utCheckBtn');
      var connStatus = $('utConnStatus');
      var pushBtn    = $('utPushBtn');
      var pushStatus = $('utPushStatus');
      var prog       = $('utProg');
      var progBar    = $('utProgBar');
      var progTxt    = $('utProgTxt');

      /* Hosted updater file — relative to this docs page */
      var UPDATER_GZ_URL = '../_static/updater/webui-update.html.gz';

      function setStat(el, msg, cls) {
        el.innerHTML = msg;
        el.className = 'status' + (cls ? ' status-' + cls : '');
      }

      function baseUrl() {
        var ip = deviceIp.value.trim();
        if (!ip) ip = '192.168.4.1';
        return 'http://' + ip;
      }

      /* ── Step 1: Check connection ── */
      checkBtn.addEventListener('click', async function () {
        checkBtn.disabled = true;
        setStat(connStatus, 'Connecting to <b>' + baseUrl() + '</b>…', 'info');

        try {
          var resp = await fetch(baseUrl() + '/api/v2/device?action=getIOCaps', {
            mode: 'cors',
            signal: AbortSignal.timeout(5000)
          });
          if (!resp.ok) throw new Error('HTTP ' + resp.status);
          var data = await resp.json();

          var info = 'TBD-16';
          if (data && data.fw_version) {
            info += ' — firmware ' + data.fw_version;
          }

          setStat(connStatus, '✓ Connected to <b>' + info + '</b>', 'ok');
          pushBtn.disabled = false;
          setStat(pushStatus, 'Ready. Click <b>Push Update to Device</b> to proceed.');
        } catch (e) {
          setStat(connStatus,
            'Cannot reach <b>' + baseUrl() + '</b>. ' +
            'Make sure you are connected to the TBD-16 WiFi network.<br>' +
            '<small>Error: ' + e.message + '</small>', 'err');
          pushBtn.disabled = true;
        }
        checkBtn.disabled = false;
      });

      /* ── Step 2: Download .gz and push to device ── */
      pushBtn.addEventListener('click', async function () {
        pushBtn.disabled = true;
        checkBtn.disabled = true;
        prog.style.display = 'block';
        progBar.style.width = '0%';
        progTxt.textContent = '0 %';

        try {
          setStat(pushStatus, 'Downloading latest webui-update.html.gz…', 'info');
          progBar.style.width = '10%';
          progTxt.textContent = '10 %';

          var dlResp = await fetch(UPDATER_GZ_URL);
          if (!dlResp.ok) throw new Error('Download failed: HTTP ' + dlResp.status);
          var gzData = await dlResp.arrayBuffer();
          var sizeKB = (gzData.byteLength / 1024).toFixed(1);

          progBar.style.width = '40%';
          progTxt.textContent = '40 %';
          setStat(pushStatus, 'Downloaded ' + sizeKB + ' KB. Uploading to device…', 'info');

          var uploadUrl = baseUrl() +
            '/api/v2/samples?action=uploadwww&path=' +
            encodeURIComponent('webui-update.html.gz');

          var upResp = await fetch(uploadUrl, {
            method: 'POST',
            headers: { 'Content-Type': 'application/octet-stream' },
            body: gzData,
            mode: 'cors'
          });

          progBar.style.width = '90%';
          progTxt.textContent = '90 %';

          if (!upResp.ok) {
            var errText = await upResp.text().catch(function () { return upResp.statusText; });
            throw new Error('Device rejected upload: HTTP ' + upResp.status + ' — ' + errText);
          }

          var result = await upResp.json();
          if (!result.ok) throw new Error(result.error || 'Upload rejected by device');

          progBar.style.width = '100%';
          progTxt.textContent = '100 %';

          setStat(pushStatus,
            '✓ <b>WebUI Updater updated successfully!</b><br>' +
            'Hard-refresh the updater page on your device ' +
            '(<code>Cmd+Shift+R</code> / <code>Ctrl+Shift+R</code>) to use the new version.<br>' +
            '<a href="http://' + deviceIp.value.trim() + '/webui-update.html" ' +
            'target="_blank" style="color:inherit; font-weight:600;">Open Updater →</a>',
            'ok');

        } catch (e) {
          setStat(pushStatus,
            'Failed: ' + e.message + '<br>' +
            '<small>Make sure you are connected to the TBD-16 WiFi and the device is in normal mode (not MSC).</small>',
            'err');
        }

        pushBtn.disabled = false;
        checkBtn.disabled = false;
      });
    })();
    </script>

.. tip::
   After updating, hard-refresh the updater page on your device
   (``Cmd+Shift+R`` / ``Ctrl+Shift+R``) to load the new version.
   The updater now has a light theme matching the rest of the WebUI.

Why Can't the Updater Update Itself?
====================================

The WebUI Updater runs inside your browser — loaded from the device's SD card.
While it's running, overwriting its own file risks a broken state with no
recovery path. This external page writes the file while it's not in use.

After the Updater is Updated
=============================

Once you have the latest updater on your device, you can use these features
directly from ``http://192.168.4.1/webui-update.html``:

- **Online updates** — the updater auto-checks for new WebUI versions and
  offers one-click install
- **Reinstall current version** — if your WebUI files get corrupted or you
  want a factory reset of the latest version, click the **Reinstall** button
  (available even when the installed version matches the latest)
- **Install older versions** — expand the version history to roll back to
  any previously released version
- **Manual ZIP upload** — drag and drop any ``.zip`` update package for
  offline installs
