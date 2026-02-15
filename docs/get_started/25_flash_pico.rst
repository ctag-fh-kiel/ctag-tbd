*********************
RP2350 Device Flasher
*********************

Flash Device From Browser
=========================

Put your device into **BOOTSEL mode** and connect it to your computer via USB.
Select your firmware below and click **Connect** to begin.

.. note::

   To enter BOOTSEL mode, hold the BOOTSEL button while plugging in the USB cable,
   or while pressing the RESET button.

.. note::

   This flasher requires a browser that supports `WebUSB <https://developer.mozilla.org/en-US/docs/Web/API/WebUSB_API#browser_compatibility>`_.
   Chrome, Edge and Opera are supported. Firefox and Safari are **not** supported.


.. raw:: html

    <style>
      .pico-flasher {
        border: 1px solid var(--color-background-border, #ccc);
        border-radius: 8px;
        padding: 1.5em;
        margin: 1.5em 0;
        background: var(--color-background-secondary, #f8f8f8);
        max-width: 520px;
      }
      .pico-flasher label {
        display: block;
        font-weight: 600;
        margin-bottom: 0.4em;
      }
      .pico-flasher select {
        width: 100%;
        padding: 0.5em;
        border-radius: 4px;
        border: 1px solid var(--color-background-border, #aaa);
        margin-bottom: 1em;
        font-size: 1em;
        background: var(--color-background-primary, #fff);
        color: var(--color-foreground-primary, #333);
      }
      .pico-flasher .btn-row {
        display: flex;
        gap: 0.6em;
        flex-wrap: wrap;
        margin-bottom: 1em;
      }
      .pico-flasher button {
        padding: 0.55em 1.3em;
        border: none;
        border-radius: 4px;
        font-size: 0.95em;
        font-weight: 600;
        cursor: pointer;
        transition: background 0.15s;
      }
      .pico-flasher button:disabled {
        opacity: 0.45;
        cursor: not-allowed;
      }
      .pico-flasher .btn-connect {
        background: #2563EB;
        color: #fff;
      }
      .pico-flasher .btn-connect:hover:not(:disabled) {
        background: #1D4ED8;
      }
      .pico-flasher .btn-flash {
        background: #16A34A;
        color: #fff;
      }
      .pico-flasher .btn-flash:hover:not(:disabled) {
        background: #15803D;
      }
      .pico-flasher .btn-reboot {
        background: #9333EA;
        color: #fff;
      }
      .pico-flasher .btn-reboot:hover:not(:disabled) {
        background: #7E22CE;
      }
      .pico-flasher .btn-disconnect {
        background: #DC2626;
        color: #fff;
      }
      .pico-flasher .btn-disconnect:hover:not(:disabled) {
        background: #B91C1C;
      }
      .pico-flasher .status-box {
        padding: 0.7em 1em;
        border-radius: 4px;
        font-size: 0.9em;
        min-height: 1.4em;
        word-break: break-word;
      }
      .pico-flasher .status-idle     { background: #E5E7EB; color: #374151; }
      .pico-flasher .status-busy     { background: #DBEAFE; color: #1E40AF; }
      .pico-flasher .status-success  { background: #D1FAE5; color: #065F46; }
      .pico-flasher .status-error    { background: #FEE2E2; color: #991B1B; }
      .pico-flasher .device-info {
        font-size: 0.85em;
        margin-bottom: 1em;
        padding: 0.5em 0.8em;
        border-radius: 4px;
        background: var(--color-background-primary, #fff);
        border: 1px solid var(--color-background-border, #ddd);
        display: none;
      }
      .pico-flasher .progress-wrap {
        height: 6px;
        background: #E5E7EB;
        border-radius: 3px;
        margin-top: 0.6em;
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
        <option value="possan-tbd.uf2">Possan TBD (Experimental)</option>
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

    <script type="module">
      // picoflash – MIT License – Copyright (C) 2025 Piers Finlayson
      import { Picoboot } from '../_static/picoflash/pkg/index.js';
      import { uf2ToFlashBuffer } from '../_static/picoflash/js/uf2.js';

      const FIRMWARE_BASE = '../_static/firmware/pico/';
      const OP_TIMEOUT    = 30000;  // 30s for long operations (flash)
      const SHORT_TIMEOUT = 5000;   // 5s for quick operations

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

      /* ── helpers ─────────────────────────────────────── */
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
          btnConnect.disabled = true;
          btnFlash.disabled = true;
          btnReboot.disabled = true;
          btnDisconnect.disabled = true;
          return;
        }
        const connected = picoboot && picoboot.isConnected();
        btnConnect.disabled    = connected;
        btnFlash.disabled      = !connected;
        btnReboot.disabled     = !connected;
        btnDisconnect.disabled = !connected;
      }

      async function tryRecover() {
        try {
          if (connection) await connection.resetInterface();
          return true;
        } catch (_) {
          try { if (picoboot) await picoboot.disconnect(); } catch (_) {}
          picoboot = null;
          connection = null;
          deviceInfo.style.display = 'none';
          return false;
        }
      }

      async function doDisconnect() {
        try { if (picoboot) await picoboot.disconnect(); } catch (_) {}
        picoboot = null;
        connection = null;
        deviceInfo.style.display = 'none';
      }

      /* ── connect ─────────────────────────────────────── */
      btnConnect.addEventListener('click', async () => {
        try {
          updateButtons(true);
          setStatus('Waiting for device selection…', 'busy');

          picoboot = await Picoboot.requestDevice();

          setStatus('Connecting…', 'busy');
          connection = await withTimeout(
            picoboot.connect(), SHORT_TIMEOUT, 'Connect'
          );

          // Required: clear endpoint halts so bulk transfers work
          setStatus('Resetting interface…', 'busy');
          await withTimeout(
            connection.resetInterface(), SHORT_TIMEOUT, 'Reset interface'
          );

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

      /* ── flash ───────────────────────────────────────── */
      btnFlash.addEventListener('click', async () => {
        const fwFile = fwSelect.value;
        if (!fwFile) { setStatus('Please select a firmware.', 'error'); return; }

        try {
          updateButtons(true);

          // Phase 1 – download
          setStatus('Downloading firmware…', 'busy');
          showProgress(10);
          const resp = await fetch(FIRMWARE_BASE + fwFile);
          if (!resp.ok) throw new Error('Download failed: HTTP ' + resp.status);
          const uf2Data = new Uint8Array(await resp.arrayBuffer());
          showProgress(25);

          // Phase 2 – parse UF2
          setStatus('Parsing UF2 file…', 'busy');
          const { address, data } = uf2ToFlashBuffer(uf2Data);
          const sizeKB = (data.length / 1024).toFixed(0);
          setStatus('Erasing & writing ' + sizeKB + ' KB at 0x' + address.toString(16) + '…', 'busy');
          showProgress(35);

          // Phase 3 – erase & write (library internally calls resetInterface + exitXip)
          await picoboot.flashEraseAndWrite(address, data);
          showProgress(100);

          setStatus('Flash complete &#10003; — click <b>Reboot</b> to restart the device.', 'success');
        } catch (e) {
          hideProgress();
          setStatus('Flash failed: ' + e.message + '. Attempting recovery…', 'error');
          const recovered = await tryRecover();
          if (!recovered) {
            setStatus('Flash failed: ' + e.message + '. Device disconnected — please reconnect.', 'error');
          } else {
            setStatus('Flash failed: ' + e.message + '. Device recovered — you can try again.', 'error');
          }
        }
        updateButtons(false);
      });

      /* ── reboot ──────────────────────────────────────── */
      btnReboot.addEventListener('click', async () => {
        try {
          updateButtons(true);
          setStatus('Rebooting device…', 'busy');
          try {
            await withTimeout(connection.reboot(100), SHORT_TIMEOUT, 'Reboot');
          } catch (e) {
            console.warn('Reboot command error (may be expected):', e.message);
          }
          // Always disconnect after reboot
          await doDisconnect();
          hideProgress();
          setStatus('Device rebooted and disconnected successfully.', 'success');
        } catch (e) {
          setStatus('Reboot failed: ' + e.message, 'error');
          await doDisconnect();
        }
        updateButtons(false);
      });

      /* ── disconnect ──────────────────────────────────── */
      btnDisconnect.addEventListener('click', async () => {
        await doDisconnect();
        hideProgress();
        setStatus('Disconnected.', 'idle');
        updateButtons(false);
      });

      /* ── WebUSB support check ────────────────────────── */
      if (!('usb' in navigator)) {
        setStatus(
          'Your browser does not support WebUSB. Please use Chrome, Edge or Opera.',
          'error'
        );
        btnConnect.disabled = true;
      }
    </script>


Download Latest Firmware
========================

You can also download firmware files directly:

* `Possan TBD UF2 <../_static/firmware/pico/possan-tbd.uf2>`_


Manual Flashing (UF2 Drag-and-Drop)
====================================

If the web flasher does not work, you can flash using the standard UF2 method:

1. Put the device into **BOOTSEL mode** (hold BOOTSEL while pressing RESET or plugging in USB).
2. A USB drive named ``RPI-RP2`` will appear on your computer.
3. Drag and drop the ``.uf2`` file onto the drive.
4. The device will reboot automatically after the file is copied.
