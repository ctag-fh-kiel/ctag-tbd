**********************
SD Card Recovery
**********************

Restore your TBD-16's SD card content directly from the browser — no card reader,
no terminal commands, no opening the device.

**What this does:**

1. Flashes the USB Mass Storage firmware to your device (so the SD card mounts via USB)
2. Downloads and extracts the factory SD card image directly onto your device's SD card
3. Switches your device back to normal operation
4. Flashes the latest Possan firmware to the ESP32-P4
5. Flashes the latest Possan firmware to the RP2350

**Hardware setup:**

You need **USB-C cables** connected at different steps:

1. **Front JTAG port** (USB-C #3) → for serial communication (Steps 1, 3, 4)
2. **Back USB-C Port #1** → for the SD card to appear as a USB drive (Steps 1, 2) and for powering the device
3. **Back USB-C Port #2** → for flashing the RP2350 co-processor (Step 5)

.. tip::
   Port #1 is the back USB-C port **closest to the center** of the device.
   Port #2 is the back USB-C port **closest to the edge** of the device.

**Browser:** Chrome, Edge or Opera required (WebSerial + File System Access).

**Time:** 5–10 minutes

.. raw:: html

    <style>
      /* ── Recovery Tool Styles ── */
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
      .sd-recovery .step-card.done {
        opacity: 0.5;
        pointer-events: none;
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

      <!-- ════════ STEP 1 ════════ -->
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

      <!-- ════════ STEP 2 ════════ -->
      <div class="step-card" id="card2" style="opacity:0.4; pointer-events:none;">
        <div class="step-hdr"><span class="step-num">2</span> Restore SD Card Content</div>
        <div class="step-desc">
          The SD card should now be mounted via <b>back Port&nbsp;#1</b> (look for a <b>"NO NAME"</b> drive in Finder).
          Select the mounted drive below. The factory image will be downloaded,
          extracted, and written directly — your SD card will be ready to use.
        </div>
        <div class="btn-row">
          <button id="btn2Pick" class="btn-primary" disabled>Select SD Card Drive</button>
        </div>
        <div class="progress-wrap" id="prog2"><div class="progress-bar" id="prog2Bar"></div><span class="progress-text" id="prog2Txt">0 %</span></div>
        <div class="status" id="stat2">Waiting for Step 1…</div>
        <div class="file-log" id="fileLog"></div>
      </div>

      <!-- ════════ STEP 3 ════════ -->
      <div class="step-card" id="card3" style="opacity:0.4; pointer-events:none;">
        <div class="step-hdr"><span class="step-num">3</span> Switch Back to Normal Mode</div>
        <div class="step-desc">
          Eject the SD card drive from your computer (right-click → Eject in Finder).
          Keep <b>back USB-C Port&nbsp;#1</b> connected for power, and the <b>front JTAG port</b> cable connected for serial.
          Click <b>Connect</b> below to reconnect via the front JTAG port.
          This erases the OTA boot selection so the device boots its normal firmware
          instead of the USB Mass Storage helper.
        </div>
        <div class="btn-row">
          <button id="btn3Connect" class="btn-primary" disabled>Connect</button>
          <button id="btn3Go" class="btn-success" disabled>Switch to Normal Mode</button>
        </div>
        <div class="status" id="stat3">Waiting for Step 2…</div>
      </div>

      <!-- ════════ STEP 4 ════════ -->
      <div class="step-card" id="card4" style="opacity:0.4; pointer-events:none;">
        <div class="step-hdr"><span class="step-num">4</span> Flash ESP32-P4 (Possan Firmware)</div>
        <div class="step-desc">
          First <b>power-cycle the device</b>: unplug the cable from <b>back USB-C Port&nbsp;#1</b>,
          wait 3 seconds, then plug it back in.
          Once the device has rebooted, click <b>Connect</b> below (via the <b>front JTAG port</b>)
          to flash <code>possan-tbd-2026-02-17.bin</code> to the ESP32-P4.
        </div>
        <div class="btn-row">
          <button id="btn4Connect" class="btn-primary" disabled>Connect</button>
          <button id="btn4Flash" class="btn-success" disabled>Flash Possan Firmware</button>
        </div>
        <div class="progress-wrap" id="prog4"><div class="progress-bar" id="prog4Bar"></div><span class="progress-text" id="prog4Txt">0 %</span></div>
        <div class="status" id="stat4">Waiting for Step 3…</div>
      </div>

      <!-- ════════ STEP 5 ════════ -->
      <div class="step-card" id="card5" style="opacity:0.4; pointer-events:none;">
        <div class="step-hdr"><span class="step-num">5</span> Flash RP2350 (Possan Firmware)</div>
        <div class="step-desc">
          <b>Connect the back USB-C Port&nbsp;#2</b> to your computer (the port closest to the edge of the device).
          You can disconnect the front JTAG cable — it is no longer needed.
          Put the RP2350 in <b>BOOTSEL mode</b> (hold BOOTSEL button + press RESET — both on the front panel, next to the JTAG port),
          then click <b>Connect</b> below. This flashes <code>possan-tbd-2026-02-17.uf2</code> to the RP2350 co-processor.
        </div>
        <div class="btn-row">
          <button id="btn5Connect" class="btn-primary" disabled>Connect</button>
          <button id="btn5Flash" class="btn-success" disabled>Flash Possan Firmware</button>
          <button id="btn5Reboot" class="btn-secondary" disabled>Reboot</button>
        </div>
        <div class="progress-wrap" id="prog5"><div class="progress-bar" id="prog5Bar"></div><span class="progress-text" id="prog5Txt">0 %</span></div>
        <div class="status" id="stat5">Waiting for Step 4…</div>
      </div>

      <!-- ════════ DONE ════════ -->
      <div class="complete-card" id="cardDone">
        <h3>✓ SD Card Recovery &amp; Firmware Update Complete</h3>
        <p>Your TBD-16 has a fresh SD card and the latest Possan firmware on both processors.<br>
        <b>Remove all USB cables</b> from the device and wait 3 seconds to fully power-cycle.
        Then reconnect a single USB-C cable to <b>back Port&nbsp;#1</b> (ESP32-P4 — MIDI &amp; USB&nbsp;NCM)
        or <b>back Port&nbsp;#2</b> (RP2350) and open
        <b>http://192.168.4.1/</b> to access the web interface.</p>
      </div>
    </div>

    <script>
    (async function () {
      /* ──────────────────────────────
         DOM refs
         ────────────────────────────── */
      var $ = function (id) { return document.getElementById(id); };

      var card1 = $('card1'), card2 = $('card2'), card3 = $('card3');
      var card4 = $('card4'), card5 = $('card5'), cardDone = $('cardDone');
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

      /* ──────────────────────────────
         Constants
         ────────────────────────────── */
      var TUSB_MSC_URL      = '../_static/firmware/p4/tusb_msc.bin';
      var POSSAN_P4_URL     = '../_static/firmware/p4/possan-tbd-2026-02-17.bin';
      var POSSAN_PICO_URL   = '../_static/firmware/pico/possan-tbd-2026-02-17.uf2';
      var SDCARD_ZIP_URL    = '../_static/sdcard_image/tbd-sd-card.zip';
      var HASH_URL          = '../_static/sdcard_image/tbd-sd-card-hash.txt';
      var OTA_DATA_ADDR     = 0xd000;     /* otadata partition */
      var OTA1_ADDR         = null;       /* detected from device partition table */
      var OTA_DATA_SIZE     = 0x2000;     /* 8 KB              */
      var PT_ADDR           = 0x8000;     /* partition table address */
      var PT_READ_SIZE      = 0xC00;      /* 3 KB — enough for partition table */

      /* ──────────────────────────────
         Helpers
         ────────────────────────────── */
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
      function activateCard(card) {
        card.style.opacity = '1';
        card.style.pointerEvents = 'auto';
        card.classList.add('active-step');
      }
      function markDone(card) {
        card.classList.remove('active-step');
        card.classList.add('done');
      }

      /* binary string helper (8 KB chunks to avoid call-stack overflow) */
      function toBinStr(u8) {
        var parts = [], cs = 8192;
        for (var i = 0; i < u8.length; i += cs)
          parts.push(String.fromCharCode.apply(null, u8.subarray(i, i + cs)));
        return parts.join('');
      }

      /* CRC-32 (same polynomial as ESP-IDF / zlib) */
      var crcTable = null;
      function ensureCrcTable() {
        if (!crcTable) {
          crcTable = new Uint32Array(256);
          for (var n = 0; n < 256; n++) {
            var c = n;
            for (var k = 0; k < 8; k++)
              c = (c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1);
            crcTable[n] = c;
          }
        }
      }

      /* Parse ESP-IDF partition table from binary data.
       * Returns array of {name, type, subtype, offset, size}.
       * See: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/
       *        api-guides/partition-tables.html                               */
      function parsePartitionTable(data) {
        var entries = [];
        for (var i = 0; i < data.length; i += 32) {
          var magic = data[i] | (data[i + 1] << 8);
          if (magic === 0xEBEB) break;       /* end-of-table marker */
          if (magic !== 0xAA50) continue;     /* skip non-entry rows */
          var type    = data[i + 2];
          var subtype = data[i + 3];
          var offset  = (data[i+4] | (data[i+5]<<8) | (data[i+6]<<16) | (data[i+7]<<24)) >>> 0;
          var size    = (data[i+8] | (data[i+9]<<8) | (data[i+10]<<16) | (data[i+11]<<24)) >>> 0;
          var nameBytes = data.subarray(i + 12, i + 28);
          var name = '';
          for (var j = 0; j < nameBytes.length && nameBytes[j] !== 0; j++)
            name += String.fromCharCode(nameBytes[j]);
          entries.push({name:name, type:type, subtype:subtype, offset:offset, size:size});
        }
        return entries;
      }

      /* Read partition table from device flash and find ota_1 offset.
       * Uses esptool-js readFlash() to read 3 KB at 0x8000.            */
      async function detectOta1Address(loader) {
        var ptStr = await loader.readFlash(PT_ADDR, PT_READ_SIZE);
        var ptData = new Uint8Array(ptStr.length);
        for (var i = 0; i < ptStr.length; i++) ptData[i] = ptStr.charCodeAt(i);
        var parts = parsePartitionTable(ptData);
        for (var p = 0; p < parts.length; p++) {
          if (parts[p].name === 'ota_1') {
            return parts[p].offset;
          }
        }
        throw new Error('ota_1 partition not found in device partition table');
      }

      /* Recursively delete macOS resource-fork files (._*) from a
       * FileSystemDirectoryHandle.  macOS creates these automatically
       * when Chrome writes to a FAT32 volume via the File System
       * Access API (for extended-attribute / quarantine storage).       */
      async function cleanMacOSFiles(dirHandle, logFn) {
        var childDirs = [];
        var dotFiles = [];
        for await (var entry of dirHandle.entries()) {
          var eName = entry[0], eHandle = entry[1];
          if (eHandle.kind === 'directory' && !eName.startsWith('.')) {
            childDirs.push(eHandle);
          }
          if (eName.startsWith('._')) {
            dotFiles.push(eName);
          }
        }
        for (var d = 0; d < childDirs.length; d++) {
          await cleanMacOSFiles(childDirs[d], logFn);
        }
        for (var f = 0; f < dotFiles.length; f++) {
          try {
            await dirHandle.removeEntry(dotFiles[f]);
            if (logFn) logFn('DEL  ' + dotFiles[f]);
          } catch (e) { /* ignore — file may already be gone */ }
        }
      }

      /* esp_rom_crc32_le(UINT32_MAX, buf, len) — matches ESP-IDF bootloader.
       * zlib-style: {init XOR 0xFFFFFFFF} → accumulate → {result XOR 0xFFFFFFFF}
       * With init = 0xFFFFFFFF the internal start is 0x00000000.              */
      function espCrc32(buf) {
        ensureCrcTable();
        var crc = 0x00000000;
        for (var i = 0; i < buf.length; i++)
          crc = crcTable[(crc ^ buf[i]) & 0xFF] ^ (crc >>> 8);
        return (crc ^ 0xFFFFFFFF) >>> 0;
      }

      /* Build the 8 KB OTA data blob that selects a given slot.
       *   slot 0 → ota_seq = 1   (boot ota_0)
       *   slot 1 → ota_seq = 2   (boot ota_1 / tusb_msc)
       *   null   → all 0xFF      (erased → boots ota_0)            */
      function buildOtaData(slot) {
        var blob = new Uint8Array(OTA_DATA_SIZE);
        blob.fill(0xFF);
        if (slot === null) return blob;          /* erased state */

        var seq = slot + 1;                      /* ota_seq is 1-based */
        /* esp_ota_select_entry_t  (32 bytes):
             uint32  ota_seq
             uint8   seq_label[20]   (left 0xFF)
             uint32  ota_state       (0xFFFFFFFF = undefined, boots OK)
             uint32  crc             (CRC-32 of ota_seq only — 4 bytes)
           See ESP-IDF bootloader_common_ota_select_crc():
             esp_rom_crc32_le(UINT32_MAX, &s->ota_seq, 4)             */
        var entry = new Uint8Array(32);
        entry.fill(0xFF);
        /* ota_seq  (little-endian) */
        entry[0] = seq & 0xFF;
        entry[1] = (seq >> 8) & 0xFF;
        entry[2] = (seq >> 16) & 0xFF;
        entry[3] = (seq >> 24) & 0xFF;
        /* seq_label[20] stays 0xFF — matches ESP-IDF set_actual_ota_seq() */
        /* ota_state = 0xFFFFFFFF  (already filled) */
        /* CRC-32 of ota_seq field ONLY (4 bytes) */
        var c = espCrc32(entry.subarray(0, 4));
        entry[28] = c & 0xFF;
        entry[29] = (c >> 8) & 0xFF;
        entry[30] = (c >> 16) & 0xFF;
        entry[31] = (c >> 24) & 0xFF;
        /* write entry into sector 0 of the blob */
        blob.set(entry, 0);
        return blob;
      }

      /* ──────────────────────────────
         Pre-flight
         ────────────────────────────── */
      if (!('serial' in navigator)) {
        setStat(stat1, 'Your browser does not support <b>WebSerial</b>. Use Chrome, Edge, or Opera on desktop.', 'err');
        return;
      }
      if (typeof window.showDirectoryPicker !== 'function') {
        setStat(stat1, 'Your browser does not support the <b>File System Access API</b>. Use Chrome or Edge on desktop.', 'err');
        return;
      }

      /* ---- load esptool-js ---- */
      var ESPLoader, Transport;
      try {
        var mod = await import('https://unpkg.com/esptool-js@0.5.7/bundle.js');
        ESPLoader = mod.ESPLoader;
        Transport = mod.Transport;
      } catch (e) {
        setStat(stat1, 'Failed to load flash tool: ' + e.message, 'err');
        return;
      }

      /* ---- load JSZip ---- */
      var JSZip;
      try {
        await new Promise(function (resolve, reject) {
          var s = document.createElement('script');
          s.src = 'https://unpkg.com/jszip@3.10.1/dist/jszip.min.js';
          s.onload = resolve;
          s.onerror = function () { reject(new Error('JSZip load failed')); };
          document.head.appendChild(s);
        });
        JSZip = window.JSZip;
      } catch (e) {
        setStat(stat1, 'Failed to load ZIP library: ' + e.message, 'err');
        return;
      }

      /* ── Ready ── */
      btn1Connect.textContent = 'Connect';
      btn1Connect.disabled = false;
      skip1.style.display = 'inline-block';
      setStat(stat1, 'Click <b>Connect</b> to start. Make sure <b>both</b> USB-C cables are connected: <b>front JTAG port</b> (serial) and <b>back Port&nbsp;#1</b> (SD card drive).');

      /* ══════════════════════════════
         STEP 1 — Flash tusb_msc.bin + switch to ota_1
         ══════════════════════════════ */
      var esp1 = null, trans1 = null, conn1 = false;

      async function cleanup1() {
        conn1 = false;
        if (trans1) { try { await trans1.disconnect(); } catch (_) {} }
        esp1 = null; trans1 = null;
      }

      async function connectStep1() {
        btn1Connect.disabled = true;
        setStat(stat1, 'Requesting serial port…');
        var port = await navigator.serial.requestPort({});
        trans1 = new Transport(port, true);
        var term = { clean:function(){}, writeLine:function(d){console.log(d);}, write:function(d){console.log(d);} };
        esp1 = new ESPLoader({ transport: trans1, baudrate: 460800, terminal: term });
        setStat(stat1, 'Connecting to device…');
        var chip = await esp1.main();
        conn1 = true;

        /* Auto-detect ota_1 address from the device's partition table */
        setStat(stat1, 'Reading partition table…');
        try {
          OTA1_ADDR = await detectOta1Address(esp1);
          console.log('Detected ota_1 at 0x' + OTA1_ADDR.toString(16));
        } catch (e) {
          console.warn('Partition table read failed, using default 0x510000:', e);
          OTA1_ADDR = 0x510000;   /* fallback for ctag-tbd layout */
        }
        return chip;
      }

      /* Full flow: flash tusb_msc + switch OTA */
      async function flashAndSwitch() {
        /* 1a — download tusb_msc.bin */
        setStat(stat1, 'Downloading tusb_msc.bin…');
        var resp = await fetch(TUSB_MSC_URL);
        if (!resp.ok) throw new Error('Download failed: ' + resp.statusText);
        var fw = new Uint8Array(await resp.arrayBuffer());
        var sizeMB = (fw.length / 1024 / 1024).toFixed(1);

        /* 1b — flash to ota_1 partition (auto-detected address) */
        setStat(stat1, 'Flashing tusb_msc.bin (' + sizeMB + ' MB) to ota_1 @ 0x' + OTA1_ADDR.toString(16) + ' — do not unplug…');
        await esp1.writeFlash({
          fileArray: [{ data: toBinStr(fw), address: OTA1_ADDR }],
          flashSize: '16MB', flashMode: 'dio', flashFreq: '80m',
          eraseAll: false, compress: true,
          reportProgress: function (_, written, total) {
            showProg(prog1, prog1Bar, prog1Txt, Math.round(written / total * 100));
          }
        });
        showProg(prog1, prog1Bar, prog1Txt, 100);

        /* 1c — write OTA data to select ota_1 */
        setStat(stat1, 'Switching boot partition to ota_1…');
        var otaBlob = buildOtaData(1);
        await esp1.writeFlash({
          fileArray: [{ data: toBinStr(otaBlob), address: OTA_DATA_ADDR }],
          flashSize: '16MB', flashMode: 'dio', flashFreq: '80m',
          eraseAll: false, compress: true
        });

        /* 1d — attempt hard reset, then release the serial port.
         *      The USB-JTAG reset via WebSerial is unreliable on
         *      ESP32-P4, so we always show manual-reset instructions
         *      as a fallback. Releasing the port is essential: holding
         *      the serial connection open can block USB re-enumeration
         *      after the chip reboots.                                  */
        setStat(stat1, 'Attempting device reset…');
        try { await esp1.after('hard_reset'); } catch (e) {
          console.warn('Software reset failed (expected on some setups):', e);
        }
        await cleanup1();
      }

      /* Switch OTA only (skip flashing tusb_msc) */
      async function switchOnlyToOta1() {
        setStat(stat1, 'Switching boot partition to ota_1…');
        var otaBlob = buildOtaData(1);
        await esp1.writeFlash({
          fileArray: [{ data: toBinStr(otaBlob), address: OTA_DATA_ADDR }],
          flashSize: '16MB', flashMode: 'dio', flashFreq: '80m',
          eraseAll: false, compress: true
        });
        setStat(stat1, 'Attempting device reset…');
        try { await esp1.after('hard_reset'); } catch (e) {
          console.warn('Software reset failed (expected on some setups):', e);
        }
        await cleanup1();
      }

      /* ── Countdown after flashing: give the device time to reboot
       *    and the SD card time to enumerate as USB Mass Storage.
       *    Show manual-reset instructions in case the software
       *    reset didn't work.                                      ── */
      var REBOOT_WAIT = 20;          /* seconds */

      function startRebootCountdown() {
        var remaining = REBOOT_WAIT;
        hideProg(prog1);
        showProg(prog1, prog1Bar, prog1Txt, 0);
        setStat(stat1,
          '✓ Firmware written &amp; OTA switched. The device should now reboot into SD card mode.<br>' +
          '<b>If the SD card drive does not appear within ' + REBOOT_WAIT + ' seconds:</b><br>' +
          '&nbsp;&nbsp;① Press the <b>RESET button</b> on the back (between USB-C Port&nbsp;#1 and MIDI&nbsp;OUT&nbsp;2), <i>or</i><br>' +
          '&nbsp;&nbsp;② Unplug <b>both</b> USB cables, wait 3 s, replug them.<br>' +
          '<small>The drive should appear as <b>"NO NAME"</b> in Finder (via back Port&nbsp;#1).</small>',
          'ok');

        var iv = setInterval(function () {
          remaining--;
          var pct = Math.round((REBOOT_WAIT - remaining) / REBOOT_WAIT * 100);
          showProg(prog1, prog1Bar, prog1Txt, pct);
          prog1Txt.textContent = 'Waiting for reboot: ' + remaining + ' s';
          if (remaining <= 0) {
            clearInterval(iv);
            hideProg(prog1);
            finishStep1();
          }
        }, 1000);
      }

      function finishStep1() {
        markDone(card1);
        activateCard(card2);
        btn2Pick.disabled = false;
        setStat(stat1,
          '✓ Ready. Look for the <b>"NO NAME"</b> drive in Finder (mounted via back Port&nbsp;#1).<br>' +
          '<small>If no drive appeared: press <b>RESET</b> on the back (between USB-C Port&nbsp;#1 and MIDI&nbsp;OUT&nbsp;2) or power-cycle and wait 15 s.</small>',
          'ok');
        setStat(stat2,
          'Select the <b>"NO NAME"</b> SD card drive below.<br>' +
          '<small>If no drive is visible in Finder: press the <b>RESET</b> button on the back (between USB-C Port&nbsp;#1 and MIDI&nbsp;OUT&nbsp;2), ' +
          'or unplug both USB cables → wait 3 s → replug. The drive appears via <b>back Port&nbsp;#1</b>.</small>');
      }

      btn1Connect.addEventListener('click', async function () {
        try {
          var chip = await connectStep1();
          btn1Go.disabled = false;
          setStat(stat1, 'Connected to <b>' + chip + '</b>. Click <b>Flash &amp; Switch</b> to proceed.', 'ok');
        } catch (e) {
          console.error(e);
          setStat(stat1, 'Connection failed: ' + e.message, 'err');
          btn1Connect.disabled = false;
          await cleanup1();
        }
      });

      btn1Go.addEventListener('click', async function () {
        if (!conn1) return;
        btn1Go.disabled = true; btn1Connect.disabled = true;
        try {
          await flashAndSwitch();
          startRebootCountdown();
        } catch (e) {
          console.error(e);
          setStat(stat1, 'Failed: ' + e.message, 'err');
          btn1Connect.disabled = false;
          await cleanup1();
        }
      });

      /* Skip link — already have tusb_msc, just switch OTA */
      skip1.addEventListener('click', async function () {
        try {
          skip1.style.display = 'none';
          var chip = await connectStep1();
          btn1Go.disabled = true;
          setStat(stat1, 'Connected to <b>' + chip + '</b>. Switching to SD card mode…', 'info');
          await switchOnlyToOta1();
          startRebootCountdown();
        } catch (e) {
          console.error(e);
          setStat(stat1, 'Failed: ' + e.message, 'err');
          btn1Connect.disabled = false;
          skip1.style.display = 'inline-block';
          await cleanup1();
        }
      });

      /* ══════════════════════════════
         STEP 2 — Restore SD card via File System Access API
         ══════════════════════════════ */
      btn2Pick.addEventListener('click', async function () {
        var dirHandle;
        try {
          dirHandle = await window.showDirectoryPicker({ mode: 'readwrite' });
        } catch (e) {
          if (e.name === 'AbortError') return;   /* user cancelled */
          setStat(stat2, 'Could not access directory: ' + e.message, 'err');
          return;
        }

        btn2Pick.disabled = true;
        fileLog.style.display = 'block';
        fileLog.textContent = '';

        function log(msg) {
          fileLog.textContent += msg + '\n';
          fileLog.scrollTop = fileLog.scrollHeight;
        }

        try {
          /* 2a — download the SD card ZIP */
          setStat(stat2, 'Downloading SD card image…');
          var zipResp = await fetch(SDCARD_ZIP_URL);
          if (!zipResp.ok) throw new Error('ZIP download failed: ' + zipResp.statusText);
          var contentLength = parseInt(zipResp.headers.get('Content-Length') || '0', 10);
          var reader = zipResp.body.getReader();
          var chunks = [];
          var received = 0;
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
          for (var ci = 0; ci < chunks.length; ci++) {
            zipBuf.set(chunks[ci], offset);
            offset += chunks[ci].length;
          }
          chunks = null;  /* free download chunk references */
          log('Downloaded ' + (received / 1024 / 1024).toFixed(1) + ' MB');

          /* 2b — download the hash */
          var hashResp = await fetch(HASH_URL);
          var sdHash = (await hashResp.text()).trim();
          log('Hash: ' + sdHash);

          /* 2c — extract ZIP */
          setStat(stat2, 'Extracting ZIP…');
          var zip = await JSZip.loadAsync(zipBuf);
          zipBuf = null;  /* free raw ZIP buffer — JSZip has its own copy */
          var entries = Object.keys(zip.files);
          var total = entries.length;
          log('ZIP contains ' + total + ' entries');

          /* 2d — write each file/folder to the SD card directory handle */
          setStat(stat2, 'Writing files to SD card…');
          var written = 0;
          var errors = 0;

          /* Helper: get (or create) a nested directory handle */
          async function getDir(root, pathParts) {
            var cur = root;
            for (var p = 0; p < pathParts.length; p++) {
              cur = await cur.getDirectoryHandle(pathParts[p], { create: true });
            }
            return cur;
          }

          for (var ei = 0; ei < entries.length; ei++) {
            var name = entries[ei];
            var zipEntry = zip.files[name];
            var pct = 50 + Math.round((ei + 1) / total * 45);
            showProg(prog2, prog2Bar, prog2Txt, pct);

            /* skip macOS resource forks, ._ files, and hidden zip metadata */
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
                /* get parent directory, then write file */
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
                data = null;  /* free extracted data immediately */
              }
              written++;
            } catch (fe) {
              errors++;
              log('ERR  ' + name + ': ' + fe.message);
            }

            /* Yield to the browser every 10 files to prevent UI freeze / crash */
            if (ei % 10 === 0) {
              await new Promise(function (r) { setTimeout(r, 0); });
            }
          }

          /* 2e — write .version file */
          try {
            var versionHandle = await dirHandle.getFileHandle('.version', { create: true });
            var vw = await versionHandle.createWritable();
            await vw.write(sdHash);
            await vw.close();
            log('FILE .version  (' + sdHash + ')');
            written++;
          } catch (ve) {
            errors++;
            log('ERR  .version: ' + ve.message);
          }

          /* 2e-b — write tbd-sd-card-hash.txt */
          try {
            var hashHandle = await dirHandle.getFileHandle('tbd-sd-card-hash.txt', { create: true });
            var hw = await hashHandle.createWritable();
            await hw.write(sdHash);
            await hw.close();
            log('FILE tbd-sd-card-hash.txt  (' + sdHash + ')');
          } catch (he) {
            log('ERR  tbd-sd-card-hash.txt: ' + he.message);
          }

          /* 2f — clean up macOS resource-fork files (._*) that macOS
           *       auto-creates when Chrome writes to a FAT32 volume.  */
          setStat(stat2, 'Cleaning up macOS metadata files…');
          showProg(prog2, prog2Bar, prog2Txt, 97);
          try {
            await cleanMacOSFiles(dirHandle, log);
          } catch (ce) {
            log('WARN cleanup: ' + ce.message);
          }

          zip = null;  /* free JSZip instance */
          showProg(prog2, prog2Bar, prog2Txt, 100);

          if (errors > 0) {
            setStat(stat2, '⚠ Written <b>' + written + '</b> items with <b>' + errors + '</b> error(s). Check the log above.', 'err');
          } else {
            setStat(stat2, '✓ <b>' + written + '</b> files written &amp; cleaned. SD card restored! <b>Eject the drive</b> from Finder, then proceed to Step 3.', 'ok');
          }
          markDone(card2);
          activateCard(card3);
          btn3Connect.disabled = false;
          setStat(stat3, '<b>Eject the SD card drive</b> from Finder first, then click <b>Connect</b> to reconnect via the front JTAG port.');

        } catch (e) {
          console.error(e);
          setStat(stat2, 'Failed: ' + e.message, 'err');
          btn2Pick.disabled = false;
        }
      });

      /* ══════════════════════════════
         STEP 3 — Erase OTA data → boot ota_0
         ══════════════════════════════ */
      var esp3 = null, trans3 = null, conn3 = false;

      async function cleanup3() {
        conn3 = false;
        if (trans3) { try { await trans3.disconnect(); } catch (_) {} }
        esp3 = null; trans3 = null;
      }

      btn3Connect.addEventListener('click', async function () {
        try {
          btn3Connect.disabled = true;
          setStat(stat3, 'Requesting serial port…');
          var port = await navigator.serial.requestPort({});
          trans3 = new Transport(port, true);
          var term = { clean:function(){}, writeLine:function(d){console.log(d);}, write:function(d){console.log(d);} };
          esp3 = new ESPLoader({ transport: trans3, baudrate: 460800, terminal: term });
          setStat(stat3, 'Connecting…');
          var chip = await esp3.main();
          conn3 = true;
          btn3Go.disabled = false;
          setStat(stat3, 'Connected to <b>' + chip + '</b>. Click <b>Switch to Normal Mode</b>.', 'ok');
        } catch (e) {
          console.error(e);
          setStat(stat3, 'Connection failed: ' + e.message, 'err');
          btn3Connect.disabled = false;
          await cleanup3();
        }
      });

      btn3Go.addEventListener('click', async function () {
        if (!conn3) return;
        btn3Go.disabled = true; btn3Connect.disabled = true;
        try {
          setStat(stat3, 'Erasing OTA data (selects normal firmware)…');
          var erased = buildOtaData(null);   /* 8 KB of 0xFF */
          await esp3.writeFlash({
            fileArray: [{ data: toBinStr(erased), address: OTA_DATA_ADDR }],
            flashSize: '16MB', flashMode: 'dio', flashFreq: '80m',
            eraseAll: false, compress: true
          });

          setStat(stat3, 'Resetting device…', 'ok');
          try { await esp3.after('hard_reset'); } catch (e) {
            console.warn('Software reset failed (expected on some setups):', e);
          }
          await cleanup3();

          markDone(card3);
          activateCard(card4);
          btn4Connect.disabled = false;
          setStat(stat4, '<b>Power-cycle the device</b>: unplug the cable from back USB-C Port&nbsp;#1, wait 3 s, replug it. Then click <b>Connect</b> to flash <code>possan-tbd-2026-02-17.bin</code>.');
        } catch (e) {
          console.error(e);
          setStat(stat3, 'Failed: ' + e.message, 'err');
          btn3Connect.disabled = false;
          await cleanup3();
        }
      });

      /* ══════════════════════════════
         STEP 4 — Flash ESP32-P4 with Possan firmware
         ══════════════════════════════ */
      var esp4 = null, trans4 = null, conn4 = false;

      async function cleanup4() {
        conn4 = false;
        if (trans4) { try { await trans4.disconnect(); } catch (_) {} }
        esp4 = null; trans4 = null;
      }

      btn4Connect.addEventListener('click', async function () {
        try {
          btn4Connect.disabled = true;
          setStat(stat4, 'Requesting serial port…');
          var port = await navigator.serial.requestPort({});
          trans4 = new Transport(port, true);
          var term = { clean:function(){}, writeLine:function(d){console.log(d);}, write:function(d){console.log(d);} };
          esp4 = new ESPLoader({ transport: trans4, baudrate: 460800, terminal: term });
          setStat(stat4, 'Connecting…');
          var chip = await esp4.main();
          conn4 = true;
          btn4Flash.disabled = false;
          setStat(stat4, 'Connected to <b>' + chip + '</b>. Click <b>Flash Possan Firmware</b>.', 'ok');
        } catch (e) {
          console.error(e);
          setStat(stat4, 'Connection failed: ' + e.message, 'err');
          btn4Connect.disabled = false;
          await cleanup4();
        }
      });

      btn4Flash.addEventListener('click', async function () {
        if (!conn4 || !esp4) return;
        btn4Flash.disabled = true; btn4Connect.disabled = true;
        try {
          setStat(stat4, 'Downloading Possan firmware…');
          var resp = await fetch(POSSAN_P4_URL);
          if (!resp.ok) throw new Error('Download failed: ' + resp.statusText);
          var fw = new Uint8Array(await resp.arrayBuffer());
          var sizeMB = (fw.length / 1024 / 1024).toFixed(1);

          setStat(stat4, 'Flashing <code>possan-tbd-2026-02-17.bin</code> (' + sizeMB + ' MB) — do not unplug…');
          await esp4.writeFlash({
            fileArray: [{ data: toBinStr(fw), address: 0x0 }],
            flashSize: '16MB', flashMode: 'dio', flashFreq: '80m',
            eraseAll: false, compress: true,
            reportProgress: function (_, written, total) {
              showProg(prog4, prog4Bar, prog4Txt, Math.round(written / total * 100));
            }
          });
          showProg(prog4, prog4Bar, prog4Txt, 100);

          setStat(stat4, 'Resetting device…', 'ok');
          try { await esp4.after('hard_reset'); } catch (e) {
            console.warn('Software reset failed:', e);
          }
          await cleanup4();

          setStat(stat4, '✓ ESP32-P4 firmware updated. Proceed to Step 5.', 'ok');
          markDone(card4);
          activateCard(card5);
          btn5Connect.disabled = false;
          setStat(stat5, 'Connect <b>back USB-C Port&nbsp;#2</b> (you can disconnect the front JTAG cable). Put the RP2350 in <b>BOOTSEL mode</b> (hold BOOTSEL + press RESET — both on the front panel), then click <b>Connect</b>.');
        } catch (e) {
          console.error(e);
          setStat(stat4, 'Flash failed: ' + e.message, 'err');
          btn4Connect.disabled = false;
          await cleanup4();
        }
      });

      /* ══════════════════════════════
         STEP 5 — Flash RP2350 with Possan firmware (WebUSB / Picoboot)
         ══════════════════════════════ */
      var Picoboot = null, uf2ToFlashBuffer = null;
      var pico5 = null, picoConn5 = null;

      /* Load Picoboot modules dynamically (ES module import works in async) */
      try {
        var picoMod = await import('../_static/picoflash/pkg/index.js');
        Picoboot = picoMod.Picoboot;
        var uf2Mod = await import('../_static/picoflash/js/uf2.js');
        uf2ToFlashBuffer = uf2Mod.uf2ToFlashBuffer;
      } catch (e) {
        console.warn('Picoboot module load failed — Step 5 will be unavailable:', e);
      }

      async function cleanup5() {
        try { if (pico5) await pico5.disconnect(); } catch (_) {}
        pico5 = null; picoConn5 = null;
      }

      btn5Connect.addEventListener('click', async function () {
        if (!Picoboot) {
          setStat(stat5, 'Picoboot module failed to load. Try reloading the page.', 'err');
          return;
        }
        try {
          btn5Connect.disabled = true;
          setStat(stat5, 'Waiting for device selection… choose <b>RP2350 Boot</b>');
          pico5 = await Picoboot.requestDevice();
          setStat(stat5, 'Connecting…');
          picoConn5 = await pico5.connect();
          await picoConn5.resetInterface();
          btn5Flash.disabled = false;
          var info = pico5.getUsbDeviceInfo();
          setStat(stat5, 'Connected to <b>' + (info.productName || 'RP2350') + '</b>. Click <b>Flash Possan Firmware</b>.', 'ok');
        } catch (e) {
          console.error(e);
          setStat(stat5, 'Connection failed: ' + e.message, 'err');
          btn5Connect.disabled = false;
          await cleanup5();
        }
      });

      btn5Flash.addEventListener('click', async function () {
        if (!pico5 || !uf2ToFlashBuffer) return;
        btn5Flash.disabled = true; btn5Connect.disabled = true;
        try {
          setStat(stat5, 'Downloading <code>possan-tbd-2026-02-17.uf2</code>…');
          showProg(prog5, prog5Bar, prog5Txt, 10);
          var resp = await fetch(POSSAN_PICO_URL);
          if (!resp.ok) throw new Error('Download failed: ' + resp.statusText);
          var uf2Data = new Uint8Array(await resp.arrayBuffer());
          showProg(prog5, prog5Bar, prog5Txt, 25);

          setStat(stat5, 'Parsing UF2 file…');
          var parsed = uf2ToFlashBuffer(uf2Data);
          var sizeKB = (parsed.data.length / 1024).toFixed(0);

          setStat(stat5, 'Erasing &amp; writing ' + sizeKB + ' KB…');
          showProg(prog5, prog5Bar, prog5Txt, 35);
          await pico5.flashEraseAndWrite(parsed.address, parsed.data);
          showProg(prog5, prog5Bar, prog5Txt, 100);

          btn5Reboot.disabled = false;
          setStat(stat5, '✓ RP2350 firmware updated (<code>possan-tbd-2026-02-17.uf2</code>). Click <b>Reboot</b> to restart the device.', 'ok');
        } catch (e) {
          console.error(e);
          setStat(stat5, 'Flash failed: ' + e.message, 'err');
          btn5Connect.disabled = false;
          await cleanup5();
        }
      });

      btn5Reboot.addEventListener('click', async function () {
        try {
          btn5Reboot.disabled = true;
          setStat(stat5, 'Rebooting device…');
          try { await picoConn5.reboot(100); } catch (e) {
            console.warn('Reboot command error (may be expected):', e.message);
          }
          await cleanup5();
          hideProg(prog5);

          setStat(stat5, '✓ RP2350 rebooted. <b>Remove all USB cables</b>, wait 3 s, then reconnect via a back port.', 'ok');
          markDone(card5);
          cardDone.style.display = 'block';
        } catch (e) {
          console.error(e);
          setStat(stat5, 'Reboot failed: ' + e.message, 'err');
          await cleanup5();
        }
      });

    })();
    </script>


Troubleshooting
===============

**"Your browser does not support WebSerial"**

  Use Chrome, Edge, or Opera on desktop. Safari and Firefox do not support WebSerial.

**"Your browser does not support the File System Access API"**

  Use Chrome or Edge on desktop. Firefox and Safari do not support this API.
  The File System Access API is required to write files to the mounted SD card.

**Serial port request fails**

  - Make sure the **front JTAG port** cable is connected to your computer
  - Unplug the USB cable and replug it
  - Try a different USB cable
  - Close any terminals or serial monitors that may hold the port open
  - Restart your browser

**SD card doesn't mount after Step 1**

  The most common cause is that the device didn't actually reboot after flashing.
  The software reset via WebSerial is unreliable on ESP32-P4 — you may need a
  manual reset.

  1. **Press the RESET button** on the back of the device (between USB-C Port #1 and MIDI OUT 2)
  2. Wait 15 seconds for the SD card drive to appear
  3. If still nothing: **unplug both USB cables**, wait 3 seconds, replug them

  Also check:

  - Make sure a USB-C cable is connected from **back Port #1** to your computer
  - Port #1 is the back USB-C port closest to the center of the device
  - Check Finder / your file manager for a new drive (typically named ``NO NAME``)
  - If still no drive: go back to Step 1 and try again

**"Permission denied" when writing files**

  When Chrome asks for permission to write to the selected directory, click **Allow**.
  If you accidentally denied, close the prompt and click "Select SD Card Drive" again.

**Step 3 connection fails (device in MSC mode)**

  When the device is in USB Mass Storage mode, the serial port may not be available.
  Make sure you **eject the drive first** (right-click → Eject in Finder),
  then try connecting via the **front JTAG port**. If the serial port
  still doesn't appear, briefly unplug and replug the front USB cable.
