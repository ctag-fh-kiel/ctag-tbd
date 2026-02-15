*********************
ESP32-P4 Device Flasher
*********************

Flash Device From Browser
=========================

Select your firmware and click **Install** to flash your ESP32-P4.

**Hardware setup:**

1. Connect a USB cable to the **JTAG USB-C port on the front** of the device (data connection).
2. Connect a second USB cable to one of the **two USB-C ports on the back** (power).

**Browser:** Chrome, Edge or Opera required.

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
      .esp-flasher esp-web-install-button {
        display: none;
      }
      .esp-flasher esp-web-install-button.active {
        display: inline-block;
      }
      .esp-flasher esp-web-install-button button {
        padding: 0.5em 1.2em;
        border: none;
        border-radius: 4px;
        font-size: 0.9em;
        font-weight: 600;
        cursor: pointer;
        background: #2563EB;
        color: #fff;
        transition: opacity 0.15s;
      }
      .esp-flasher esp-web-install-button button:hover {
        opacity: 0.85;
      }
    </style>

    <script
      type="module"
      src="https://unpkg.com/esp-web-tools@10.2.1/dist/web/install-button.js?module"
    ></script>

    <div class="esp-flasher" id="espFlasher">
      <label for="espFirmwareSelect">Firmware</label>
      <select id="espFirmwareSelect">
        <option value="ctag" selected>CTAG TBD (Development) — 2026-02-11</option>
        <option value="possan">Possan TBD (Experimental) — 2026-02-14</option>
      </select>

      <div class="btn-row">
        <esp-web-install-button id="espBtnCtag" class="active" manifest="https://dadamachines.github.io/ctag-tbd/_static/device_manifests/manifest-tbd-ctag.json">
          <button slot="activate">Install Firmware</button>
          <span slot="unsupported">Unsupported browser — use Chrome, Edge or Opera</span>
          <span slot="not-allowed">Permission denied</span>
        </esp-web-install-button>
        <esp-web-install-button id="espBtnPossan" manifest="https://dadamachines.github.io/ctag-tbd/_static/device_manifests/manifest-tbd-possan.json">
          <button slot="activate">Install Firmware</button>
          <span slot="unsupported">Unsupported browser — use Chrome, Edge or Opera</span>
          <span slot="not-allowed">Permission denied</span>
        </esp-web-install-button>
      </div>

      <div class="status-box">
        Select a firmware from the dropdown and click <b>Install Firmware</b>.
      </div>
    </div>

    <script>
      (function() {
        var sel = document.getElementById('espFirmwareSelect');
        var btnCtag = document.getElementById('espBtnCtag');
        var btnPossan = document.getElementById('espBtnPossan');
        function update() {
          if (sel.value === 'ctag') {
            btnCtag.classList.add('active');
            btnPossan.classList.remove('active');
          } else {
            btnCtag.classList.remove('active');
            btnPossan.classList.add('active');
          }
        }
        sel.addEventListener('change', update);
        update();
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
