*********************
ESP32-P4 Device Flasher
*********************

Flash Device From Browser
=========================

Connect your device to computer, select your device from the list below and select the
device when asked for permission by browser.

.. raw:: html

    <embed>
        <script
        type="module"
        src="https://unpkg.com/esp-web-tools@10/dist/web/install-button.js?module"
        ></script>

        <esp-web-install-button manifest="https://dadamachines.github.io/ctag-tbd/_static/device_manifests/manifest-tbd-ctag.json">
            <button slot="activate">Flash CTAG TBD (Development)</button>
            <span slot="unsupported">error: unsupported browser</span>
            <span slot="not-allowed">error: permission denied</span>
        </esp-web-install-button>
        </br>
        <esp-web-install-button manifest="https://dadamachines.github.io/ctag-tbd/_static/device_manifests/manifest-tbd-possan.json">
            <button slot="activate">Flash Possan TBD (Experimental)</button>
            <span slot="unsupported">error: unsupported browser</span>
            <span slot="not-allowed">error: permission denied</span>
        </esp-web-install-button>
    </embed>


Download Latest Firmware Builds
===============================

You can download the firmware images directly if you prefer to flash using command line tools.

**CTAG TBD (Development)** — Build 2026-02-11

* `ctag-tbd-2026-02-11.bin <../_static/firmware/p4/ctag-tbd-2026-02-11.bin>`_

**Possan TBD (Experimental)** — Build 2026-02-14

* `possan-tbd-2026-02-14.bin <../_static/firmware/p4/possan-tbd-2026-02-14.bin>`_


Manual Flashing (esptool)
=========================

If the web flasher does not work, you can use ``esptool`` manually.

Requirements:

* Python installed
* esptool installed (``pip install esptool``)

.. code-block:: bash

    esptool.py --chip esp32p4 -b 460800 --before=default_reset --after=hard_reset \
      write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \
      0x0 ctag-tbd-2026-02-11.bin
