*********************
ESP32-P4 Device Flasher
*********************

Flash Device From Browser
=========================

Select your firmware, connect your device, and flash directly from the browser.

**Hardware setup:**

1. Connect a USB cable to the **JTAG USB-C port on the front** of the device (data connection).
2. Connect a second USB cable to one of the **two USB-C ports on the back** (power).

**Browser:** Chrome, Edge or Opera required (WebSerial).

.. raw:: html

    <style>
      .esp-flasher {
        border: 1px solid var(--color-background-border, #d1d5db);
        border-radius: 8px;
        padding: 1.5em 1.8em;
        margin: 1em 0 1.5em;
        background: var(--color-background-secondary, #fafafa);
        max-width: 480px;
      }
      .esp-flasher label {
        display: block;
        font-weight: 600;
        margin-bottom: 0.35em;
        font-size: 0.95em;
        color: var(--color-foreground-secondary, #555);
      }
      .esp-flasher select {
        width: 100%;
        padding: 0.5em 0.7em;
        border-radius: 4px;
        border: 1px solid var(--color-background-border, #ccc);
        margin-bottom: 1em;
        font-size: 0.95em;
        background: var(--color-background-primary, #fff);
        color: var(--color-foreground-primary, #222);
      }
      .esp-flasher .btn-row {
        display: flex;
        gap: 0.5em;
        flex-wrap: wrap;
      }
      .esp-flasher button {
        padding: 0.5em 1.2em;
        border: none;
        border-radius: 4px;
        font-size: 0.9em;
        font-weight: 600;
        cursor: pointer;
        color: #fff;
        transition: opacity 0.15s;
      }
      .esp-flasher button:disabled {
        opacity: 0.4;
        cursor: not-allowed;
      }
      .esp-flasher button:not(:disabled):hover {
        opacity: 0.85;
      }
      .esp-flasher .btn-connect { background: #2563EB; }
      .esp-flasher .btn-flash { background: #16A34A; }
      .esp-flasher .btn-disconnect { background: #6B7280; }
      .esp-flasher .progress-wrap {
        margin-top: 0.8em;
        background: #E5E7EB;
        border-radius: 4px;
        overflow: hidden;
        height: 22px;
        position: relative;
      }
      .esp-flasher .progress-bar {
        height: 100%;
        background: #2563EB;
        width: 0%;
        transition: width 0.15s;
      }
      .esp-flasher .progress-text {
        position: absolute;
        top: 0; left: 0; right: 0;
        text-align: center;
        line-height: 22px;
        font-size: 0.8em;
        font-weight: 600;
        color: #374151;
      }
      .esp-flasher .status-box {
        padding: 0.6em 0.9em;
        border-radius: 4px;
        font-size: 0.88em;
        min-height: 1.3em;
        margin-top: 0.8em;
        word-break: break-word;
        background: #F3F4F6;
        color: #374151;
      }
      .esp-flasher .status-success {
        background: #DEF7EC;
        color: #065F46;
      }
      .esp-flasher .status-error {
        background: #FEE2E2;
        color: #991B1B;
      }
    </style>

    <script src="https://cdnjs.cloudflare.com/ajax/libs/crypto-js/4.2.0/crypto-js.min.js" async></script>

    <div class="esp-flasher" id="espFlasher">
      <label for="espFirmwareSelect">Firmware</label>
      <select id="espFirmwareSelect">
        <option value="ctag" selected>CTAG TBD (Development) — 2026-02-11</option>
        <option value="possan">Possan TBD (Experimental) — 2026-02-14</option>
      </select>

      <div class="btn-row">
        <button id="btnConnect" class="btn-connect" disabled>Loading…</button>
        <button id="btnFlash" class="btn-flash" disabled>Flash</button>
        <button id="btnDisconnect" class="btn-disconnect" disabled>Disconnect</button>
      </div>

      <div class="progress-wrap" id="progressWrap" style="display:none;">
        <div class="progress-bar" id="progressBar"></div>
        <span class="progress-text" id="progressText">0 %</span>
      </div>

      <div class="status-box" id="statusBox">Loading flash tool…</div>
    </div>

    <script>
      /* ESP32-P4 Browser Flasher — powered by esptool-js (no esp-web-tools)
       *
       * Key difference: we call writeFlash() with eraseAll:false so only the
       * sectors being written are erased.  The old esp-web-tools wrapper forced
       * a full 16 MB flash erase which overwhelmed Chrome's WebSerial thread
       * and crashed the browser.
       */
      (async function () {
        var btnConnect    = document.getElementById('btnConnect');
        var btnFlash      = document.getElementById('btnFlash');
        var btnDisconnect = document.getElementById('btnDisconnect');
        var sel           = document.getElementById('espFirmwareSelect');
        var progressWrap  = document.getElementById('progressWrap');
        var progressBar   = document.getElementById('progressBar');
        var progressText  = document.getElementById('progressText');
        var statusBox     = document.getElementById('statusBox');

        /* ---- helpers ---- */
        function setStatus(msg, type) {
          statusBox.innerHTML = msg;
          statusBox.className = 'status-box' + (type ? ' status-' + type : '');
        }
        function setProgress(pct) {
          progressWrap.style.display = 'block';
          progressBar.style.width = pct + '%';
          progressText.textContent = pct + ' %';
        }
        function resetProgress() {
          progressWrap.style.display = 'none';
          progressBar.style.width = '0%';
          progressText.textContent = '0 %';
        }

        /* ---- pre-flight checks ---- */
        if (!('serial' in navigator)) {
          setStatus('Your browser does not support WebSerial. Please use <b>Chrome</b>, <b>Edge</b> or <b>Opera</b> on desktop.', 'error');
          return;
        }

        /* ---- load esptool-js from CDN ---- */
        /* Use the official bundle from unpkg (built by rollup, format: es).
         * Do NOT use esm.sh — it injects an atob-browser polyfill that breaks
         * the base64-encoded chip stub loaders inside esptool-js.  The unpkg
         * bundle resolves atob-lite to native browser atob() which works. */
        var ESPLoader, Transport;
        try {
          var mod = await import('https://unpkg.com/esptool-js@0.5.7/bundle.js');
          ESPLoader = mod.ESPLoader;
          Transport = mod.Transport;
        } catch (e) {
          setStatus('Failed to load flash tool: ' + e.message, 'error');
          return;
        }

        /* ---- ready ---- */
        btnConnect.textContent = 'Connect';
        btnConnect.disabled = false;
        setStatus('Select a firmware, then click <b>Connect</b>.');

        var FIRMWARE = {
          ctag:   { url: '../_static/firmware/p4/ctag-tbd-2026-02-11.bin',   name: 'CTAG TBD' },
          possan: { url: '../_static/firmware/p4/possan-tbd-2026-02-14.bin', name: 'Possan TBD' }
        };

        var device    = null;
        var transport = null;
        var esploader = null;
        var connected = false;

        async function cleanup() {
          connected = false;
          if (transport) { try { await transport.disconnect(); } catch (_) {} }
          device = null;  transport = null;  esploader = null;
        }

        /* ---- CONNECT ---- */
        btnConnect.addEventListener('click', async function () {
          try {
            btnConnect.disabled = true;
            setStatus('Requesting serial port…');

            device    = await navigator.serial.requestPort({});
            transport = new Transport(device, true);

            var terminal = {
              clean:     function () {},
              writeLine: function (d) { console.log(d); },
              write:     function (d) { console.log(d); }
            };
            esploader = new ESPLoader({
              transport: transport,
              baudrate:  460800,
              terminal:  terminal
            });

            setStatus('Connecting…');
            var chip = await esploader.main();
            connected = true;

            btnFlash.disabled      = false;
            btnDisconnect.disabled = false;
            sel.disabled           = true;
            setStatus('Connected to <b>' + chip + '</b>. Click <b>Flash</b> to program.', 'success');
          } catch (e) {
            console.error(e);
            setStatus('Connection failed: ' + e.message, 'error');
            btnConnect.disabled = false;
            await cleanup();
          }
        });

        /* ---- FLASH ---- */
        btnFlash.addEventListener('click', async function () {
          if (!connected || !esploader) return;
          try {
            btnFlash.disabled      = true;
            btnDisconnect.disabled = true;
            btnConnect.disabled    = true;

            var fw = FIRMWARE[sel.value];
            setStatus('Downloading <b>' + fw.name + '</b> firmware…');

            var resp = await fetch(fw.url);
            if (!resp.ok) throw new Error('Download failed: ' + resp.statusText);
            var data = new Uint8Array(await resp.arrayBuffer());

            /* esptool-js v0.5.7 expects fileArray[].data as a binary string,
             * not a Uint8Array.  Convert in 8 KB chunks to avoid call-stack
             * overflow on String.fromCharCode.apply(). */
            var chunks = [];
            var chunkSize = 8192;
            for (var i = 0; i < data.length; i += chunkSize) {
              chunks.push(String.fromCharCode.apply(null, data.subarray(i, i + chunkSize)));
            }
            var binaryString = chunks.join('');

            var sizeMB = (data.length / 1024 / 1024).toFixed(1);
            setStatus('Flashing <b>' + fw.name + '</b> (' + sizeMB + ' MB) — do not unplug the device…');

            var flashOptions = {
              fileArray:      [{ data: binaryString, address: 0x0 }],
              flashSize:      '16MB',
              flashMode:      'dio',
              flashFreq:      '80m',
              eraseAll:       false,   /* <-- only erase sectors being written */
              compress:       true,
              reportProgress: function (fileIndex, written, total) {
                var pct = Math.round((written / total) * 100);
                setProgress(pct);
              }
            };

            /* MD5 verification (optional — uses CryptoJS if loaded) */
            if (typeof CryptoJS !== 'undefined') {
              flashOptions.calculateMD5Hash = function (image) {
                var raw = Array.from(image, function (b) { return String.fromCharCode(b); }).join('');
                return CryptoJS.MD5(CryptoJS.enc.Latin1.parse(raw)).toString();
              };
            }

            await esploader.writeFlash(flashOptions);
            setProgress(100);
            setStatus('Flash complete — resetting device…', 'success');

            try { await esploader.after(); } catch (_) {}    /* hard-reset */

            /* Device has been reset — auto-disconnect and return to initial state */
            await cleanup();
            btnConnect.disabled = false;
            sel.disabled        = false;
            setStatus('&#10003; Flash complete. The device has been reset and is ready to use.', 'success');
          } catch (e) {
            console.error(e);
            setStatus('Flash failed: ' + e.message, 'error');
            btnDisconnect.disabled = false;
            if (connected) btnFlash.disabled = false;   /* allow retry */
          }
        });

        /* ---- DISCONNECT ---- */
        btnDisconnect.addEventListener('click', async function () {
          await cleanup();
          btnConnect.disabled    = false;
          btnFlash.disabled      = true;
          btnDisconnect.disabled = true;
          sel.disabled           = false;
          resetProgress();
          setStatus('Disconnected. Click <b>Connect</b> to start again.');
        });
      })();
    </script>


Download Firmware
=================

You can download the firmware images directly if you prefer to flash using command line tools.

**CTAG TBD (Development)** — Build 2026-02-11

* `ctag-tbd-2026-02-11.bin <../_static/firmware/p4/ctag-tbd-2026-02-11.bin>`_

**Possan TBD (Experimental)** — Build 2026-02-14

* `possan-tbd-2026-02-14.bin <../_static/firmware/p4/possan-tbd-2026-02-14.bin>`_


Manual Flashing (esptool)
=========================

If the browser flasher does not work, you can use ``esptool`` manually.

**Requirements:**

* Python 3 installed
* esptool: ``pip install esptool``

.. code-block:: bash

    esptool.py --chip esp32p4 -b 460800 --before=default_reset --after=hard_reset \
      write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \
      0x0 ctag-tbd-2026-02-11.bin
