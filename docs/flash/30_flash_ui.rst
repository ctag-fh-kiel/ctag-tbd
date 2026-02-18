*******************
Flash UI Firmware
*******************

Flash Device From Browser
=========================

**How to enter BOOTSEL mode:** Hold the **BOOTSEL** button while plugging in USB or pressing **RESET**.

**Button locations on the dada tbd:**

* **BOOTSEL** — front panel, next to the JTAG USB-C port.
* **RESET** — to its left (or to the right of the 3.5 mm headphone jack).

Once in BOOTSEL mode, select your firmware and click **Connect**.

**Browser:** Chrome, Edge or Opera required — Firefox and Safari do not support WebUSB.

.. raw:: html

    <style>
      .pico-flasher {
        border: 1px solid var(--color-background-border, #d1d5db);
        border-radius: 8px;
        padding: 1.5em 1.8em;
        margin: 1em 0 1.5em;
        background: var(--color-background-secondary, #fafafa);
        max-width: 480px;
      }
      .pico-flasher label {
        display: block;
        font-weight: 600;
        margin-bottom: 0.35em;
        font-size: 0.95em;
        color: var(--color-foreground-secondary, #555);
      }
      .pico-flasher select {
        width: 100%;
        padding: 0.5em 0.7em;
        border-radius: 4px;
        border: 1px solid var(--color-background-border, #ccc);
        margin-bottom: 1em;
        font-size: 0.95em;
        background: var(--color-background-primary, #fff);
        color: var(--color-foreground-primary, #222);
      }
      .pico-flasher .btn-row {
        display: flex;
        gap: 0.5em;
        flex-wrap: wrap;
        margin-bottom: 0.8em;
      }
      .pico-flasher button {
        padding: 0.5em 1.2em;
        border: none;
        border-radius: 4px;
        font-size: 0.9em;
        font-weight: 600;
        cursor: pointer;
        transition: opacity 0.15s;
      }
      .pico-flasher button:disabled {
        opacity: 0.35;
        cursor: not-allowed;
      }
      .pico-flasher .btn-connect    { background: #2563EB; color: #fff; }
      .pico-flasher .btn-flash      { background: #16A34A; color: #fff; }
      .pico-flasher .btn-reboot     { background: #6B7280; color: #fff; }
      .pico-flasher .btn-disconnect { background: #6B7280; color: #fff; }
      .pico-flasher button:hover:not(:disabled) { opacity: 0.85; }
      .pico-flasher .status-box {
        padding: 0.6em 0.9em;
        border-radius: 4px;
        font-size: 0.88em;
        min-height: 1.3em;
        word-break: break-word;
      }
      .pico-flasher .status-idle    { background: #F3F4F6; color: #374151; }
      .pico-flasher .status-busy    { background: #DBEAFE; color: #1E40AF; }
      .pico-flasher .status-success { background: #D1FAE5; color: #065F46; }
      .pico-flasher .status-error   { background: #FEE2E2; color: #991B1B; }
      .pico-flasher .device-info {
        font-size: 0.82em;
        margin-bottom: 0.8em;
        padding: 0.4em 0.7em;
        border-radius: 4px;
        background: var(--color-background-primary, #fff);
        border: 1px solid var(--color-background-border, #e5e7eb);
        display: none;
      }
      .pico-flasher .progress-wrap {
        height: 5px;
        background: #E5E7EB;
        border-radius: 3px;
        margin-top: 0.5em;
        overflow: hidden;
        display: none;
      }
      .pico-flasher .progress-bar {
        height: 100%;
        background: #2563EB;
        border-radius: 3px;
        width: 0%;
        transition: width 0.3s;
      }
    </style>

    <div class="pico-flasher" id="picoFlasher">
      <label for="firmwareSelect">Firmware</label>
      <select id="firmwareSelect">        
        <option value="ctag-tbd-2026-02-11.uf2">ctag-tbd-2026-02-11.uf2 — CTAG TBD (Development)</option>
        <option value="possan-tbd-2026-02-14.uf2">possan-tbd-2026-02-14.uf2 — Possan TBD (Experimental)</option>
        <option value="possan-tbd-2026-02-17.uf2" selected>possan-tbd-2026-02-17.uf2 — Possan TBD (Experimental)</option>
      </select>

      <div class="btn-row">
        <button class="btn-connect" id="btnConnect">Connect</button>
        <button class="btn-flash"   id="btnFlash"   disabled>Flash</button>
        <button class="btn-reboot"  id="btnReboot"  disabled>Reboot</button>
        <button class="btn-disconnect" id="btnDisconnect" disabled>Disconnect</button>
      </div>

      <div class="device-info" id="deviceInfo"></div>

      <div class="status-box status-idle" id="statusBox">
        Ready &mdash; put your device in BOOTSEL mode and click <b>Connect</b>.
      </div>

      <div class="progress-wrap" id="progressWrap">
        <div class="progress-bar" id="progressBar"></div>
      </div>
    </div>

.. raw:: html

    <script type="module">
      // picoflash – MIT License – Copyright (C) 2025 Piers Finlayson
      import { Picoboot } from '../_static/picoflash/pkg/index.js';
      import { uf2ToFlashBuffer } from '../_static/picoflash/js/uf2.js';

      const FIRMWARE_BASE = '../_static/firmware/pico/';
      const OP_TIMEOUT    = 30000;
      const SHORT_TIMEOUT = 5000;

      const $ = id => document.getElementById(id);

      const btnConnect    = $('btnConnect');
      const btnFlash      = $('btnFlash');
      const btnReboot     = $('btnReboot');
      const btnDisconnect = $('btnDisconnect');
      const fwSelect      = $('firmwareSelect');
      const statusBox     = $('statusBox');
      const deviceInfo    = $('deviceInfo');
      const progressWrap  = $('progressWrap');
      const progressBar   = $('progressBar');

      let picoboot = null;
      let connection = null;

      function setStatus(msg, cls) {
        statusBox.className = 'status-box status-' + cls;
        statusBox.innerHTML = msg;
      }
      function showProgress(pct) {
        progressWrap.style.display = 'block';
        progressBar.style.width = pct + '%';
      }
      function hideProgress() {
        progressWrap.style.display = 'none';
        progressBar.style.width = '0%';
      }
      function withTimeout(promise, ms, label) {
        return Promise.race([
          promise,
          new Promise((_, reject) =>
            setTimeout(() => reject(new Error(label + ' timed out after ' + (ms/1000) + 's')), ms)
          )
        ]);
      }
      function updateButtons(busy) {
        if (busy) {
          btnConnect.disabled = btnFlash.disabled = btnReboot.disabled = btnDisconnect.disabled = true;
          return;
        }
        const c = picoboot && picoboot.isConnected();
        btnConnect.disabled = c;
        btnFlash.disabled = !c;
        btnReboot.disabled = !c;
        btnDisconnect.disabled = !c;
      }
      async function tryRecover() {
        try { if (connection) await connection.resetInterface(); return true; }
        catch (_) { await doDisconnect(); return false; }
      }
      async function doDisconnect() {
        try { if (picoboot) await picoboot.disconnect(); } catch (_) {}
        picoboot = null; connection = null;
        deviceInfo.style.display = 'none';
      }

      /* ── connect ── */
      btnConnect.addEventListener('click', async () => {
        try {
          updateButtons(true);
          setStatus('Waiting for device selection…', 'busy');
          picoboot = await Picoboot.requestDevice();
          setStatus('Connecting…', 'busy');
          connection = await withTimeout(picoboot.connect(), SHORT_TIMEOUT, 'Connect');
          setStatus('Resetting interface…', 'busy');
          await withTimeout(connection.resetInterface(), SHORT_TIMEOUT, 'Reset interface');
          const info = picoboot.getUsbDeviceInfo();
          const target = picoboot.getTarget();
          deviceInfo.style.display = 'block';
          deviceInfo.innerHTML =
            '<b>Connected:</b> ' + (info.productName || target.toString()) +
            ' &nbsp;|&nbsp; ' + picoboot.getInfo();
          setStatus('Device connected — select firmware and click <b>Flash</b>.', 'success');
        } catch (e) {
          setStatus('Connection failed: ' + e.message, 'error');
          await doDisconnect();
        }
        updateButtons(false);
      });

      /* ── flash ── */
      btnFlash.addEventListener('click', async () => {
        const fwFile = fwSelect.value;
        if (!fwFile) { setStatus('Please select a firmware.', 'error'); return; }
        try {
          updateButtons(true);
          setStatus('Downloading firmware…', 'busy'); showProgress(10);
          const resp = await fetch(FIRMWARE_BASE + fwFile);
          if (!resp.ok) throw new Error('Download failed: HTTP ' + resp.status);
          const uf2Data = new Uint8Array(await resp.arrayBuffer());
          showProgress(25);
          setStatus('Parsing UF2 file…', 'busy');
          const { address, data } = uf2ToFlashBuffer(uf2Data);
          const sizeKB = (data.length / 1024).toFixed(0);
          setStatus('Erasing & writing ' + sizeKB + ' KB at 0x' + address.toString(16) + '…', 'busy');
          showProgress(35);
          await picoboot.flashEraseAndWrite(address, data);
          showProgress(100);
          setStatus('Flash complete &#10003; — click <b>Reboot</b> to restart the device.', 'success');
        } catch (e) {
          hideProgress();
          const recovered = await tryRecover();
          setStatus('Flash failed: ' + e.message +
            (recovered ? '. Device recovered — you can try again.' : '. Please reconnect the device.'),
            'error');
        }
        updateButtons(false);
      });

      /* ── reboot ── */
      btnReboot.addEventListener('click', async () => {
        try {
          updateButtons(true);
          setStatus('Rebooting device…', 'busy');
          try { await withTimeout(connection.reboot(100), SHORT_TIMEOUT, 'Reboot'); }
          catch (e) { console.warn('Reboot command error (may be expected):', e.message); }
          await doDisconnect(); hideProgress();
          setStatus('Device rebooted and disconnected.', 'success');
        } catch (e) {
          setStatus('Reboot failed: ' + e.message, 'error');
          await doDisconnect();
        }
        updateButtons(false);
      });

      /* ── disconnect ── */
      btnDisconnect.addEventListener('click', async () => {
        await doDisconnect(); hideProgress();
        setStatus('Disconnected.', 'idle');
        updateButtons(false);
      });

      if (!('usb' in navigator)) {
        setStatus('Your browser does not support WebUSB. Please use Chrome, Edge or Opera.', 'error');
        btnConnect.disabled = true;
      }
    </script>


Download Latest Firmware
========================

* `possan-tbd-2026-02-17.uf2 <../_static/firmware/pico/possan-tbd-2026-02-17.uf2>`_


Manual Flashing (UF2 Drag-and-Drop)
====================================

1. Put the device into **BOOTSEL mode** (hold BOOTSEL while pressing RESET or plugging in USB).
2. A USB drive named ``RP2350 Boot`` will appear on your computer.
3. Drag and drop the ``.uf2`` file onto the drive.
4. The device will reboot automatically after the file is copied.
