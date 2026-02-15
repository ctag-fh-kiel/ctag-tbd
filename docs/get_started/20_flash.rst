**************
Device Flasher
**************

Flash Device From browser
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

You can download the artifacts directly if you prefer to flash using command line tools.

**CTAG TBD (Development)**

* `Factory Image (Recommended) <../_static/device_manifests/ctag-tbd.bin>`_

* `Bootloader <https://dadamachines.github.io/ctag-tbd/downloads/ctag-dada-tbd/bootloader.bin>`_
* `Partition Table <https://dadamachines.github.io/ctag-tbd/downloads/ctag-dada-tbd/partition-table.bin>`_
* `OTA Data <https://dadamachines.github.io/ctag-tbd/downloads/ctag-dada-tbd/ota_data_initial.bin>`_
* `Application <https://dadamachines.github.io/ctag-tbd/downloads/ctag-dada-tbd/ctag-tbd.bin>`_

**Possan TBD (Experimental)**

* `Factory Image (Recommended) <../_static/device_manifests/possan-tbd.bin>`_

* `Bootloader <https://dadamachines.github.io/ctag-tbd/downloads/possan-dada-tbd/bootloader.bin>`_
* `Partition Table <https://dadamachines.github.io/ctag-tbd/downloads/possan-dada-tbd/partition-table.bin>`_
* `OTA Data <https://dadamachines.github.io/ctag-tbd/downloads/possan-dada-tbd/ota_data_initial.bin>`_
* `Application <https://dadamachines.github.io/ctag-tbd/downloads/possan-dada-tbd/ctag-tbd.bin>`_


Manual Flashing (esptool)
=========================

If the web flasher does not work, you can use `esptool` manually.

Requirements:

* Python installed
* esptool installed (`pip install esptool`)
* The 4 binary files downloaded from above OR the single factory image

**Command (using factory image):**

.. code-block:: bash

    esptool.py --chip esp32p4 -b 460800 --before=default_reset --after=hard_reset \
      write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \
      0x0 factory.bin

**Command (using split parts):**

.. code-block:: bash

    esptool.py --chip esp32p4 -b 460800 --before=default_reset --after=hard_reset \
      write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \
      0x2000  bootloader.bin \
      0x8000  partition-table.bin \
      0xD000  ota_data_initial.bin \
      0x10000 ctag-tbd.bin
