:orphan:

****************************
Beta Channel
****************************

.. warning::

   This is the **beta channel** — firmware here is built from the ``staging`` branch
   or from ``feature-test/*`` branches and may contain unfinished features or
   regressions. Use the `Stable Channel <10_stable_channel.html>`_ for production use.

Flash **pre-release firmware** and **SD card image** to your TBD-16 directly from
the browser — no card reader, no terminal commands, no opening the device.

Use the **channel selector** below to choose between the latest staging build
or any active feature-test branch build.

.. dropdown:: How it Works

   **Path A — Quick Update** (recommended):

   1. Flashes the selected P4 firmware to the ESP32-P4
   2. Flashes the RP2350 co-processor firmware
   3. You update the WebUI over WiFi from the device

   **Path B — Full SD Card Deploy** (fresh install / corrupted SD card):

   1. Flashes the USB Mass Storage firmware (mounts the SD card via USB)
   2. Downloads and extracts the SD card image directly onto the device
   3. Switches the device back to normal operation
   4. Flashes the P4 firmware
   5. Flashes the RP2350 co-processor firmware

   **Hardware setup:**

   - **Front JTAG port** (USB-C #3) → serial communication (Steps 1, 3, 4 / Path A Step 1)
   - **Back USB-C Port #1** → SD card drive + power (Steps 1, 2 / always connected)
   - **Back USB-C Port #2** → RP2350 BOOTSEL flash (Step 5 / Path A Step 2)

   **Browser:** Chrome, Edge or Opera required (WebSerial + WebUSB).

   **Time:** 5–10 minutes

.. raw:: html

    <style>
      /* ── Beta Channel Styles ── */
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
      .sd-recovery .btn-cyan      { background: #0891B2; }
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

      <!-- ════════ CHANNEL SELECTOR ════════ -->
      <div class="step-card active-step" id="cardSelect" style="border-color:#7c3aed; box-shadow:0 0 0 1px #7c3aed;">
        <div class="step-hdr" style="color:#7c3aed;">
          <span class="step-num" style="background:#7c3aed;">▼</span> Select Channel
        </div>
        <div class="step-desc">
          Choose a pre-release channel. <b>Staging</b> is the default.
          Feature-test channels appear when engineers push to <code>feature-test/*</code> branches.
        </div>
        <select id="channelSelect" disabled style="width:100%; padding:0.5em; border-radius:5px; border:1px solid #d1d5db; font-size:0.92em; margin-bottom:0.6em; background:var(--color-background-primary,#fff); color:var(--color-foreground-primary,#1a1a1a);">
          <option value="">Loading channels…</option>
        </select>
        <div class="status status-info" id="statPkg">Discovering available channels…</div>
      </div>

      <!-- ════════ PATH CHOOSER ════════ -->
      <div class="step-card" id="cardPathChooser" style="display:none; border-color:#7c3aed; box-shadow:0 0 0 1px #7c3aed;">
        <div class="step-hdr" style="color:#7c3aed;">
          <span class="step-num" style="background:#7c3aed;">?</span> Choose Your Update Method
        </div>
        <div style="display:flex; gap:1em; flex-wrap:wrap; margin-top:0.3em;">
          <div id="optionCardA" style="flex:1; min-width:220px; border:2px solid #0891B2; border-radius:8px; padding:1em 1.2em; cursor:pointer; background:var(--color-background-secondary,#f0fdfa); transition: box-shadow 0.15s, border-color 0.15s;" onclick="choosePath('A')">
            <div style="font-weight:700; font-size:0.95em; color:#0891B2; margin-bottom:0.4em;">⚡ Quick Update <span style="font-size:0.75em; background:#0891B2; color:#fff; padding:0.15em 0.5em; border-radius:3px; vertical-align:middle; margin-left:0.3em;">Recommended</span></div>
            <div style="font-size:0.82em; color:var(--color-foreground-secondary,#555); line-height:1.5;">
              Flash <b>P4 + Pico firmware</b>, then update the WebUI over WiFi.<br>
              No SD card erase, no MSC mode — fast &amp; easy.
            </div>
          </div>
          <div id="optionCardB" style="flex:1; min-width:220px; border:2px solid #6B7280; border-radius:8px; padding:1em 1.2em; cursor:pointer; background:var(--color-background-secondary,#fafafa); transition: box-shadow 0.15s, border-color 0.15s;" onclick="choosePath('B')">
            <div style="font-weight:700; font-size:0.95em; color:#6B7280; margin-bottom:0.4em;">🗄️ Full SD Card Deploy</div>
            <div style="font-size:0.82em; color:var(--color-foreground-secondary,#555); line-height:1.5;">
              Flash <b>P4 + Pico firmware</b> and re-write the entire SD card image.<br>
              Use for fresh installs or if your SD card data is corrupted.
            </div>
          </div>
        </div>
        <div class="status status-info" id="statPath" style="margin-top:0.8em;">
          Choose an update method above to continue.
        </div>
      </div>

      <!-- ════════════════════════════════════════
           PATH A — Quick Update (firmware only)
           ════════════════════════════════════════ -->
      <div id="pathA" style="display:none;">

      <!-- A·1 — Flash ESP32-P4 -->
      <div class="step-card active-step" id="cardA1">
        <div class="step-hdr"><span class="step-num" style="background:#0891B2;">1</span> Flash ESP32-P4 Firmware</div>
        <div class="step-desc">
          Connect the <b>front JTAG port</b> (USB-C&nbsp;#3) and keep <b>back Port&nbsp;#1</b> connected for power.
          Click <b>Connect</b>, then flash <code id="fwNameA1">dada-tbd.bin</code> to the ESP32-P4.
        </div>
        <div class="btn-row">
          <button id="btnA1Connect" class="btn-primary" disabled>Loading…</button>
          <button id="btnA1Flash" class="btn-success" disabled>Flash TBD-16 Firmware</button>
        </div>
        <div class="progress-wrap" id="progA1"><div class="progress-bar" id="progA1Bar"></div><span class="progress-text" id="progA1Txt">0 %</span></div>
        <div class="status" id="statA1">Loading flash tool…</div>
      </div>

      <!-- A·2 — Flash RP2350 -->
      <div class="step-card" id="cardA2">
        <div class="step-hdr"><span class="step-num" style="background:#0891B2;">2</span> Flash RP2350 (Pico Firmware)</div>
        <div class="step-desc">
          <b>Connect the back USB-C Port&nbsp;#2</b> (closest to the edge).
          You can disconnect the front JTAG cable.
          Put the RP2350 in <b>BOOTSEL mode</b> (hold BOOTSEL + press RESET on the front panel),
          then click <b>Connect</b>.
        </div>
        <div class="btn-row">
          <button id="btnA2Connect" class="btn-primary">Connect</button>
          <button id="btnA2Flash" class="btn-success" disabled>Flash Pico Firmware</button>
          <button id="btnA2Reboot" class="btn-secondary" disabled>Reboot</button>
        </div>
        <div class="progress-wrap" id="progA2"><div class="progress-bar" id="progA2Bar"></div><span class="progress-text" id="progA2Txt">0 %</span></div>
        <div class="status" id="statA2">Put the RP2350 in <b>BOOTSEL mode</b>, then click <b>Connect</b>.</div>
      </div>

      <!-- A·3 — WebUI over WiFi -->
      <div class="step-card" id="cardA3" style="border-color:#0891B2;">
        <div class="step-hdr" style="color:#0891B2;"><span class="step-num" style="background:#0891B2;">3</span> Update WebUI over WiFi</div>
        <div class="step-desc">
          <b>Remove all USB cables</b>, wait 3 seconds, then reconnect a single cable to <b>back Port&nbsp;#1</b>.
          Connect your computer to the TBD-16's WiFi and open <b>http://192.168.4.1/webui-update.html</b>.
          The updater page will check for the latest WebUI version — click <b>Install</b> to update.
        </div>
        <div class="status status-info">After installing, hard-refresh (<b>Ctrl+Shift+R</b> / <b>Cmd+Shift+R</b>) the main page at <b>http://192.168.4.1</b>.</div>
      </div>

      <!-- A·DONE -->
      <div class="complete-card" id="cardDoneA">
        <h3>✓ Quick Update Complete</h3>
        <p>Your TBD-16 has the latest beta firmware.<br>
        Open <b>http://192.168.4.1</b> to use the device.</p>
      </div>

      </div><!-- end pathA -->

      <!-- ════════════════════════════════════════
           PATH B — Full SD Card Deploy
           ════════════════════════════════════════ -->
      <div id="pathB" style="display:none;">

      <!-- B·1 — Flash MSC + Mount SD -->
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

      <!-- B·2 — Write SD Card -->
      <div class="step-card" id="card2">
        <div class="step-hdr"><span class="step-num">2</span> Write SD Card Image</div>
        <div class="step-desc">
          The SD card should now be mounted via <b>back Port&nbsp;#1</b> (look for a <b>"NO NAME"</b> drive).
          Select the mounted drive below. The SD card image will be downloaded from GitHub,
          extracted, and written directly.
        </div>
        <div class="btn-row">
          <button id="btn2Pick" class="btn-primary">Select SD Card Drive</button>
        </div>
        <div class="progress-wrap" id="prog2"><div class="progress-bar" id="prog2Bar"></div><span class="progress-text" id="prog2Txt">0 %</span></div>
        <div class="status" id="stat2">Select the <b>"NO NAME"</b> SD card drive.</div>
        <div class="file-log" id="fileLog"></div>
      </div>

      <!-- B·3 — Switch Back -->
      <div class="step-card" id="card3">
        <div class="step-hdr"><span class="step-num">3</span> Switch Back to Normal Mode</div>
        <div class="step-desc">
          Safely eject the SD card drive from your computer before proceeding.
          Keep <b>back Port&nbsp;#1</b> connected for power, and the <b>front JTAG port</b> connected for serial.
          Click <b>Connect</b> to reconnect via the front JTAG port.
        </div>
        <div class="btn-row">
          <button id="btn3Connect" class="btn-primary">Connect</button>
          <button id="btn3Go" class="btn-success" disabled>Switch to Normal Mode</button>
        </div>
        <div class="status" id="stat3">Click <b>Connect</b> via the <b>front JTAG port</b> to switch back to normal mode.</div>
      </div>

      <!-- B·4 — Flash P4 -->
      <div class="step-card" id="card4">
        <div class="step-hdr"><span class="step-num">4</span> Flash ESP32-P4 Firmware</div>
        <div class="step-desc">
          <b>Power-cycle the device</b>: unplug <b>back Port&nbsp;#1</b>, wait 3 seconds, replug it.
          Then click <b>Connect</b> via the <b>front JTAG port</b>.
        </div>
        <div class="btn-row">
          <button id="btn4Connect" class="btn-primary">Connect</button>
          <button id="btn4Flash" class="btn-success" disabled>Flash TBD-16 Firmware</button>
        </div>
        <div class="progress-wrap" id="prog4"><div class="progress-bar" id="prog4Bar"></div><span class="progress-text" id="prog4Txt">0 %</span></div>
        <div class="status" id="stat4">Click <b>Connect</b> via the <b>front JTAG port</b>, then flash.</div>
      </div>

      <!-- B·5 — Flash Pico -->
      <div class="step-card" id="card5">
        <div class="step-hdr"><span class="step-num">5</span> Flash RP2350 (Pico Firmware)</div>
        <div class="step-desc">
          <b>Connect back USB-C Port&nbsp;#2</b> (closest to the edge).
          Put the RP2350 in <b>BOOTSEL mode</b> (hold BOOTSEL + press RESET),
          then click <b>Connect</b>.
        </div>
        <div class="btn-row">
          <button id="btn5Connect" class="btn-primary">Connect</button>
          <button id="btn5Flash" class="btn-success" disabled>Flash Pico Firmware</button>
          <button id="btn5Reboot" class="btn-secondary" disabled>Reboot</button>
        </div>
        <div class="progress-wrap" id="prog5"><div class="progress-bar" id="prog5Bar"></div><span class="progress-text" id="prog5Txt">0 %</span></div>
        <div class="status" id="stat5">Put the RP2350 in <b>BOOTSEL mode</b>, then click <b>Connect</b>.</div>
      </div>

      <!-- B·DONE -->
      <div class="complete-card" id="cardDone">
        <h3>✓ Pre-release Firmware Setup Complete</h3>
        <p>Your TBD-16 has the SD card image and selected pre-release firmware.<br>
        <b>Remove all USB cables</b>, wait 3 seconds, reconnect via <b>back Port&nbsp;#1</b>,
        then open <b>http://192.168.4.1</b>.</p>
      </div>

      </div><!-- end pathB -->
    </div>

    <script type="module">
    /* ══════════════════════════════════════════════════════════
       Beta Channel (staging) — browser flash tool
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

    /* Path A refs */
    var btnA1Connect = $('btnA1Connect'), btnA1Flash = $('btnA1Flash');
    var progA1 = $('progA1'), progA1Bar = $('progA1Bar'), progA1Txt = $('progA1Txt');
    var statA1 = $('statA1');
    var btnA2Connect = $('btnA2Connect'), btnA2Flash = $('btnA2Flash'), btnA2Reboot = $('btnA2Reboot');
    var progA2 = $('progA2'), progA2Bar = $('progA2Bar'), progA2Txt = $('progA2Txt');
    var statA2 = $('statA2');
    var cardDoneA = $('cardDoneA');

    /* Path B refs */
    var btn1Connect = $('btn1Connect'), btn1Go = $('btn1Go');
    var prog1 = $('prog1'), prog1Bar = $('prog1Bar'), prog1Txt = $('prog1Txt');
    var stat1 = $('stat1'), skip1 = $('skip1');
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
    var RELEASE = null;
    var FIRMWARE_CDN = 'https://dadamachines.github.io/dada-tbd-firmware';
    var CHANNEL = 'staging';
    var TUSB_MSC_URL = null;
    var channelSelect = $('channelSelect');

    /* ══════════════════════════════
       Channel discovery + loading
       ══════════════════════════════ */

    /* Discover available channels by checking CDN catalogs */
    async function discoverChannels() {
      var channels = [{ name: 'staging', label: 'Staging (latest pre-release)' }];

      /* Find feature-test channels from GitHub pre-releases */
      try {
        var resp = await fetch('https://api.github.com/repos/dadamachines/ctag-tbd/releases?per_page=30');
        if (resp.ok) {
          var releases = await resp.json();
          var seen = {};
          releases.forEach(function (r) {
            if (!r.prerelease) return;
            /* feature-test tags look like: feature-test-cool-thing-abc12345 */
            var m = r.tag_name.match(/^(feature-test-[a-z0-9-]+?)-[0-9a-f]{6,}$/);
            if (m && !seen[m[1]]) {
              seen[m[1]] = true;
              channels.push({
                name: m[1],
                label: m[1].replace('feature-test-', 'Feature: ') + ' (' + r.tag_name.slice(-8) + ')'
              });
            }
          });
        }
      } catch (_) { /* GitHub API unavailable — staging only */ }

      return channels;
    }

    /* Load firmware info for a given channel from its releases.json */
    async function loadChannel(channelName) {
      CHANNEL = channelName;

      var catalogUrl = FIRMWARE_CDN + '/' + channelName + '/releases.json';
      var resp = await fetch(catalogUrl);
      if (!resp.ok) throw new Error('Channel catalog not found: ' + channelName);
      var catalog = await resp.json();

      if (!catalog.versions || catalog.versions.length === 0) {
        throw new Error('No versions in ' + channelName + ' channel');
      }

      /* Use the latest version (first in array) */
      var latest = catalog.versions[0];
      var f = latest.files;

      /* Set TUSB_MSC_URL from shared files (fall back to stable) */
      if (catalog.shared && catalog.shared.tusb_msc) {
        TUSB_MSC_URL = FIRMWARE_CDN + '/' + catalog.shared.tusb_msc;
      } else {
        /* Seed from stable channel */
        try {
          var stableResp = await fetch(FIRMWARE_CDN + '/stable/releases.json');
          if (stableResp.ok) {
            var stableCatalog = await stableResp.json();
            if (stableCatalog.shared && stableCatalog.shared.tusb_msc) {
              TUSB_MSC_URL = FIRMWARE_CDN + '/' + stableCatalog.shared.tusb_msc;
            }
          }
        } catch (_) {}
      }

      return {
        tag: latest.tag,
        name: channelName + ' @ ' + latest.tag,
        p4Url: f.unified ? (FIRMWARE_CDN + '/' + f.unified) : null,
        zipUrl: f.sdcard ? (FIRMWARE_CDN + '/' + f.sdcard) : null,
        hashUrl: f.hash ? (FIRMWARE_CDN + '/' + f.hash) : null,
        htmlUrl: 'https://github.com/dadamachines/ctag-tbd/releases/tag/' + latest.tag
      };
    }

    /* Reset all steps when channel changes */
    function resetAllSteps() {
      /* Path A — Connect buttons stay enabled; only action buttons reset */
      btnA1Connect.textContent = 'Connect'; btnA1Connect.disabled = false; btnA1Flash.disabled = true;
      hideProg(progA1); setStat(statA1, 'Click <b>Connect</b> via the <b>front JTAG port</b>.');
      btnA2Connect.disabled = false; btnA2Flash.disabled = true; btnA2Reboot.disabled = true;
      hideProg(progA2); setStat(statA2, 'Put the RP2350 in <b>BOOTSEL mode</b>, then click <b>Connect</b>.');
      cardDoneA.style.display = 'none';

      /* Path B — Connect buttons + SD picker stay enabled */
      btn1Connect.textContent = 'Connect'; btn1Connect.disabled = false; btn1Go.disabled = true;
      btn2Pick.disabled = false;
      btn3Connect.disabled = false; btn3Go.disabled = true;
      btn4Connect.disabled = false; btn4Flash.disabled = true;
      btn5Connect.disabled = false; btn5Flash.disabled = true; btn5Reboot.disabled = true;
      skip1.style.display = 'inline-block';
      hideProg(prog1); hideProg(prog2); hideProg(prog4); hideProg(prog5);
      setStat(stat1, 'Click <b>Connect</b>. Make sure <b>both</b> USB-C cables are connected.');
      setStat(stat2, 'Select the <b>"NO NAME"</b> SD card drive.');
      setStat(stat3, '<b>Safely eject the SD card drive</b>, then click <b>Connect</b> via the front JTAG port.');
      setStat(stat4, '<b>Power-cycle the device</b>, then click <b>Connect</b>.');
      setStat(stat5, 'Put the RP2350 in <b>BOOTSEL mode</b>, then click <b>Connect</b>.');
      fileLog.style.display = 'none'; fileLog.textContent = '';
      cardDone.style.display = 'none';

      /* Hide path chooser until channel loads */
      $('cardPathChooser').style.display = 'none';
      $('pathA').style.display = 'none';
      $('pathB').style.display = 'none';
    }

    /* Activate channel: load firmware info + enable UI */
    async function activateChannel(channelName) {
      resetAllSteps();
      setStat($('statPkg'), 'Loading <b>' + channelName + '</b> channel…');

      try {
        var releaseData = await loadChannel(channelName);
        var picoFwName = await discoverPicoFirmware(releaseData.tag, channelName);

        RELEASE = releaseData;

        var desc = $('cardSelect').querySelector('.step-desc');
        var channelLabel = channelName === 'staging' ? 'staging branch' : channelName + ' branch';
        desc.innerHTML = 'Channel: <b>' + channelName + '</b> — flashing <b>' + RELEASE.tag + '</b>. ' +
          '<a href="' + RELEASE.htmlUrl + '" target="_blank">View release notes \u2192</a>';
        setStat($('statPkg'), '<b>' + RELEASE.name + '</b> — P4 firmware' +
          (picoFwName ? ' + Pico firmware' : '') + ' + SD card image');

        if (!RELEASE.p4Url) {
          setStat($('statPkg'), 'Channel <b>' + channelName + '</b> does not have P4 firmware. Cannot flash.', 'err');
          return;
        }

        /* Show path chooser */
        $('cardPathChooser').style.display = 'block';
      } catch (e) {
        console.error(e);
        setStat($('statPkg'), 'Failed to load channel <b>' + channelName + '</b>: ' + e.message, 'err');
      }
    }

    /* ── Path chooser (global — called from onclick) ── */
    window.choosePath = function (path) {
      var pA = $('pathA'), pB = $('pathB');
      var oA = $('optionCardA'), oB = $('optionCardB');
      var st = $('statPath');
      if (path === 'A') {
        pA.style.display = 'block'; pB.style.display = 'none';
        oA.style.boxShadow = '0 0 0 2px #0891B2'; oA.style.borderColor = '#0891B2';
        oB.style.boxShadow = 'none'; oB.style.borderColor = '#6B7280';
        st.innerHTML = '\u26a1 <b>Quick Update</b> selected \u2014 flash firmware, then update WebUI over WiFi.';
      } else {
        pA.style.display = 'none'; pB.style.display = 'block';
        oB.style.boxShadow = '0 0 0 2px #2563EB'; oB.style.borderColor = '#2563EB';
        oA.style.boxShadow = 'none'; oA.style.borderColor = '#6B7280';
        st.innerHTML = '\ud83d\uddc4\ufe0f <b>Full SD Card Deploy</b> selected \u2014 flash firmware and re-write the entire SD card.';
      }
    };

    /* ══════════════════════════════
       Pico firmware — from CDN repo
       ══════════════════════════════ */
    var PICO_UF2_URL = null;

    async function discoverPicoFirmware(tag, ch) {
      /* Check the CDN for pico firmware availability */
      var name = 'dada-tbd-16-' + tag + '-pico.uf2';
      PICO_UF2_URL = FIRMWARE_CDN + '/' + (ch || CHANNEL) + '/pico/' + name;
      try {
        var resp = await fetch(PICO_UF2_URL, { method: 'HEAD' });
        if (resp.ok) return name;
      } catch (_) {}
      /* Fallback: try stable channel pico (feature-test channels may not have pico) */
      if (ch && ch !== 'staging') {
        PICO_UF2_URL = FIRMWARE_CDN + '/stable/pico/' + name;
        try {
          var resp2 = await fetch(PICO_UF2_URL, { method: 'HEAD' });
          if (resp2.ok) return name;
        } catch (_) {}
      }
      PICO_UF2_URL = null;
      return null;
    }

    /* ══════════════════════════════
       INIT — load tools + fetch release
       ══════════════════════════════ */
    try {
      /* Pre-flight checks */
      if (!('serial' in navigator)) {
        setStat($('statPkg'), 'Your browser does not support <b>WebSerial</b>. Use Chrome, Edge, or Opera on desktop.', 'err');
        throw new Error('WebSerial not supported');
      }

      /* Load tools + discover channels in parallel */
      var [channels, _esp, _pico] = await Promise.all([
        discoverChannels(),
        loadEspTool(),
        loadPicoboot('../_static/picoflash').catch(function (e) {
          console.warn('Picoboot load failed — Pico flash steps will be unavailable:', e);
        })
      ]);

      /* Populate channel dropdown */
      channelSelect.innerHTML = '';
      channels.forEach(function (ch) {
        var opt = document.createElement('option');
        opt.value = ch.name;
        opt.textContent = ch.label;
        channelSelect.appendChild(opt);
      });
      channelSelect.disabled = false;

      /* Handle URL hash — allow linking to ?channel=feature-test-foo */
      var urlParams = new URLSearchParams(window.location.search);
      var requestedChannel = urlParams.get('channel');
      if (requestedChannel && channels.some(function (c) { return c.name === requestedChannel; })) {
        channelSelect.value = requestedChannel;
      }

      /* Channel change handler */
      channelSelect.addEventListener('change', function () {
        if (channelSelect.value) activateChannel(channelSelect.value);
      });

      /* Auto-load the selected channel */
      await activateChannel(channelSelect.value || 'staging');

    } catch (e) {
      if (e.message !== 'WebSerial not supported') {
        setStat($('statPkg'), 'Failed to load: ' + e.message, 'err');
      }
      /* Stop — don't wire up any buttons */
      throw e;
    }

    /* ══════════════════════════════
       PATH A — Step A1: Flash ESP32-P4
       ══════════════════════════════ */
    var ctxA1 = null;

    btnA1Connect.addEventListener('click', async function () {
      try {
        btnA1Connect.disabled = true;
        ctxA1 = await connectP4({
          onStatus: function (msg) { setStat(statA1, msg); }
        });
        btnA1Flash.disabled = false;
        setStat(statA1, 'Connected to <b>' + ctxA1.chip + '</b>. Click <b>Flash TBD-16 Firmware</b>.', 'ok');
      } catch (e) {
        console.error(e);
        setStat(statA1, 'Connection failed: ' + e.message, 'err');
        btnA1Connect.disabled = false;
        await disconnectP4(ctxA1); ctxA1 = null;
      }
    });

    btnA1Flash.addEventListener('click', async function () {
      if (!ctxA1) return;
      btnA1Flash.disabled = true; btnA1Connect.disabled = true;
      try {
        /* unified.bin = bootloader + partition table + OTA data + app at correct offsets */
        await flashP4(ctxA1, RELEASE.p4Url, 0x0, {
          onStatus: function (msg) { setStat(statA1, msg); },
          onProgress: function (pct) { showProg(progA1, progA1Bar, progA1Txt, pct); }
        });
        await resetDevice(ctxA1);
        await disconnectP4(ctxA1); ctxA1 = null;

        setStat(statA1, '✓ ESP32-P4 firmware updated.', 'ok');

        if (PICO_UF2_URL) {
          setStat(statA2, '<b>Connect back USB-C Port&nbsp;#2</b>, put the RP2350 in <b>BOOTSEL mode</b>, then click <b>Connect</b>.');
        } else {
          setStat(statA2, 'No Pico firmware available — skip this step.', 'info');
          cardDoneA.style.display = 'block';
        }
      } catch (e) {
        console.error(e);
        setStat(statA1, 'Flash failed: ' + e.message, 'err');
        btnA1Connect.disabled = false;
        await disconnectP4(ctxA1); ctxA1 = null;
      }
    });

    /* ══════════════════════════════
       PATH A — Step A2: Flash RP2350
       ══════════════════════════════ */
    var ctxA2 = null;

    btnA2Connect.addEventListener('click', async function () {
      try {
        btnA2Connect.disabled = true;
        ctxA2 = await connectRP2350({
          onStatus: function (msg) { setStat(statA2, msg); }
        });
        btnA2Flash.disabled = false;
        setStat(statA2, 'Connected to <b>' + (ctxA2.info.productName || 'RP2350') + '</b>. Click <b>Flash Pico Firmware</b>.', 'ok');
      } catch (e) {
        console.error(e);
        setStat(statA2, 'Connection failed: ' + e.message, 'err');
        btnA2Connect.disabled = false;
        await disconnectRP2350(ctxA2); ctxA2 = null;
      }
    });

    btnA2Flash.addEventListener('click', async function () {
      if (!ctxA2) return;
      btnA2Flash.disabled = true; btnA2Connect.disabled = true;
      try {
        await flashRP2350(ctxA2, PICO_UF2_URL, {
          onStatus: function (msg) { setStat(statA2, msg); },
          onProgress: function (pct) { showProg(progA2, progA2Bar, progA2Txt, pct); }
        });
        btnA2Reboot.disabled = false;
        setStat(statA2, '✓ RP2350 firmware updated. Click <b>Reboot</b> to restart.', 'ok');
      } catch (e) {
        console.error(e);
        setStat(statA2, 'Flash failed: ' + e.message, 'err');
        btnA2Connect.disabled = false;
        await disconnectRP2350(ctxA2); ctxA2 = null;
      }
    });

    btnA2Reboot.addEventListener('click', async function () {
      try {
        btnA2Reboot.disabled = true;
        await rebootRP2350(ctxA2);
        ctxA2 = null;
        hideProg(progA2);
        setStat(statA2, '✓ RP2350 rebooted. Now continue to Step 3 — update WebUI over WiFi.', 'ok');
        cardDoneA.style.display = 'block';
      } catch (e) {
        console.error(e);
        setStat(statA2, 'Reboot failed: ' + e.message, 'err');
        await disconnectRP2350(ctxA2); ctxA2 = null;
      }
    });

    /* ══════════════════════════════
       PATH B — Step 1: Flash MSC + switch OTA
       ══════════════════════════════ */
    var ctxB1 = null;
    var REBOOT_WAIT = 20;

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
        await disconnectP4(ctxB1); ctxB1 = null;
      }
    });

    skip1.addEventListener('click', async function () {
      try {
        skip1.style.display = 'none';
        ctxB1 = await connectP4({
          onStatus: function (msg) { setStat(stat1, msg); }
        });
        setStat(stat1, 'Connected to <b>' + ctxB1.chip + '</b>. Switching to SD card mode…', 'info');
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
        await disconnectP4(ctxB1); ctxB1 = null;
      }
    });

    function startRebootCountdown() {
      var remaining = REBOOT_WAIT;
      hideProg(prog1);
      showProg(prog1, prog1Bar, prog1Txt, 0);
      setStat(stat1,
        '✓ Firmware written &amp; OTA switched. The device should reboot into SD card mode.<br>' +
        '<b>If the SD card drive does not appear within ' + REBOOT_WAIT + ' seconds:</b><br>' +
        '&nbsp;&nbsp;① Press the <b>RESET button</b> on the back, <i>or</i><br>' +
        '&nbsp;&nbsp;② Unplug <b>both</b> cables, wait 3 s, replug them.', 'ok');

      var iv = setInterval(function () {
        remaining--;
        var pct = Math.round((REBOOT_WAIT - remaining) / REBOOT_WAIT * 100);
        showProg(prog1, prog1Bar, prog1Txt, pct);
        prog1Txt.textContent = 'Waiting for reboot: ' + remaining + ' s';
        if (remaining <= 0) {
          clearInterval(iv);
          hideProg(prog1);
          setStat(stat1, '✓ Ready. Look for the <b>"NO NAME"</b> drive.', 'ok');
          setStat(stat2, 'Select the <b>"NO NAME"</b> SD card drive below.');
        }
      }, 1000);
    }

    /* ══════════════════════════════
       PATH B — Step 2: Write SD Card via File System Access API
       ══════════════════════════════ */

    /* Recursively delete macOS resource-fork files (._*) */
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
        setStat(stat2, 'Downloading SD card image…');
        var zipResp = await fetch(RELEASE.zipUrl);
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
            setStat(stat2, 'Downloading… ' + (received / 1024 / 1024).toFixed(1) + ' / ' + (contentLength / 1024 / 1024).toFixed(1) + ' MB');
          }
        }
        var zipBuf = new Uint8Array(received);
        var offset = 0;
        for (var ci = 0; ci < chunks.length; ci++) { zipBuf.set(chunks[ci], offset); offset += chunks[ci].length; }
        chunks = null;
        log('Downloaded ' + (received / 1024 / 1024).toFixed(1) + ' MB');

        /* Download hash */
        var hashResp = await fetch(RELEASE.hashUrl);
        var sdHash = (await hashResp.text()).trim();
        log('Hash: ' + sdHash);

        /* Extract ZIP */
        setStat(stat2, 'Extracting ZIP…');
        var zip = await JSZip.loadAsync(zipBuf);
        zipBuf = null;
        var entries = Object.keys(zip.files);
        var total = entries.length;
        log('ZIP contains ' + total + ' entries');

        /* Write files */
        setStat(stat2, 'Writing files to SD card…');
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
        try {
          var vh = await dirHandle.getFileHandle('.version', { create: true });
          var vw = await vh.createWritable(); await vw.write(sdHash); await vw.close();
          log('FILE .version  (' + sdHash + ')'); written++;
        } catch (ve) { errors++; log('ERR  .version: ' + ve.message); }

        try {
          var hh = await dirHandle.getFileHandle('dada-tbd-sd-hash.txt', { create: true });
          var hw = await hh.createWritable(); await hw.write(sdHash); await hw.close();
          log('FILE dada-tbd-sd-hash.txt  (' + sdHash + ')');
        } catch (he) { log('ERR  dada-tbd-sd-hash.txt: ' + he.message); }

        /* Cleanup macOS files */
        setStat(stat2, 'Cleaning up metadata files…');
        showProg(prog2, prog2Bar, prog2Txt, 97);
        try { await cleanMacOSFiles(dirHandle, log); } catch (_) {}

        zip = null;
        showProg(prog2, prog2Bar, prog2Txt, 100);

        if (errors > 0) {
          setStat(stat2, '⚠ Written <b>' + written + '</b> items with <b>' + errors + '</b> error(s). Check the log above.', 'err');
        } else {
          setStat(stat2, '✓ <b>' + written + '</b> files written. <b>⏏ Please safely eject the drive</b> before proceeding.', 'ok');
        }
        setStat(stat3, '<b>Safely eject the SD card drive</b>, then click <b>Connect</b> via the front JTAG port.');
      } catch (e) {
        console.error(e);
        setStat(stat2, 'Failed: ' + e.message, 'err');
        btn2Pick.disabled = false;
      }
    });

    /* ══════════════════════════════
       PATH B — Step 3: Switch back to normal mode
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

        setStat(stat3, '✓ Switched to normal mode.', 'ok');
        setStat(stat4, '<b>Power-cycle the device</b>: unplug back Port&nbsp;#1, wait 3 s, replug. Then click <b>Connect</b>.');
      } catch (e) {
        console.error(e);
        setStat(stat3, 'Failed: ' + e.message, 'err');
        btn3Connect.disabled = false;
        await disconnectP4(ctxB3); ctxB3 = null;
      }
    });

    /* ══════════════════════════════
       PATH B — Step 4: Flash P4 firmware
       ══════════════════════════════ */
    var ctxB4 = null;

    btn4Connect.addEventListener('click', async function () {
      try {
        btn4Connect.disabled = true;
        ctxB4 = await connectP4({
          onStatus: function (msg) { setStat(stat4, msg); }
        });
        btn4Flash.disabled = false;
        setStat(stat4, 'Connected to <b>' + ctxB4.chip + '</b>. Click <b>Flash TBD-16 Firmware</b>.', 'ok');
      } catch (e) {
        console.error(e);
        setStat(stat4, 'Connection failed: ' + e.message, 'err');
        btn4Connect.disabled = false;
        await disconnectP4(ctxB4); ctxB4 = null;
      }
    });

    btn4Flash.addEventListener('click', async function () {
      if (!ctxB4) return;
      btn4Flash.disabled = true; btn4Connect.disabled = true;
      try {
        /* unified.bin = bootloader + partition table + OTA data + app at correct offsets */
        await flashP4(ctxB4, RELEASE.p4Url, 0x0, {
          onStatus: function (msg) { setStat(stat4, msg); },
          onProgress: function (pct) { showProg(prog4, prog4Bar, prog4Txt, pct); }
        });
        await resetDevice(ctxB4);
        await disconnectP4(ctxB4); ctxB4 = null;

        setStat(stat4, '✓ ESP32-P4 firmware updated. <b>Remove all USB cables</b>, wait 3 s, reconnect via back Port&nbsp;#1.', 'ok');

        if (PICO_UF2_URL) {
          setStat(stat5, '<b>Connect back USB-C Port&nbsp;#2</b>, put RP2350 in <b>BOOTSEL mode</b>, then click <b>Connect</b>.');
        } else {
          setStat(stat5, 'No Pico firmware available — skip this step.', 'info');
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
       PATH B — Step 5: Flash RP2350
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
        setStat(stat5, '✓ RP2350 firmware updated. Click <b>Reboot</b>.', 'ok');
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
        setStat(stat5, '✓ RP2350 rebooted.', 'ok');
        cardDone.style.display = 'block';
      } catch (e) {
        console.error(e);
        setStat(stat5, 'Reboot failed: ' + e.message, 'err');
        await disconnectRP2350(ctxB5); ctxB5 = null;
      }
    });

    </script>

Having trouble? See the `Troubleshooting <50_troubleshooting.html>`_ page.
