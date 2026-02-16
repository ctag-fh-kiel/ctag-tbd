***********
Development
***********

This section is currently under development and will be updated in a future release.


Build Notes
===========

SD Card Initialization
----------------------

Initial SD card data is created during ``idf.py build`` (``tbd-sd-card.zip`` and ``tbd-sd-card-hash.txt``).

Build Process
^^^^^^^^^^^^^

When the firmware is built with ``idf.py build``, two important files are created in the ``build/`` folder:

- ``tbd-sd-card.zip``
- ``tbd-sd-card-hash.txt``

The data contained in ``tbd-sd-card.zip`` is packaged during the build process with the script ``create_sd_archive.sh`` (located in the repository root). This script bundles:

- All files in ``sdcard_image/`` (former SPIFFS data), including all JSON configs and www
- Data from the ``sample_rom/tbdsamples/`` folder
- Calculates a hash and includes it into the ``.zip`` and the ``tbd-sd-card-hash.txt`` file to detect changes in base data

Initial Setup
^^^^^^^^^^^^^

1. Delete all content on the SD card
2. Copy ``tbd-sd-card.zip`` and ``tbd-sd-card-hash.txt`` onto the SD card
3. Upon first boot, the firmware recognizes the ``.zip`` archive and unpacks it (contains all config files, www, and stock samples)
4. Unpacking takes some time - monitor it with ``idf.py monitor``

Troubleshooting
^^^^^^^^^^^^^^^

If you want to force re-unpacking of the ``.zip``, delete the file ``.version`` in the root of the SD card when USB-MSC is active. This file contains the hash of the archive and is checked by the firmware upon boot for version comparison with ``tbd-sd-card-hash.txt``.


SD Card Access via USB-MSC
--------------------------

In order to access the SD card, partition ``ota1`` has to be flashed with the USB-MSC firmware.

Flashing OTA1 Partition
^^^^^^^^^^^^^^^^^^^^^^^

The firmware and commands are in the ``bin/`` folder of the ctag-tbd repository. The USB-MSC firmware binary is ``tusb_msc.bin``. Sources for the ota_1 partition are at: https://github.com/ctag-fh-kiel/tbd-usb-msc

**Prerequisites:**

- Requires `esptool <https://github.com/espressif/esptool>`_
- Set ``IDF_PATH`` environment variable

**To flash the ota1 partition:**

.. code-block:: bash

   python $IDF_PATH/components/app_update/otatool.py --port /dev/cu.usbmodem101 write_ota_partition --name ota_1 --input tusb_msc.bin

**To boot into ota1 partition:**

.. code-block:: bash

   python $IDF_PATH/components/app_update/otatool.py --port /dev/cu.usbmodem101 switch_ota_partition --name ota_1

*Note: Adjust the* ``--port`` *parameter to match your device's serial port.*

Check ``flash_ota_1.sh`` in the ``bin/`` folder for the complete script with both commands.

Usage
^^^^^

When booted into ``ota1``:

- The SD card is automatically exposed through USB (rear left USB port of TBD)
- Upon dismount on host, the USB-MSC firmware boots back into the TBD firmware (ota0)

The RP2350 front-end can check with the SpiAPI call ``GetFirmwareInfo`` from the P4 which firmware is active, detecting USB-MSC mode from ``ota1`` or normal firmware from ``ota0``. Example: https://github.com/ctag-fh-kiel/rp2350-arduino-tbd-fw/blob/main/src/Ui.cpp#L506


C6 Firmware Update (WiFi Co-Processor)
---------------------------------------

Steps to update the C6 firmware:

1. Create a folder ``c6_fw/`` on the P4 SD card when in USB-MSC mode
2. Copy ``network_adapter.bin`` into it: https://github.com/ctag-fh-kiel/tbd-usb-msc/tree/main/esp_hosted_slave_fw
3. The update will take place on USB-MSC dismount and takes approximately 30 seconds
4. Afterwards, the stock firmware from ``ota0`` is booted