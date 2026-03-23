:orphan:

****************************
App Manager
****************************

Manage **RP2350 apps** on your TBD-16 directly from the browser.
Install apps to the Pico SD card, choose a boot mode, and flash
firmware — no card reader or terminal commands needed.

Browse the `App Catalog <https://dadamachines.github.io/dada-tbd-firmware/app-catalog.json>`_
for all available apps.

.. dropdown:: How it Works

   **Step 1 — Mount SD Card:**
   Flash the USB Mass Storage firmware to the RP2350 so the Pico
   SD card appears as a removable drive.

   **Step 2 — Manage Apps:**
   Open the mounted SD card, browse the app catalog, and install
   or remove apps. Apps are stored in the ``tbd-apps/`` folder.

   **Step 3 — Choose Boot Mode:**
   Eject the SD card, put the RP2350 back in BOOTSEL mode, and
   flash the firmware you want:

   - **Bootloader** — Boot menu that lets you switch between
     installed apps using the front-panel controls.
   - **Groovebox Only** — Flash the groovebox directly for
     single-app operation (no boot menu).
   - **Flash Nuke** — Erase the RP2350 flash completely.
     Use this to factory-reset or recover from a broken state.

   **Hardware setup:**

   - **Back USB-C Port #2** (closest to the edge) → RP2350 BOOTSEL + SD card
   - **Back USB-C Port #1** → power (keep connected)

   **Browser:** Chrome, Edge or Opera required (WebUSB + File System Access).

   **Time:** 5–10 minutes

.. raw:: html

    <style>
      /* ── App Manager Styles ── */
      .app-mgr {
        max-width: 680px;
        margin: 1em 0 1.5em;
      }
      .app-mgr .step-card {
        border: 1px solid var(--color-background-border, #d1d5db);
        border-radius: 8px;
        padding: 1.2em 1.5em;
        margin-bottom: 1em;
        background: var(--color-background-secondary, #fafafa);
      }
      .app-mgr .step-card.active-step {
        border-color: #2563EB;
        box-shadow: 0 0 0 1px #2563EB;
      }
      .app-mgr .step-num {
        display: inline-flex;
        align-items: center;
        justify-content: center;
        width: 26px; height: 26px;
        border-radius: 50%;
        background: #2563EB;
        color: #fff;
        font-weight: 700;
        font-size: 0.85em;
        margin-right: 0.5em;
        flex-shrink: 0;
      }
      .app-mgr .step-card.done .step-num {
        background: #10b981;
      }
      .app-mgr .step-hdr {
        display: flex;
        align-items: center;
        font-weight: 700;
        font-size: 1em;
        margin-bottom: 0.6em;
        color: var(--color-foreground-primary, #1a1a1a);
      }
      .app-mgr .step-desc {
        font-size: 0.88em;
        color: var(--color-foreground-secondary, #555);
        line-height: 1.55;
        margin-bottom: 0.8em;
      }
      .app-mgr .btn-row {
        display: flex;
        gap: 0.5em;
        flex-wrap: wrap;
      }
      .app-mgr button {
        padding: 0.5em 1.2em;
        border: none;
        border-radius: 5px;
        font-size: 0.88em;
        font-weight: 600;
        cursor: pointer;
        color: #fff;
        transition: opacity 0.15s;
      }
      .app-mgr button:disabled {
        opacity: 0.35;
        cursor: not-allowed;
      }
      .app-mgr button:not(:disabled):hover {
        opacity: 0.85;
      }
      .app-mgr .btn-primary   { background: #2563EB; }
      .app-mgr .btn-success   { background: #16A34A; }
      .app-mgr .btn-secondary { background: #6B7280; }
      .app-mgr .btn-danger    { background: #DC2626; }
      .app-mgr .btn-cyan      { background: #0891B2; }
      .app-mgr .btn-sm {
        padding: 0.35em 0.8em;
        font-size: 0.82em;
      }
      .app-mgr .progress-wrap {
        margin-top: 0.7em;
        background: #E5E7EB;
        border-radius: 4px;
        overflow: hidden;
        height: 20px;
        position: relative;
        display: none;
      }
      .app-mgr .progress-bar {
        height: 100%;
        background: #2563EB;
        width: 0%;
        transition: width 0.15s;
      }
      .app-mgr .progress-text {
        position: absolute;
        top: 0; left: 0; right: 0;
        text-align: center;
        line-height: 20px;
        font-size: 0.78em;
        font-weight: 600;
        color: #374151;
      }
      .app-mgr .status {
        padding: 0.55em 0.85em;
        border-radius: 4px;
        font-size: 0.85em;
        margin-top: 0.7em;
        word-break: break-word;
        background: #F3F4F6;
        color: #374151;
      }
      .app-mgr .status-ok  { background: #DEF7EC; color: #065F46; }
      .app-mgr .status-err { background: #FEE2E2; color: #991B1B; }
      .app-mgr .status-info { background: #EFF6FF; color: #1E40AF; }

      /* ── App catalog table ── */
      .app-mgr .app-table {
        width: 100%;
        border-collapse: collapse;
        font-size: 0.85em;
        margin-top: 0.8em;
      }
      .app-mgr .app-table th {
        text-align: left;
        padding: 0.5em 0.6em;
        border-bottom: 2px solid #d1d5db;
        font-weight: 700;
        font-size: 0.9em;
      }
      .app-mgr .app-table td {
        padding: 0.5em 0.6em;
        border-bottom: 1px solid #e5e7eb;
        vertical-align: middle;
      }
      .app-mgr .app-table tr:last-child td {
        border-bottom: none;
      }
      .app-mgr .badge {
        display: inline-block;
        padding: 0.15em 0.5em;
        border-radius: 3px;
        font-size: 0.78em;
        font-weight: 600;
        text-transform: uppercase;
        letter-spacing: 0.03em;
      }
      .app-mgr .badge-official  { background: #DBEAFE; color: #1E40AF; }
      .app-mgr .badge-system    { background: #E5E7EB; color: #374151; }
      .app-mgr .badge-partner   { background: #FEF3C7; color: #92400E; }
      .app-mgr .badge-community { background: #D1FAE5; color: #065F46; }
      .app-mgr .badge-installed { background: #D1FAE5; color: #065F46; }
      .app-mgr .badge-missing   { background: #FEE2E2; color: #991B1B; }
      .app-mgr .app-desc {
        font-size: 0.88em;
        color: var(--color-foreground-secondary, #666);
      }

      /* ── Boot mode selector ── */
      .app-mgr .mode-option {
        border: 2px solid #d1d5db;
        border-radius: 8px;
        padding: 0.8em 1em;
        margin-bottom: 0.6em;
        cursor: pointer;
        transition: border-color 0.15s, box-shadow 0.15s;
      }
      .app-mgr .mode-option:hover {
        border-color: #93c5fd;
      }
      .app-mgr .mode-option.selected {
        border-color: #2563EB;
        box-shadow: 0 0 0 1px #2563EB;
      }
      .app-mgr .mode-option h4 {
        margin: 0 0 0.3em;
        font-size: 0.95em;
      }
      .app-mgr .mode-option p {
        margin: 0;
        font-size: 0.82em;
        color: var(--color-foreground-secondary, #555);
        line-height: 1.45;
      }

      .app-mgr .file-log {
        margin-top: 0.5em;
        max-height: 140px;
        overflow-y: auto;
        font-size: 0.78em;
        font-family: monospace;
        background: #f9fafb;
        border: 1px solid #e5e7eb;
        border-radius: 4px;
        padding: 0.4em 0.6em;
        color: #374151;
        display: none;
      }

      .app-mgr .complete-card {
        border: 2px solid #10b981;
        border-radius: 8px;
        padding: 1.5em;
        background: #f0fdf4;
        text-align: center;
        display: none;
      }
      .app-mgr .complete-card h3 {
        color: #065f46;
        margin: 0 0 0.5em;
        font-size: 1.2em;
      }
      .app-mgr .complete-card p {
        color: #047857;
        font-size: 0.92em;
        margin: 0;
      }

      /* ── Dark mode ── */
      body[data-theme="dark"] .app-mgr .step-card {
        background: #1e293b; border-color: #334155;
      }
      body[data-theme="dark"] .app-mgr .step-hdr { color: #f1f5f9; }
      body[data-theme="dark"] .app-mgr .step-desc { color: #94a3b8; }
      body[data-theme="dark"] .app-mgr .status {
        background: #1e293b; color: #cbd5e1;
      }
      body[data-theme="dark"] .app-mgr .status-ok {
        background: #064e3b; color: #6ee7b7;
      }
      body[data-theme="dark"] .app-mgr .status-err {
        background: #450a0a; color: #fca5a5;
      }
      body[data-theme="dark"] .app-mgr .status-info {
        background: #1e3a5f; color: #93c5fd;
      }
      body[data-theme="dark"] .app-mgr .file-log {
        background: #0f172a; border-color: #334155; color: #cbd5e1;
      }
      body[data-theme="dark"] .app-mgr .app-table th {
        border-color: #475569;
      }
      body[data-theme="dark"] .app-mgr .app-table td {
        border-color: #334155;
      }
      body[data-theme="dark"] .app-mgr .app-desc { color: #94a3b8; }
      body[data-theme="dark"] .app-mgr .mode-option {
        border-color: #475569; background: #1e293b;
      }
      body[data-theme="dark"] .app-mgr .mode-option:hover {
        border-color: #60a5fa;
      }
      body[data-theme="dark"] .app-mgr .mode-option.selected {
        border-color: #3b82f6; box-shadow: 0 0 0 1px #3b82f6;
      }
      body[data-theme="dark"] .app-mgr .mode-option p { color: #94a3b8; }
      body[data-theme="dark"] .app-mgr .complete-card {
        background: #064e3b; border-color: #10b981;
      }
      body[data-theme="dark"] .app-mgr .complete-card h3 { color: #6ee7b7; }
      body[data-theme="dark"] .app-mgr .complete-card p { color: #a7f3d0; }
      @media (prefers-color-scheme: dark) {
        body:not([data-theme="light"]) .app-mgr .step-card {
          background: #1e293b; border-color: #334155;
        }
        body:not([data-theme="light"]) .app-mgr .step-hdr { color: #f1f5f9; }
        body:not([data-theme="light"]) .app-mgr .step-desc { color: #94a3b8; }
        body:not([data-theme="light"]) .app-mgr .status {
          background: #1e293b; color: #cbd5e1;
        }
        body:not([data-theme="light"]) .app-mgr .status-ok {
          background: #064e3b; color: #6ee7b7;
        }
        body:not([data-theme="light"]) .app-mgr .status-err {
          background: #450a0a; color: #fca5a5;
        }
        body:not([data-theme="light"]) .app-mgr .status-info {
          background: #1e3a5f; color: #93c5fd;
        }
        body:not([data-theme="light"]) .app-mgr .file-log {
          background: #0f172a; border-color: #334155; color: #cbd5e1;
        }
        body:not([data-theme="light"]) .app-mgr .app-table th { border-color: #475569; }
        body:not([data-theme="light"]) .app-mgr .app-table td { border-color: #334155; }
        body:not([data-theme="light"]) .app-mgr .app-desc { color: #94a3b8; }
        body:not([data-theme="light"]) .app-mgr .mode-option {
          border-color: #475569; background: #1e293b;
        }
        body:not([data-theme="light"]) .app-mgr .mode-option.selected {
          border-color: #3b82f6; box-shadow: 0 0 0 1px #3b82f6;
        }
        body:not([data-theme="light"]) .app-mgr .mode-option p { color: #94a3b8; }
        body:not([data-theme="light"]) .app-mgr .complete-card {
          background: #064e3b; border-color: #10b981;
        }
        body:not([data-theme="light"]) .app-mgr .complete-card h3 { color: #6ee7b7; }
        body:not([data-theme="light"]) .app-mgr .complete-card p { color: #a7f3d0; }
      }
    </style>

    <div class="app-mgr" id="appManager">

      <!-- ════════ STEP 1 — Flash MSC & Mount SD Card ════════ -->
      <div class="step-card active-step" id="card1">
        <div class="step-hdr">
          <span class="step-num">1</span> Flash USB Mass Storage &amp; Mount SD Card
        </div>
        <div class="step-desc">
          Connect <b>back USB-C Port #2</b> (closest to the edge) and keep <b>Port #1</b>
          connected for power.<br>
          Put the RP2350 in <b>BOOTSEL mode</b> (hold BOOTSEL + press RESET on the front panel),
          then click <b>Connect</b>.
        </div>
        <div class="btn-row">
          <button id="btn1Connect" class="btn-primary" disabled>Loading…</button>
          <button id="btn1Flash" class="btn-success" disabled>Flash MSC Firmware</button>
        </div>
        <div class="progress-wrap" id="prog1">
          <div class="progress-bar" id="prog1Bar"></div>
          <span class="progress-text" id="prog1Txt">0 %</span>
        </div>
        <div class="status status-info" id="stat1">Loading flash tools and app catalog…</div>
      </div>

      <!-- ════════ STEP 2 — Manage Apps on SD Card ════════ -->
      <div class="step-card" id="card2">
        <div class="step-hdr">
          <span class="step-num">2</span> Manage Apps on SD Card
        </div>
        <div class="step-desc">
          The Pico SD card should now appear as a removable drive (<b>"NO NAME"</b>)
          on <b>back Port #2</b>.<br>
          Click <b>Open SD Card</b> to select the mounted drive, then install or remove apps.
        </div>
        <div class="btn-row">
          <button id="btn2Open" class="btn-primary">Open SD Card Folder</button>
          <button id="btn2Refresh" class="btn-secondary" disabled>Refresh</button>
        </div>
        <div class="status" id="stat2">Select the mounted SD card drive to scan for installed apps.</div>

        <!-- App catalog table (populated by JS) -->
        <div id="appListWrap" style="display:none;">
          <table class="app-table">
            <thead>
              <tr>
                <th>App</th>
                <th>Status</th>
                <th>Action</th>
              </tr>
            </thead>
            <tbody id="appListBody"></tbody>
          </table>
          <div class="status status-info" id="statApps" style="margin-top:0.6em;"></div>
        </div>

        <div class="progress-wrap" id="prog2">
          <div class="progress-bar" id="prog2Bar"></div>
          <span class="progress-text" id="prog2Txt">0 %</span>
        </div>
        <div class="file-log" id="fileLog"></div>
      </div>

      <!-- ════════ STEP 3 — Choose Boot Mode & Flash ════════ -->
      <div class="step-card" id="card3">
        <div class="step-hdr">
          <span class="step-num">3</span> Choose Boot Mode &amp; Flash
        </div>
        <div class="step-desc">
          <b>Safely eject the SD card drive</b> from your computer first.<br>
          Then put the RP2350 in <b>BOOTSEL mode</b> again (hold BOOTSEL + press RESET).
        </div>

        <div id="modeSelector">
          <div class="mode-option selected" id="modeBootloader" onclick="selectMode('bootloader')">
            <h4>🔀 Boot Menu (Bootloader)</h4>
            <p>Flash the custom bootloader. On startup, use the front-panel controls
               to choose which app to run from the SD card. <b>Recommended</b> when you
               have multiple apps installed.</p>
          </div>
          <div class="mode-option" id="modeGroovebox" onclick="selectMode('groovebox')">
            <h4>🎹 Groovebox Only</h4>
            <p>Flash the groovebox directly — single-app mode with no boot menu.
               The device boots straight into the sequencer.</p>
          </div>
          <div class="mode-option" id="modeNuke" onclick="selectMode('nuke')">
            <h4>💥 Factory Reset (Flash Nuke)</h4>
            <p>Erase the RP2350 flash completely. Use this to recover from a broken
               state or remove the bootloader. After nuking, flash the groovebox
               or bootloader to restore operation.</p>
          </div>
        </div>

        <div class="btn-row" style="margin-top:0.8em;">
          <button id="btn3Connect" class="btn-primary">Connect</button>
          <button id="btn3Flash" class="btn-success" disabled>Flash Selected Firmware</button>
          <button id="btn3Reboot" class="btn-secondary" disabled>Reboot</button>
        </div>
        <div class="progress-wrap" id="prog3">
          <div class="progress-bar" id="prog3Bar"></div>
          <span class="progress-text" id="prog3Txt">0 %</span>
        </div>
        <div class="status" id="stat3">Eject the SD card, enter <b>BOOTSEL mode</b>, then click <b>Connect</b>.</div>
      </div>

      <!-- ════════ DONE ════════ -->
      <div class="complete-card" id="cardDone">
        <h3>✓ App Manager Complete</h3>
        <p id="doneMsg">Your TBD-16 RP2350 apps are set up.<br>
        Disconnect USB cables, wait 3 seconds, then reconnect via <b>back Port #1</b>.</p>
      </div>

    </div>

    <script type="module">
    /* ══════════════════════════════════════════════════════════
       App Manager — RP2350 app install/remove + boot mode flash
       Uses tbd-flasher-rp2350.js (Picoboot WebUSB)
       ══════════════════════════════════════════════════════════ */

    import {
      loadPicoboot, connectRP2350, flashRP2350,
      rebootRP2350, disconnectRP2350
    } from '../_static/js/tbd-flasher-rp2350.js';

    /* ── DOM refs ── */
    var $ = function (id) { return document.getElementById(id); };

    /* Step 1 */
    var btn1Connect = $('btn1Connect'), btn1Flash = $('btn1Flash');
    var prog1 = $('prog1'), prog1Bar = $('prog1Bar'), prog1Txt = $('prog1Txt');
    var stat1 = $('stat1');

    /* Step 2 */
    var btn2Open = $('btn2Open'), btn2Refresh = $('btn2Refresh');
    var stat2 = $('stat2'), statApps = $('statApps');
    var appListWrap = $('appListWrap'), appListBody = $('appListBody');
    var prog2 = $('prog2'), prog2Bar = $('prog2Bar'), prog2Txt = $('prog2Txt');
    var fileLog = $('fileLog');

    /* Step 3 */
    var btn3Connect = $('btn3Connect'), btn3Flash = $('btn3Flash'), btn3Reboot = $('btn3Reboot');
    var prog3 = $('prog3'), prog3Bar = $('prog3Bar'), prog3Txt = $('prog3Txt');
    var stat3 = $('stat3');

    var cardDone = $('cardDone'), doneMsg = $('doneMsg');

    /* ── Helpers ── */
    function setStat(el, msg, cls) {
      el.innerHTML = msg;
      el.className = 'status' + (cls ? ' status-' + cls : '');
    }
    function showProg(wrap, bar, txt, pct) {
      wrap.style.display = 'block';
      bar.style.width = pct + '%';
      txt.textContent = pct + ' %';
    }
    function hideProg(wrap) { wrap.style.display = 'none'; }
    function log(msg) {
      fileLog.style.display = 'block';
      fileLog.textContent += msg + '\n';
      fileLog.scrollTop = fileLog.scrollHeight;
    }

    /* ── State ── */
    var FIRMWARE_CDN = 'https://dadamachines.github.io/dada-tbd-firmware';
    var CATALOG = null;        /* app-catalog.json */
    var SD_APPS = [];          /* list of installed .uf2 filenames on SD card */
    var DIR_HANDLE = null;     /* File System Access directory handle */
    var SELECTED_MODE = 'bootloader';
    var BOOT2350_CDN = FIRMWARE_CDN + '/apps/bootloader/sd-card/BOOT2350.uf2';

    /* IDs of apps that are Pico firmware (flashed directly, not put on SD card) */
    var PICO_FIRMWARE_IDS = ['bootloader', 'flash-nuke'];

    /* ── Catalog helpers ── */

    /** Get apps that can be installed to tbd-apps/ on the SD card. */
    function getInstallableApps() {
      if (!CATALOG) return [];
      return CATALOG.apps.filter(function (a) {
        return PICO_FIRMWARE_IDS.indexOf(a.id) === -1;
      });
    }

    /** Get the latest release for an app (prefers flash target for Picoboot). */
    function latestRelease(app) {
      if (!app.releases || app.releases.length === 0) return null;
      return app.releases[0];
    }

    /** Get the SD card release (prefers target:"ram" for bootloader loading). */
    function sdRelease(app) {
      if (!app.releases || app.releases.length === 0) return null;
      var ram = app.releases.find(function (r) { return r.target === 'ram'; });
      return ram || app.releases[0];
    }

    /** Build a CDN URL from a release object. */
    function releaseCdnUrl(app, rel) {
      if (!rel) return null;
      if (rel.cdnPath) return FIRMWARE_CDN + '/' + rel.cdnPath;
      return FIRMWARE_CDN + '/apps/' + app.id + '/' + app.id + '-' + rel.version + '.uf2';
    }

    /** Get the CDN download URL for an app's latest release (Picoboot flash). */
    function appCdnUrl(app) {
      return releaseCdnUrl(app, latestRelease(app));
    }

    /** Get the CDN download URL for an app's SD card release (bootloader RAM). */
    function appSdUrl(app) {
      return releaseCdnUrl(app, sdRelease(app));
    }

    /** Get a catalog app by ID. */
    function getApp(id) {
      if (!CATALOG) return null;
      return CATALOG.apps.find(function (a) { return a.id === id; }) || null;
    }

    /* ══════════════════════════════
       INIT — load tools + fetch catalog
       ══════════════════════════════ */
    try {
      if (!('usb' in navigator)) {
        setStat(stat1, 'Your browser does not support <b>WebUSB</b>. Use Chrome, Edge, or Opera on desktop.', 'err');
        throw new Error('WebUSB not supported');
      }

      var [catalog, _pico] = await Promise.all([
        fetch(FIRMWARE_CDN + '/app-catalog.json').then(function (r) {
          if (!r.ok) throw new Error('Catalog fetch failed: ' + r.statusText);
          return r.json();
        }),
        loadPicoboot('../_static/picoflash').catch(function (e) {
          console.warn('Picoboot load failed:', e);
        })
      ]);

      CATALOG = catalog;
      var mscApp = getApp('tusb-msc-pico');
      if (!mscApp) {
        setStat(stat1, 'USB Mass Storage app not found in catalog.', 'err');
        throw new Error('No MSC app');
      }

      btn1Connect.textContent = 'Connect';
      btn1Connect.disabled = false;
      setStat(stat1, 'Catalog loaded: <b>' + CATALOG.apps.length + ' apps</b> available. ' +
        'Put the RP2350 in <b>BOOTSEL mode</b>, then click <b>Connect</b>.', 'info');

    } catch (e) {
      if (e.message !== 'WebUSB not supported' && e.message !== 'No MSC app') {
        setStat(stat1, 'Failed to load: ' + e.message, 'err');
      }
      throw e;
    }

    /* ══════════════════════════════
       STEP 1 — Flash MSC to Pico
       ══════════════════════════════ */
    var ctx1 = null;

    btn1Connect.addEventListener('click', async function () {
      try {
        btn1Connect.disabled = true;
        ctx1 = await connectRP2350({
          onStatus: function (msg) { setStat(stat1, msg); }
        });
        btn1Flash.disabled = false;
        setStat(stat1, 'Connected to <b>' + (ctx1.info.productName || 'RP2350') +
          '</b>. Click <b>Flash MSC Firmware</b>.', 'ok');
      } catch (e) {
        console.error(e);
        setStat(stat1, 'Connection failed: ' + e.message +
          '<br>Make sure the RP2350 is in <b>BOOTSEL mode</b>.', 'err');
        btn1Connect.disabled = false;
        await disconnectRP2350(ctx1); ctx1 = null;
      }
    });

    btn1Flash.addEventListener('click', async function () {
      if (!ctx1) return;
      btn1Flash.disabled = true; btn1Connect.disabled = true;
      try {
        var mscUrl = appCdnUrl(getApp('tusb-msc-pico'));
        await flashRP2350(ctx1, mscUrl, {
          onStatus: function (msg) { setStat(stat1, msg); },
          onProgress: function (pct) { showProg(prog1, prog1Bar, prog1Txt, pct); }
        });
        await rebootRP2350(ctx1);
        ctx1 = null;

        hideProg(prog1);
        setStat(stat1,
          '✓ MSC firmware flashed. The Pico SD card should appear as a drive ' +
          '(<b>"NO NAME"</b>) on <b>back Port #2</b>.<br>' +
          'If the drive does not appear: unplug the cable, wait 3 seconds, replug it.', 'ok');

        $('card1').classList.add('done');
        setStat(stat2, 'Click <b>Open SD Card Folder</b> to select the mounted drive.');
      } catch (e) {
        console.error(e);
        setStat(stat1, 'Flash failed: ' + e.message, 'err');
        btn1Connect.disabled = false;
        await disconnectRP2350(ctx1); ctx1 = null;
      }
    });

    /* ══════════════════════════════
       STEP 2 — Open SD Card & Manage Apps
       ══════════════════════════════ */

    /** Scan tbd-apps/ on the SD card and return list of .uf2 filenames. */
    async function scanInstalledApps(dirHandle) {
      var found = [];
      try {
        var appsDir = await dirHandle.getDirectoryHandle('tbd-apps', { create: false });
        for await (var entry of appsDir.entries()) {
          var name = entry[0];
          if (name.endsWith('.uf2')) found.push(name);
        }
      } catch (e) {
        /* tbd-apps/ doesn't exist yet — that's fine */
      }
      return found;
    }

    /** Build the app catalog table. */
    function renderAppTable(installedFiles) {
      var apps = getInstallableApps();
      appListBody.innerHTML = '';

      var installedCount = 0;
      var totalCount = apps.length;

      apps.forEach(function (app) {
        var rel = latestRelease(app);
        var sdName = app.sdFilename || (app.id + '.uf2');
        var isInstalled = installedFiles.indexOf(sdName) >= 0;
        if (isInstalled) installedCount++;

        var tr = document.createElement('tr');

        /* Name + description */
        var tdName = document.createElement('td');
        var nameHtml = '<b>' + app.name + '</b>';
        if (app.tier) {
          var tierClass = 'badge-' + app.tier;
          nameHtml += ' <span class="badge ' + tierClass + '">' + app.tier + '</span>';
        }
        nameHtml += '<br><span class="app-desc">' + (app.description || '') + '</span>';
        if (rel) nameHtml += '<br><span class="app-desc">v' + rel.version + '</span>';
        tdName.innerHTML = nameHtml;
        tr.appendChild(tdName);

        /* Status */
        var tdStatus = document.createElement('td');
        if (isInstalled) {
          tdStatus.innerHTML = '<span class="badge badge-installed">Installed</span>';
        } else {
          tdStatus.innerHTML = '<span class="badge badge-missing">Not installed</span>';
        }
        tr.appendChild(tdStatus);

        /* Action */
        var tdAction = document.createElement('td');
        if (app.alwaysInstalled) {
          if (isInstalled) {
            tdAction.innerHTML = '<span style="font-size:0.82em;color:#6B7280;">Required</span>';
          } else {
            var btnInstall = document.createElement('button');
            btnInstall.className = 'btn-success btn-sm';
            btnInstall.textContent = 'Install';
            btnInstall.onclick = (function (a) {
              return function () { installApp(a); };
            })(app);
            tdAction.appendChild(btnInstall);
          }
        } else if (isInstalled) {
          var btnRemove = document.createElement('button');
          btnRemove.className = 'btn-danger btn-sm';
          btnRemove.textContent = 'Remove';
          btnRemove.onclick = (function (a) {
            return function () { removeApp(a); };
          })(app);
          tdAction.appendChild(btnRemove);
        } else {
          var btnInstall2 = document.createElement('button');
          btnInstall2.className = 'btn-success btn-sm';
          btnInstall2.textContent = 'Install';
          btnInstall2.onclick = (function (a) {
            return function () { installApp(a); };
          })(app);
          tdAction.appendChild(btnInstall2);
        }
        tr.appendChild(tdAction);

        appListBody.appendChild(tr);
      });

      appListWrap.style.display = 'block';
      setStat(statApps, '<b>' + installedCount + '</b> of <b>' + totalCount +
        '</b> apps installed on SD card.', 'info');
    }

    /** Refresh the installed apps list. */
    async function refreshApps() {
      if (!DIR_HANDLE) return;
      var files = await scanInstalledApps(DIR_HANDLE);
      SD_APPS = files;
      renderAppTable(files);
    }

    btn2Open.addEventListener('click', async function () {
      try {
        DIR_HANDLE = await window.showDirectoryPicker({ mode: 'readwrite' });
      } catch (e) {
        if (e.name === 'AbortError') return;
        setStat(stat2, 'Could not access directory: ' + e.message, 'err');
        return;
      }

      btn2Refresh.disabled = false;
      setStat(stat2, 'Scanning SD card for installed apps…');

      var files = await scanInstalledApps(DIR_HANDLE);
      SD_APPS = files;
      renderAppTable(files);

      /* Ensure BOOT2350.uf2 is on the SD card root */
      try { await ensureBoot2350(); } catch (e) {
        console.warn('BOOT2350.uf2 install failed:', e);
        log('Warning: could not write BOOT2350.uf2 — ' + e.message);
      }

      /* Auto-install any missing alwaysInstalled apps */
      var installable = getInstallableApps();
      for (var i = 0; i < installable.length; i++) {
        var app = installable[i];
        if (!app.alwaysInstalled) continue;
        var sdName = app.sdFilename || (app.id + '.uf2');
        if (SD_APPS.indexOf(sdName) >= 0) continue;
        setStat(stat2, 'Installing required app: <b>' + app.name + '</b>…');
        try { await installApp(app); } catch (e) {
          log('Warning: could not install ' + app.name + ' — ' + e.message);
        }
      }

      /* Re-scan after auto-installs */
      files = await scanInstalledApps(DIR_HANDLE);
      SD_APPS = files;
      renderAppTable(files);

      /* Final status */
      var hasBoot = false;
      try {
        await DIR_HANDLE.getFileHandle('BOOT2350.uf2', { create: false });
        hasBoot = true;
      } catch (_) {}

      if (hasBoot) {
        setStat(stat2, '✓ SD card ready. <b>BOOT2350.uf2</b> installed on root. ' +
          'Install or remove apps below, then proceed to <b>Step 3</b>.', 'ok');
      } else {
        setStat(stat2, '✓ SD card opened, but <b>BOOT2350.uf2 could not be installed</b>. ' +
          'The bootloader boot menu will not work without it.', 'err');
      }
    });

    btn2Refresh.addEventListener('click', refreshApps);

    /* ── Install an app to tbd-apps/ (uses RAM release for bootloader) ── */
    async function installApp(app) {
      if (!DIR_HANDLE) return;

      var sdName = app.sdFilename || (app.id + '.uf2');
      var url = appSdUrl(app);
      if (!url) {
        setStat(statApps, 'No download URL for <b>' + app.name + '</b>.', 'err');
        return;
      }

      setStat(statApps, 'Downloading <b>' + app.name + '</b>…');
      showProg(prog2, prog2Bar, prog2Txt, 10);
      log('Downloading ' + app.name + ' from CDN…');

      try {
        var resp = await fetch(url);
        if (!resp.ok) throw new Error('Download failed: ' + resp.statusText);
        var data = new Uint8Array(await resp.arrayBuffer());
        showProg(prog2, prog2Bar, prog2Txt, 60);
        log('Downloaded ' + (data.length / 1024).toFixed(0) + ' KB');

        /* Verify SHA-256 if available */
        var rel = sdRelease(app);
        if (rel && rel.sha256 && window.crypto && window.crypto.subtle) {
          var hashBuf = await crypto.subtle.digest('SHA-256', data);
          var hashArr = Array.from(new Uint8Array(hashBuf));
          var hashHex = hashArr.map(function (b) {
            return b.toString(16).padStart(2, '0');
          }).join('');
          if (hashHex !== rel.sha256) {
            throw new Error('SHA-256 mismatch: expected ' +
              rel.sha256.substring(0, 16) + '…, got ' + hashHex.substring(0, 16) + '…');
          }
          log('SHA-256 verified ✓');
        }

        /* Ensure tbd-apps/ exists */
        var appsDir = await DIR_HANDLE.getDirectoryHandle('tbd-apps', { create: true });

        /* Write file */
        var fileHandle = await appsDir.getFileHandle(sdName, { create: true });
        var writable = await fileHandle.createWritable();
        await writable.write(data);
        await writable.close();
        showProg(prog2, prog2Bar, prog2Txt, 100);
        log('Wrote tbd-apps/' + sdName + ' (' + data.length + ' bytes)');

        setStat(statApps, '✓ <b>' + app.name + '</b> installed to tbd-apps/' + sdName, 'ok');
        hideProg(prog2);

        await refreshApps();
      } catch (e) {
        console.error(e);
        setStat(statApps, 'Install failed: ' + e.message, 'err');
        hideProg(prog2);
      }
    }

    /* ── Remove an app from tbd-apps/ ── */
    async function removeApp(app) {
      if (!DIR_HANDLE) return;
      var sdName = app.sdFilename || (app.id + '.uf2');

      try {
        var appsDir = await DIR_HANDLE.getDirectoryHandle('tbd-apps', { create: false });
        await appsDir.removeEntry(sdName);
        log('Removed tbd-apps/' + sdName);
        setStat(statApps, '✓ <b>' + app.name + '</b> removed.', 'ok');
        await refreshApps();
      } catch (e) {
        console.error(e);
        setStat(statApps, 'Remove failed: ' + e.message, 'err');
      }
    }

    /* ══════════════════════════════
       STEP 3 — Choose Boot Mode & Flash
       ══════════════════════════════ */

    /* Boot mode selector */
    window.selectMode = function (mode) {
      SELECTED_MODE = mode;
      var options = document.querySelectorAll('.mode-option');
      for (var i = 0; i < options.length; i++) options[i].classList.remove('selected');
      if (mode === 'bootloader') $('modeBootloader').classList.add('selected');
      else if (mode === 'groovebox') $('modeGroovebox').classList.add('selected');
      else if (mode === 'nuke') $('modeNuke').classList.add('selected');
    };

    /** Install BOOT2350.uf2 to SD root (needed for bootloader mode). */
    async function ensureBoot2350() {
      if (!DIR_HANDLE) return;
      try {
        await DIR_HANDLE.getFileHandle('BOOT2350.uf2', { create: false });
        log('BOOT2350.uf2 already on SD root');
        return;
      } catch (_) {}

      log('Downloading BOOT2350.uf2…');
      var resp = await fetch(BOOT2350_CDN);
      if (!resp.ok) throw new Error('BOOT2350.uf2 download failed: ' + resp.statusText);
      var data = new Uint8Array(await resp.arrayBuffer());
      var fh = await DIR_HANDLE.getFileHandle('BOOT2350.uf2', { create: true });
      var w = await fh.createWritable();
      await w.write(data);
      await w.close();
      log('Wrote BOOT2350.uf2 to SD root (' + data.length + ' bytes)');
    }

    /** Get the flash URL for the selected boot mode. */
    function getFlashUrl() {
      if (SELECTED_MODE === 'bootloader') return appCdnUrl(getApp('bootloader'));
      if (SELECTED_MODE === 'groovebox') return appCdnUrl(getApp('groovebox'));
      if (SELECTED_MODE === 'nuke') return appCdnUrl(getApp('flash-nuke'));
      return null;
    }

    var ctx3 = null;

    btn3Connect.addEventListener('click', async function () {
      try {
        btn3Connect.disabled = true;
        setStat(stat3, 'Put the RP2350 in <b>BOOTSEL mode</b>, then waiting for device…');
        ctx3 = await connectRP2350({
          onStatus: function (msg) { setStat(stat3, msg); }
        });
        btn3Flash.disabled = false;
        var modeLabels = {
          bootloader: 'Boot Menu',
          groovebox: 'Groovebox',
          nuke: 'Flash Nuke'
        };
        setStat(stat3, 'Connected to <b>' + (ctx3.info.productName || 'RP2350') +
          '</b>. Click <b>Flash Selected Firmware</b> to install <b>' +
          modeLabels[SELECTED_MODE] + '</b>.', 'ok');
      } catch (e) {
        console.error(e);
        setStat(stat3, 'Connection failed: ' + e.message +
          '<br>Make sure the RP2350 is in <b>BOOTSEL mode</b>.', 'err');
        btn3Connect.disabled = false;
        await disconnectRP2350(ctx3); ctx3 = null;
      }
    });

    btn3Flash.addEventListener('click', async function () {
      if (!ctx3) return;
      btn3Flash.disabled = true; btn3Connect.disabled = true;

      var url = getFlashUrl();
      if (!url) {
        setStat(stat3, 'No firmware URL for selected mode.', 'err');
        return;
      }

      try {
        var modeLabels = {
          bootloader: 'Boot Menu',
          groovebox: 'Groovebox',
          nuke: 'Flash Nuke'
        };
        setStat(stat3, 'Flashing <b>' + modeLabels[SELECTED_MODE] + '</b>…');
        await flashRP2350(ctx3, url, {
          onStatus: function (msg) { setStat(stat3, msg); },
          onProgress: function (pct) { showProg(prog3, prog3Bar, prog3Txt, pct); }
        });
        btn3Reboot.disabled = false;
        setStat(stat3, '✓ <b>' + modeLabels[SELECTED_MODE] +
          '</b> firmware flashed. Click <b>Reboot</b> to restart the RP2350.', 'ok');
      } catch (e) {
        console.error(e);
        setStat(stat3, 'Flash failed: ' + e.message, 'err');
        btn3Connect.disabled = false;
        await disconnectRP2350(ctx3); ctx3 = null;
      }
    });

    btn3Reboot.addEventListener('click', async function () {
      try {
        btn3Reboot.disabled = true;
        await rebootRP2350(ctx3);
        ctx3 = null;
        hideProg(prog3);
        $('card3').classList.add('done');

        if (SELECTED_MODE === 'bootloader') {
          doneMsg.innerHTML =
            'The bootloader is now active. On next power-up, <b>press the cursor-up ' +
            'button</b> to access the boot menu and choose between installed apps.<br>' +
            'Disconnect USB cables, wait 3 seconds, then reconnect via <b>back Port #1</b>.';
        } else if (SELECTED_MODE === 'groovebox') {
          doneMsg.innerHTML =
            'The groovebox firmware is active. The device will boot directly into the sequencer.<br>' +
            'Disconnect USB cables, wait 3 seconds, then reconnect via <b>back Port #1</b>. ' +
            'Open <a href="http://192.168.4.1" target="_blank">http://192.168.4.1</a> to use the device.';
        } else {
          doneMsg.innerHTML =
            'Flash nuke complete — the RP2350 flash is erased.<br>' +
            'To restore operation, run the App Manager again and flash the ' +
            '<b>Bootloader</b> or <b>Groovebox</b>.';
        }
        cardDone.style.display = 'block';
        setStat(stat3, '✓ RP2350 rebooted.', 'ok');
      } catch (e) {
        console.error(e);
        setStat(stat3, 'Reboot failed: ' + e.message, 'err');
        await disconnectRP2350(ctx3); ctx3 = null;
      }
    });

    </script>

Having trouble? See the `Troubleshooting <50_troubleshooting.html>`_ page.

.. include:: /_includes/newsletter.rst

.. include:: /_includes/footer-links.rst
