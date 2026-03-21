:orphan:

****************************
Release Archive
****************************

Browse and flash **any previous stable firmware release** to your TBD-16.
Each archived release includes the **matching SD card image** — firmware
and sound data always stay in sync.

All releases are available on
`GitHub <https://github.com/dadamachines/ctag-tbd/releases>`_.

.. note::

   This page performs a **Full SD Card Deploy** — both firmware and SD card
   contents are written for the selected version.  For the latest stable
   version with a simpler quick-update path, use the
   `Stable Channel <10_stable_channel.html>`_ page.

.. raw:: html

    <style>
      /* ── Release Archive Styles ── */
      .sd-recovery {
        max-width: 640px;
        margin: 1em 0 1.5em;
      }
      .sd-recovery .step-card {
        border: 1px solid var(--color-background-border, #d1d5db);
        border-radius: 8px;
        padding: 1.2em 1.5em;
        margin-bottom: 1em;
        background: var(--color-background-secondary, #fafafa);
      }
      .sd-recovery .step-card.active-step {
        border-color: #2563EB;
        box-shadow: 0 0 0 1px #2563EB;
      }
      .sd-recovery .step-num {
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
      .sd-recovery .step-card.done .step-num {
        background: #10b981;
      }
      .sd-recovery .step-hdr {
        display: flex;
        align-items: center;
        font-weight: 700;
        font-size: 1em;
        margin-bottom: 0.6em;
        color: var(--color-foreground-primary, #1a1a1a);
      }
      .sd-recovery .step-desc {
        font-size: 0.88em;
        color: var(--color-foreground-secondary, #555);
        line-height: 1.55;
        margin-bottom: 0.8em;
      }
      .sd-recovery .btn-row {
        display: flex;
        gap: 0.5em;
        flex-wrap: wrap;
      }
      .sd-recovery button {
        padding: 0.5em 1.2em;
        border: none;
        border-radius: 5px;
        font-size: 0.88em;
        font-weight: 600;
        cursor: pointer;
        color: #fff;
        transition: opacity 0.15s;
      }
      .sd-recovery button:disabled {
        opacity: 0.35;
        cursor: not-allowed;
      }
      .sd-recovery button:not(:disabled):hover {
        opacity: 0.85;
      }
      .sd-recovery .btn-primary   { background: #2563EB; }
      .sd-recovery .btn-success   { background: #16A34A; }
      .sd-recovery .btn-secondary { background: #6B7280; }
      .sd-recovery .progress-wrap {
        margin-top: 0.7em;
        background: #E5E7EB;
        border-radius: 4px;
        overflow: hidden;
        height: 20px;
        position: relative;
        display: none;
      }
      .sd-recovery .progress-bar {
        height: 100%;
        background: #2563EB;
        width: 0%;
        transition: width 0.15s;
      }
      .sd-recovery .progress-text {
        position: absolute;
        top: 0; left: 0; right: 0;
        text-align: center;
        line-height: 20px;
        font-size: 0.78em;
        font-weight: 600;
        color: #374151;
      }
      .sd-recovery .status {
        padding: 0.55em 0.85em;
        border-radius: 4px;
        font-size: 0.85em;
        margin-top: 0.7em;
        word-break: break-word;
        background: #F3F4F6;
        color: #374151;
      }
      .sd-recovery .status-ok {
        background: #DEF7EC; color: #065F46;
      }
      .sd-recovery .status-err {
        background: #FEE2E2; color: #991B1B;
      }
      .sd-recovery .status-info {
        background: #EFF6FF; color: #1E40AF;
      }
      .sd-recovery .file-log {
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
      .sd-recovery .skip-link {
        font-size: 0.82em;
        color: #6B7280;
        cursor: pointer;
        text-decoration: underline;
        margin-top: 0.4em;
        display: inline-block;
      }
      .sd-recovery .skip-link:hover {
        color: #2563EB;
      }
      .sd-recovery .complete-card {
        border: 2px solid #10b981;
        border-radius: 8px;
        padding: 1.5em;
        background: #f0fdf4;
        text-align: center;
        display: none;
      }
      .sd-recovery .complete-card h3 {
        color: #065f46;
        margin: 0 0 0.5em;
        font-size: 1.2em;
      }
      .sd-recovery .complete-card p {
        color: #047857;
        font-size: 0.92em;
        margin: 0;
      }
      .sd-recovery select {
        padding: 0.5em 0.8em;
        border: 1px solid #d1d5db;
        border-radius: 5px;
        font-size: 0.9em;
        width: 100%;
        max-width: 360px;
        margin-bottom: 0.5em;
      }

      /* Dark mode */
      body[data-theme="dark"] .sd-recovery .step-card {
        background: #1e293b;
        border-color: #334155;
      }
      body[data-theme="dark"] .sd-recovery select {
        background: #1e293b;
        color: #f1f5f9;
        border-color: #475569;
      }
      body[data-theme="dark"] .sd-recovery .step-hdr {
        color: #f1f5f9;
      }
      body[data-theme="dark"] .sd-recovery .step-desc {
        color: #94a3b8;
      }
      body[data-theme="dark"] .sd-recovery .status {
        background: #1e293b; color: #cbd5e1;
      }
      body[data-theme="dark"] .sd-recovery .status-ok {
        background: #064e3b; color: #6ee7b7;
      }
      body[data-theme="dark"] .sd-recovery .status-err {
        background: #450a0a; color: #fca5a5;
      }
      body[data-theme="dark"] .sd-recovery .status-info {
        background: #1e3a5f; color: #93c5fd;
      }
      body[data-theme="dark"] .sd-recovery .file-log {
        background: #0f172a; border-color: #334155; color: #cbd5e1;
      }
      body[data-theme="dark"] .sd-recovery .complete-card {
        background: #064e3b; border-color: #10b981;
      }
      body[data-theme="dark"] .sd-recovery .complete-card h3 {
        color: #6ee7b7;
      }
      body[data-theme="dark"] .sd-recovery .complete-card p {
        color: #a7f3d0;
      }
      @media (prefers-color-scheme: dark) {
        body:not([data-theme="light"]) .sd-recovery .step-card {
          background: #1e293b; border-color: #334155;
        }
        body:not([data-theme="light"]) .sd-recovery select {
          background: #1e293b; color: #f1f5f9; border-color: #475569;
        }
        body:not([data-theme="light"]) .sd-recovery .step-hdr { color: #f1f5f9; }
        body:not([data-theme="light"]) .sd-recovery .step-desc { color: #94a3b8; }
        body:not([data-theme="light"]) .sd-recovery .status {
          background: #1e293b; color: #cbd5e1;
        }
        body:not([data-theme="light"]) .sd-recovery .status-ok {
          background: #064e3b; color: #6ee7b7;
        }
        body:not([data-theme="light"]) .sd-recovery .status-err {
          background: #450a0a; color: #fca5a5;
        }
        body:not([data-theme="light"]) .sd-recovery .status-info {
          background: #1e3a5f; color: #93c5fd;
        }
        body:not([data-theme="light"]) .sd-recovery .file-log {
          background: #0f172a; border-color: #334155; color: #cbd5e1;
        }
        body:not([data-theme="light"]) .sd-recovery .complete-card {
          background: #064e3b; border-color: #10b981;
        }
        body:not([data-theme="light"]) .sd-recovery .complete-card h3 { color: #6ee7b7; }
        body:not([data-theme="light"]) .sd-recovery .complete-card p { color: #a7f3d0; }
      }
    </style>

    <div class="sd-recovery" id="sdRecovery">

      <!-- ════════ VERSION SELECTOR ════════ -->
      <div class="step-card active-step" id="cardSelect" style="border-color:#7c3aed; box-shadow:0 0 0 1px #7c3aed;">
        <div class="step-hdr" style="color:#7c3aed;">
          <span class="step-num" style="background:#7c3aed;">▼</span> Select Firmware Version
        </div>
        <div class="step-desc">
          Choose a release to flash. The latest stable version is at the top.
        </div>
        <select id="versionSelect" disabled>
          <option value="">Loading releases…</option>
        </select>
        <div class="status status-info" id="statPkg">Fetching release list from GitHub…</div>
      </div>

      <!-- ════════ Step 1 — Flash MSC + Mount SD ════════ -->
      <div class="step-card active-step" id="card1">
        <div class="step-hdr"><span class="step-num">1</span> Flash MSC Firmware &amp; Mount SD Card</div>
        <div class="step-desc">
          Connect both USB-C cables: <b>front JTAG port</b> (serial) and <b>back Port&nbsp;#1</b> (SD card drive).
          This step flashes the USB Mass Storage firmware via the front port. After reboot the
          SD card will appear as a removable drive on the <b>back port</b>.
        </div>
        <div class="btn-row">
          <button id="btn1Connect" class="btn-primary" disabled>Loading…</button>
          <button id="btn1Go" class="btn-success" disabled>Flash &amp; Switch to SD Card Mode</button>
        </div>
        <div class="progress-wrap" id="prog1"><div class="progress-bar" id="prog1Bar"></div><span class="progress-text" id="prog1Txt">0 %</span></div>
        <div class="status" id="stat1">Loading flash tool…</div>
        <span class="skip-link" id="skip1" style="display:none;">Already have MSC firmware? Click here to skip flashing and just switch to SD card mode →</span>
      </div>

      <!-- ════════ Step 2 — Write SD Card ════════ -->
      <div class="step-card" id="card2">
        <div class="step-hdr"><span class="step-num">2</span> Write SD Card Image</div>
        <div class="step-desc">
          The SD card should now be mounted via <b>back Port&nbsp;#1</b> (look for a <b>"NO NAME"</b> drive).
          Select the mounted drive below. The SD card image will be downloaded from GitHub,
          extracted, and written directly.
        </div>
        <div class="btn-row">
          <button id="btn2Pick" class="btn-primary" disabled>Select SD Card Drive</button>
        </div>
        <div class="progress-wrap" id="prog2"><div class="progress-bar" id="prog2Bar"></div><span class="progress-text" id="prog2Txt">0 %</span></div>
        <div class="status" id="stat2">Complete Step 1 first, or click <b>Select SD Card Drive</b> if the drive is already mounted.</div>
        <div class="file-log" id="fileLog"></div>
      </div>

      <!-- ════════ Step 3 — Switch Back ════════ -->
      <div class="step-card" id="card3">
        <div class="step-hdr"><span class="step-num">3</span> Switch Back to Normal Mode</div>
        <div class="step-desc">
          Safely eject the SD card drive from your computer before proceeding.
          Keep <b>back Port&nbsp;#1</b> connected for power and the <b>front JTAG port</b> connected for serial.
          Click <b>Connect</b> to reconnect via the front JTAG port.
        </div>
        <div class="btn-row">
          <button id="btn3Connect" class="btn-primary" disabled>Connect</button>
          <button id="btn3Go" class="btn-success" disabled>Switch to Normal Mode</button>
        </div>
        <div class="status" id="stat3">Complete Step 2 first. <b>Safely eject the drive</b> before clicking Connect.</div>
      </div>

      <!-- ════════ Step 4 — Flash P4 ════════ -->
      <div class="step-card" id="card4">
        <div class="step-hdr"><span class="step-num">4</span> Flash ESP32-P4 Firmware</div>
        <div class="step-desc">
          <b>Power-cycle the device</b>: unplug <b>back Port&nbsp;#1</b>, wait 3 seconds, replug it.
          Then click <b>Connect</b> via the <b>front JTAG port</b>.
        </div>
        <div class="btn-row">
          <button id="btn4Connect" class="btn-primary" disabled>Connect</button>
          <button id="btn4Flash" class="btn-success" disabled>Flash ctag-tbd Firmware</button>
        </div>
        <div class="progress-wrap" id="prog4"><div class="progress-bar" id="prog4Bar"></div><span class="progress-text" id="prog4Txt">0 %</span></div>
        <div class="status" id="stat4">Complete Step 3 first. <b>Power-cycle</b>, then click <b>Connect</b>.</div>
      </div>

      <!-- ════════ Step 5 — Flash RP2350 ════════ -->
      <div class="step-card" id="card5">
        <div class="step-hdr"><span class="step-num">5</span> Flash RP2350 (Pico Firmware)</div>
        <div class="step-desc">
          <b>Connect back USB-C Port&nbsp;#2</b> (closest to the edge).
          Put the RP2350 in <b>BOOTSEL mode</b> (hold BOOTSEL + press RESET),
          then click <b>Connect</b>.
        </div>
        <div class="btn-row">
          <button id="btn5Connect" class="btn-primary" disabled>Connect</button>
          <button id="btn5Flash" class="btn-success" disabled>Flash Pico Firmware</button>
          <button id="btn5Reboot" class="btn-secondary" disabled>Reboot</button>
        </div>
        <div class="progress-wrap" id="prog5"><div class="progress-bar" id="prog5Bar"></div><span class="progress-text" id="prog5Txt">0 %</span></div>
        <div class="status" id="stat5">Complete Step 4 first. Put the RP2350 in <b>BOOTSEL mode</b>, then click <b>Connect</b>.</div>
      </div>

      <!-- ════════ DONE ════════ -->
      <div class="complete-card" id="cardDone">
        <h3>✓ Archive Firmware Deployed</h3>
        <p>Your TBD-16 is running the selected firmware version with its matching SD card image.<br>
        <b>Remove all USB cables</b>, wait 3 seconds, reconnect via <b>back Port&nbsp;#1</b>,
        then open <b>http://192.168.4.1</b>.</p>
      </div>

    </div><!-- end sd-recovery -->

    <script type="module">
    /* ══════════════════════════════════════════════════════════
       Release Archive — version picker + full SD card deploy
       Uses tbd-flasher-p4.js and tbd-flasher-rp2350.js
       ══════════════════════════════════════════════════════════ */

    import {
      loadEspTool, connectP4, flashP4, disconnectP4,
      flashMscAndSwitchOta, switchOtaSlot, resetDevice,
      toBinStr, buildOtaData
    } from '../_static/js/tbd-flasher-p4.js';

    import {
      loadPicoboot, connectRP2350, flashRP2350, rebootRP2350, disconnectRP2350
    } from '../_static/js/tbd-flasher-rp2350.js';

    /* ── DOM refs ── */
    var $ = function (id) { return document.getElementById(id); };

    var versionSelect = $('versionSelect'), statPkg = $('statPkg');

    var btn1Connect = $('btn1Connect'), btn1Go = $('btn1Go'), skip1 = $('skip1');
    var prog1 = $('prog1'), prog1Bar = $('prog1Bar'), prog1Txt = $('prog1Txt');
    var stat1 = $('stat1');

    var btn2Pick = $('btn2Pick');
    var prog2 = $('prog2'), prog2Bar = $('prog2Bar'), prog2Txt = $('prog2Txt');
    var stat2 = $('stat2'), fileLog = $('fileLog');

    var btn3Connect = $('btn3Connect'), btn3Go = $('btn3Go'), stat3 = $('stat3');

    var btn4Connect = $('btn4Connect'), btn4Flash = $('btn4Flash');
    var prog4 = $('prog4'), prog4Bar = $('prog4Bar'), prog4Txt = $('prog4Txt');
    var stat4 = $('stat4');

    var btn5Connect = $('btn5Connect'), btn5Flash = $('btn5Flash'), btn5Reboot = $('btn5Reboot');
    var prog5 = $('prog5'), prog5Bar = $('prog5Bar'), prog5Txt = $('prog5Txt');
    var stat5 = $('stat5');

    var cardDone = $('cardDone');

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

    /* ── State ── */
    var FIRMWARE_CDN = 'https://dadamachines.github.io/dada-tbd-firmware';
    var CHANNEL = 'stable';
    var TUSB_MSC_URL = FIRMWARE_CDN + '/' + CHANNEL + '/p4/tusb_msc.bin';
    var RELEASES = [];
    var SELECTED = null;   /* { tag, name, p4Url, picoUrl, zipUrl, hashUrl, htmlUrl } */
    var PICO_UF2_URL = null;
    var REBOOT_WAIT = 20;

    /* ══════════════════════════════
       Fetch all stable releases from GitHub API
       ══════════════════════════════ */
    async function fetchReleases() {
      var resp = await fetch('https://api.github.com/repos/dadamachines/ctag-tbd/releases?per_page=50');
      if (!resp.ok) throw new Error('GitHub API error: ' + resp.statusText);
      var releases = await resp.json();
      return releases.filter(function (r) { return !r.prerelease && !r.draft; });
    }

    /* Build CDN URLs for P4/Pico + GitHub asset URLs for SD card */
    function buildUrls(release) {
      var tag = release.tag_name;
      var base = FIRMWARE_CDN + '/' + CHANNEL;
      var urls = {
        tag: tag,
        name: release.name,
        htmlUrl: release.html_url,
        p4Url: base + '/p4/dada-tbd-16-' + tag + '-unified.bin',
        picoUrl: base + '/pico/dada-tbd-16-' + tag + '-pico.uf2',
        zipUrl: null,
        hashUrl: null
      };
      /* SD card assets come from the GitHub release (versioned per-release) */
      if (release.assets) {
        for (var i = 0; i < release.assets.length; i++) {
          var a = release.assets[i];
          if (a.name === 'tbd-sd-card.zip') urls.zipUrl = a.browser_download_url;
          if (a.name === 'tbd-sd-card-hash.txt') urls.hashUrl = a.browser_download_url;
        }
      }
      return urls;
    }

    /* ══════════════════════════════
       Version selection + UI reset
       ══════════════════════════════ */
    function resetSteps() {
      btn1Connect.disabled = true; btn1Go.disabled = true;
      btn2Pick.disabled = true;
      btn3Connect.disabled = true; btn3Go.disabled = true;
      btn4Connect.disabled = true; btn4Flash.disabled = true;
      btn5Connect.disabled = true; btn5Flash.disabled = true; btn5Reboot.disabled = true;
      skip1.style.display = 'none';

      hideProg(prog1); hideProg(prog2); hideProg(prog4); hideProg(prog5);

      setStat(stat1, 'Select a version, then click <b>Connect</b>.');
      setStat(stat2, 'Complete Step 1 first, or click <b>Select SD Card Drive</b> if the drive is already mounted.');
      setStat(stat3, 'Complete Step 2 first. <b>Safely eject the drive</b> before clicking Connect.');
      setStat(stat4, 'Complete Step 3 first. <b>Power-cycle</b>, then click <b>Connect</b>.');
      setStat(stat5, 'Complete Step 4 first. Put the RP2350 in <b>BOOTSEL mode</b>, then click <b>Connect</b>.');

      fileLog.style.display = 'none';
      fileLog.textContent = '';
      cardDone.style.display = 'none';
    }

    function selectVersion(tag) {
      var r = RELEASES.find(function (rel) { return rel.tag_name === tag; });
      if (!r) return;
      SELECTED = buildUrls(r);
      PICO_UF2_URL = SELECTED.picoUrl;

      resetSteps();

      if (!SELECTED.zipUrl) {
        setStat(statPkg, '\u26a0 <b>' + SELECTED.tag + '</b> does not include an SD card image. ' +
          'This release predates the automated build system. ' +
          '<a href="' + SELECTED.htmlUrl + '" target="_blank">Release notes \u2192</a>', 'err');
        return;
      }

      setStat(statPkg, 'Selected <b>' + SELECTED.tag + '</b> \u2014 ' + SELECTED.name +
        '. SD card image included. ' +
        '<a href="' + SELECTED.htmlUrl + '" target="_blank">Release notes \u2192</a>', 'info');

      btn1Connect.textContent = 'Connect';
      btn1Connect.disabled = false;
      skip1.style.display = 'inline-block';
      setStat(stat1, 'Click <b>Connect</b>. Make sure <b>both</b> USB cables are connected: ' +
        '<b>front JTAG port</b> (serial) and <b>back Port&nbsp;#1</b> (SD card drive).');
    }

    /* ══════════════════════════════
       INIT — load tools + fetch releases
       ══════════════════════════════ */
    try {
      if (!('serial' in navigator)) {
        setStat(statPkg, 'Your browser does not support <b>WebSerial</b>. Use Chrome, Edge, or Opera on desktop.', 'err');
        throw new Error('WebSerial not supported');
      }

      var [releases, _esp, _pico] = await Promise.all([
        fetchReleases(),
        loadEspTool(),
        loadPicoboot('../_static/picoflash').catch(function (e) {
          console.warn('Picoboot load failed:', e);
        })
      ]);

      RELEASES = releases;

      if (RELEASES.length === 0) {
        setStat(statPkg, 'No stable releases found.', 'err');
        throw new Error('No releases');
      }

      /* Populate dropdown */
      versionSelect.innerHTML = '';
      RELEASES.forEach(function (r) {
        var opt = document.createElement('option');
        opt.value = r.tag_name;
        opt.textContent = r.tag_name + ' \u2014 ' + r.name;
        versionSelect.appendChild(opt);
      });
      versionSelect.disabled = false;

      selectVersion(RELEASES[0].tag_name);

    } catch (e) {
      if (e.message !== 'WebSerial not supported' && e.message !== 'No releases') {
        setStat(statPkg, 'Failed to load: ' + e.message, 'err');
      }
      throw e;
    }

    /* ── Version change handler ── */
    versionSelect.addEventListener('change', function () {
      selectVersion(versionSelect.value);
    });

    /* ══════════════════════════════
       Step 1 — Flash MSC + switch OTA
       ══════════════════════════════ */
    var ctxB1 = null;

    btn1Connect.addEventListener('click', async function () {
      try {
        btn1Connect.disabled = true;
        ctxB1 = await connectP4({
          onStatus: function (msg) { setStat(stat1, msg); }
        });
        btn1Go.disabled = false;
        setStat(stat1, 'Connected to <b>' + ctxB1.chip + '</b>. Click <b>Flash &amp; Switch</b> to proceed.', 'ok');
      } catch (e) {
        console.error(e);
        setStat(stat1, 'Connection failed: ' + e.message, 'err');
        btn1Connect.disabled = false;
        await disconnectP4(ctxB1); ctxB1 = null;
      }
    });

    btn1Go.addEventListener('click', async function () {
      if (!ctxB1) return;
      btn1Go.disabled = true; btn1Connect.disabled = true;
      versionSelect.disabled = true;
      try {
        await flashMscAndSwitchOta(ctxB1, TUSB_MSC_URL, {
          onStatus: function (msg) { setStat(stat1, msg); },
          onProgress: function (pct) { showProg(prog1, prog1Bar, prog1Txt, pct); }
        });
        await resetDevice(ctxB1);
        await disconnectP4(ctxB1); ctxB1 = null;
        startRebootCountdown();
      } catch (e) {
        console.error(e);
        setStat(stat1, 'Failed: ' + e.message, 'err');
        btn1Connect.disabled = false;
        versionSelect.disabled = false;
        await disconnectP4(ctxB1); ctxB1 = null;
      }
    });

    skip1.addEventListener('click', async function () {
      try {
        skip1.style.display = 'none';
        versionSelect.disabled = true;
        ctxB1 = await connectP4({
          onStatus: function (msg) { setStat(stat1, msg); }
        });
        setStat(stat1, 'Connected to <b>' + ctxB1.chip + '</b>. Switching to SD card mode\u2026', 'info');
        await switchOtaSlot(ctxB1, 1, {
          onStatus: function (msg) { setStat(stat1, msg); }
        });
        await resetDevice(ctxB1);
        await disconnectP4(ctxB1); ctxB1 = null;
        startRebootCountdown();
      } catch (e) {
        console.error(e);
        setStat(stat1, 'Failed: ' + e.message, 'err');
        btn1Connect.disabled = false;
        skip1.style.display = 'inline-block';
        versionSelect.disabled = false;
        await disconnectP4(ctxB1); ctxB1 = null;
      }
    });

    function startRebootCountdown() {
      var remaining = REBOOT_WAIT;
      hideProg(prog1);
      showProg(prog1, prog1Bar, prog1Txt, 0);
      setStat(stat1,
        '\u2713 Firmware written &amp; OTA switched. The device should reboot into SD card mode.<br>' +
        '<b>If the SD card drive does not appear within ' + REBOOT_WAIT + ' seconds:</b><br>' +
        '&nbsp;&nbsp;\u2460 Press the <b>RESET button</b> on the back, <i>or</i><br>' +
        '&nbsp;&nbsp;\u2461 Unplug <b>both</b> cables, wait 3 s, replug them.', 'ok');

      var iv = setInterval(function () {
        remaining--;
        var pct = Math.round((REBOOT_WAIT - remaining) / REBOOT_WAIT * 100);
        showProg(prog1, prog1Bar, prog1Txt, pct);
        prog1Txt.textContent = 'Waiting for reboot: ' + remaining + ' s';
        if (remaining <= 0) {
          clearInterval(iv);
          hideProg(prog1);
          btn2Pick.disabled = false;
          setStat(stat1, '\u2713 Ready. Look for the <b>"NO NAME"</b> drive.', 'ok');
          setStat(stat2, 'Select the <b>"NO NAME"</b> SD card drive below.');
        }
      }, 1000);
    }

    /* ══════════════════════════════
       Step 2 — Write SD Card via File System Access API
       ══════════════════════════════ */

    async function cleanMacOSFiles(dirHandle, logFn) {
      var childDirs = [];
      var dotFiles = [];
      for await (var entry of dirHandle.entries()) {
        var eName = entry[0], eHandle = entry[1];
        if (eHandle.kind === 'directory' && !eName.startsWith('.')) childDirs.push(eHandle);
        if (eName.startsWith('._')) dotFiles.push(eName);
      }
      for (var d = 0; d < childDirs.length; d++) await cleanMacOSFiles(childDirs[d], logFn);
      for (var f = 0; f < dotFiles.length; f++) {
        try { await dirHandle.removeEntry(dotFiles[f]); if (logFn) logFn('DEL  ' + dotFiles[f]); } catch (_) {}
      }
    }

    btn2Pick.addEventListener('click', async function () {
      if (!SELECTED || !SELECTED.zipUrl) {
        setStat(stat2, 'No SD card image available for this release.', 'err');
        return;
      }

      var dirHandle;
      try {
        dirHandle = await window.showDirectoryPicker({ mode: 'readwrite' });
      } catch (e) {
        if (e.name === 'AbortError') return;
        setStat(stat2, 'Could not access directory: ' + e.message, 'err');
        return;
      }

      btn2Pick.disabled = true;
      fileLog.style.display = 'block';
      fileLog.textContent = '';
      function log(msg) { fileLog.textContent += msg + '\n'; fileLog.scrollTop = fileLog.scrollHeight; }

      try {
        /* Load JSZip */
        if (!window.JSZip) {
          await new Promise(function (resolve, reject) {
            var s = document.createElement('script');
            s.src = 'https://unpkg.com/jszip@3.10.1/dist/jszip.min.js';
            s.onload = resolve;
            s.onerror = function () { reject(new Error('JSZip load failed')); };
            document.head.appendChild(s);
          });
        }

        /* Download ZIP with progress */
        setStat(stat2, 'Downloading SD card image for <b>' + SELECTED.tag + '</b>\u2026');
        var zipResp = await fetch(SELECTED.zipUrl);
        if (!zipResp.ok) throw new Error('ZIP download failed: ' + zipResp.statusText);
        var contentLength = parseInt(zipResp.headers.get('Content-Length') || '0', 10);
        var reader = zipResp.body.getReader();
        var chunks = []; var received = 0;
        while (true) {
          var result = await reader.read();
          if (result.done) break;
          chunks.push(result.value);
          received += result.value.length;
          if (contentLength > 0) {
            var dlPct = Math.round(received / contentLength * 50);
            showProg(prog2, prog2Bar, prog2Txt, dlPct);
            setStat(stat2, 'Downloading\u2026 ' + (received / 1024 / 1024).toFixed(1) + ' / ' + (contentLength / 1024 / 1024).toFixed(1) + ' MB');
          }
        }
        var zipBuf = new Uint8Array(received);
        var offset = 0;
        for (var ci = 0; ci < chunks.length; ci++) { zipBuf.set(chunks[ci], offset); offset += chunks[ci].length; }
        chunks = null;
        log('Downloaded ' + (received / 1024 / 1024).toFixed(1) + ' MB');

        /* Download hash */
        var sdHash = '';
        if (SELECTED.hashUrl) {
          try {
            var hashResp = await fetch(SELECTED.hashUrl);
            sdHash = (await hashResp.text()).trim();
            log('Hash: ' + sdHash);
          } catch (_) { log('Hash file not available'); }
        }

        /* Extract ZIP */
        setStat(stat2, 'Extracting ZIP\u2026');
        var zip = await JSZip.loadAsync(zipBuf);
        zipBuf = null;
        var entries = Object.keys(zip.files);
        var total = entries.length;
        log('ZIP contains ' + total + ' entries');

        /* Write files */
        setStat(stat2, 'Writing files to SD card\u2026');
        var written = 0, errors = 0;

        async function getDir(root, pathParts) {
          var cur = root;
          for (var p = 0; p < pathParts.length; p++)
            cur = await cur.getDirectoryHandle(pathParts[p], { create: true });
          return cur;
        }

        for (var ei = 0; ei < entries.length; ei++) {
          var name = entries[ei];
          var zipEntry = zip.files[name];
          var pct = 50 + Math.round((ei + 1) / total * 45);
          showProg(prog2, prog2Bar, prog2Txt, pct);

          if (name.indexOf('__MACOSX') >= 0 || name.indexOf('.DS_Store') >= 0) continue;
          var baseName = name.split('/').pop();
          if (baseName && baseName.indexOf('._') === 0) continue;

          var parts = name.split('/').filter(function (s) { return s.length > 0; });
          if (parts.length === 0) continue;

          try {
            if (zipEntry.dir) {
              await getDir(dirHandle, parts);
              log('DIR  ' + name);
            } else {
              var parentDir = parts.length > 1
                ? await getDir(dirHandle, parts.slice(0, -1))
                : dirHandle;
              var fileName = parts[parts.length - 1];
              var fileHandle = await parentDir.getFileHandle(fileName, { create: true });
              var writable = await fileHandle.createWritable();
              var data = await zipEntry.async('uint8array');
              await writable.write(data);
              await writable.close();
              log('FILE ' + name + '  (' + data.length + ' bytes)');
              data = null;
            }
            written++;
          } catch (fe) {
            errors++;
            log('ERR  ' + name + ': ' + fe.message);
          }

          if (ei % 10 === 0) await new Promise(function (r) { setTimeout(r, 0); });
        }

        /* Write .version + hash */
        if (sdHash) {
          try {
            var vh = await dirHandle.getFileHandle('.version', { create: true });
            var vw = await vh.createWritable(); await vw.write(sdHash); await vw.close();
            log('FILE .version  (' + sdHash + ')'); written++;
          } catch (ve) { errors++; log('ERR  .version: ' + ve.message); }

          try {
            var hh = await dirHandle.getFileHandle('tbd-sd-card-hash.txt', { create: true });
            var hw = await hh.createWritable(); await hw.write(sdHash); await hw.close();
            log('FILE tbd-sd-card-hash.txt  (' + sdHash + ')');
          } catch (he) { log('ERR  tbd-sd-card-hash.txt: ' + he.message); }
        }

        /* Cleanup macOS files */
        setStat(stat2, 'Cleaning up metadata files\u2026');
        showProg(prog2, prog2Bar, prog2Txt, 97);
        try { await cleanMacOSFiles(dirHandle, log); } catch (_) {}

        zip = null;
        showProg(prog2, prog2Bar, prog2Txt, 100);

        if (errors > 0) {
          setStat(stat2, '\u26a0 Written <b>' + written + '</b> items with <b>' + errors + '</b> error(s). Check the log above.', 'err');
        } else {
          setStat(stat2, '\u2713 <b>' + written + '</b> files written. <b>\u23cf Please safely eject the drive</b> before proceeding.', 'ok');
        }
        btn3Connect.disabled = false;
        setStat(stat3, '<b>Safely eject the SD card drive</b>, then click <b>Connect</b> via the front JTAG port.');
      } catch (e) {
        console.error(e);
        setStat(stat2, 'Failed: ' + e.message, 'err');
        btn2Pick.disabled = false;
      }
    });

    /* ══════════════════════════════
       Step 3 — Switch back to normal mode
       ══════════════════════════════ */
    var ctxB3 = null;

    btn3Connect.addEventListener('click', async function () {
      try {
        btn3Connect.disabled = true;
        ctxB3 = await connectP4({
          onStatus: function (msg) { setStat(stat3, msg); }
        });
        btn3Go.disabled = false;
        setStat(stat3, 'Connected to <b>' + ctxB3.chip + '</b>. Click <b>Switch to Normal Mode</b>.', 'ok');
      } catch (e) {
        console.error(e);
        setStat(stat3, 'Connection failed: ' + e.message, 'err');
        btn3Connect.disabled = false;
        await disconnectP4(ctxB3); ctxB3 = null;
      }
    });

    btn3Go.addEventListener('click', async function () {
      if (!ctxB3) return;
      btn3Go.disabled = true; btn3Connect.disabled = true;
      try {
        await switchOtaSlot(ctxB3, null, {
          onStatus: function (msg) { setStat(stat3, msg); }
        });
        await resetDevice(ctxB3);
        await disconnectP4(ctxB3); ctxB3 = null;

        btn4Connect.disabled = false;
        setStat(stat3, '\u2713 Switched to normal mode.', 'ok');
        setStat(stat4, '<b>Power-cycle the device</b>: unplug back Port&nbsp;#1, wait 3 s, replug. Then click <b>Connect</b>.');
      } catch (e) {
        console.error(e);
        setStat(stat3, 'Failed: ' + e.message, 'err');
        btn3Connect.disabled = false;
        await disconnectP4(ctxB3); ctxB3 = null;
      }
    });

    /* ══════════════════════════════
       Step 4 — Flash P4 firmware
       ══════════════════════════════ */
    var ctxB4 = null;

    btn4Connect.addEventListener('click', async function () {
      try {
        btn4Connect.disabled = true;
        ctxB4 = await connectP4({
          onStatus: function (msg) { setStat(stat4, msg); }
        });
        btn4Flash.disabled = false;
        setStat(stat4, 'Connected to <b>' + ctxB4.chip + '</b>. Click <b>Flash ctag-tbd Firmware</b>.', 'ok');
      } catch (e) {
        console.error(e);
        setStat(stat4, 'Connection failed: ' + e.message, 'err');
        btn4Connect.disabled = false;
        await disconnectP4(ctxB4); ctxB4 = null;
      }
    });

    btn4Flash.addEventListener('click', async function () {
      if (!ctxB4 || !SELECTED) return;
      btn4Flash.disabled = true; btn4Connect.disabled = true;
      try {
        await flashP4(ctxB4, SELECTED.p4Url, 0x0, {
          onStatus: function (msg) { setStat(stat4, msg); },
          onProgress: function (pct) { showProg(prog4, prog4Bar, prog4Txt, pct); }
        });
        await resetDevice(ctxB4);
        await disconnectP4(ctxB4); ctxB4 = null;

        setStat(stat4, '\u2713 ESP32-P4 firmware <b>' + SELECTED.tag + '</b> updated. <b>Remove all USB cables</b>, wait 3 s, reconnect via back Port&nbsp;#1.', 'ok');

        if (PICO_UF2_URL) {
          btn5Connect.disabled = false;
          setStat(stat5, '<b>Connect back USB-C Port&nbsp;#2</b>, put RP2350 in <b>BOOTSEL mode</b>, then click <b>Connect</b>.');
        } else {
          setStat(stat5, 'No Pico firmware available \u2014 skip this step.', 'info');
          cardDone.style.display = 'block';
        }
      } catch (e) {
        console.error(e);
        setStat(stat4, 'Flash failed: ' + e.message, 'err');
        btn4Connect.disabled = false;
        await disconnectP4(ctxB4); ctxB4 = null;
      }
    });

    /* ══════════════════════════════
       Step 5 — Flash RP2350
       ══════════════════════════════ */
    var ctxB5 = null;

    btn5Connect.addEventListener('click', async function () {
      try {
        btn5Connect.disabled = true;
        ctxB5 = await connectRP2350({
          onStatus: function (msg) { setStat(stat5, msg); }
        });
        btn5Flash.disabled = false;
        setStat(stat5, 'Connected to <b>' + (ctxB5.info.productName || 'RP2350') + '</b>. Click <b>Flash Pico Firmware</b>.', 'ok');
      } catch (e) {
        console.error(e);
        setStat(stat5, 'Connection failed: ' + e.message, 'err');
        btn5Connect.disabled = false;
        await disconnectRP2350(ctxB5); ctxB5 = null;
      }
    });

    btn5Flash.addEventListener('click', async function () {
      if (!ctxB5) return;
      btn5Flash.disabled = true; btn5Connect.disabled = true;
      try {
        await flashRP2350(ctxB5, PICO_UF2_URL, {
          onStatus: function (msg) { setStat(stat5, msg); },
          onProgress: function (pct) { showProg(prog5, prog5Bar, prog5Txt, pct); }
        });
        btn5Reboot.disabled = false;
        setStat(stat5, '\u2713 RP2350 firmware updated. Click <b>Reboot</b>.', 'ok');
      } catch (e) {
        console.error(e);
        setStat(stat5, 'Flash failed: ' + e.message, 'err');
        btn5Connect.disabled = false;
        await disconnectRP2350(ctxB5); ctxB5 = null;
      }
    });

    btn5Reboot.addEventListener('click', async function () {
      try {
        btn5Reboot.disabled = true;
        await rebootRP2350(ctxB5);
        ctxB5 = null;
        hideProg(prog5);
        setStat(stat5, '\u2713 RP2350 rebooted.', 'ok');
        cardDone.style.display = 'block';
      } catch (e) {
        console.error(e);
        setStat(stat5, 'Reboot failed: ' + e.message, 'err');
        await disconnectRP2350(ctxB5); ctxB5 = null;
      }
    });

    </script>

Having trouble? See the `Troubleshooting <50_troubleshooting.html>`_ page.
