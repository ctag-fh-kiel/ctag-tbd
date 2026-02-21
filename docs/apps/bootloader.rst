**********************
Bootloader & Multi-App
**********************

The TBD-16 ships with a **UF2 bootloader** on the RP2350 that lets you store
multiple Apps on the SD card and switch between them from a boot menu. This is
the default setup -- the Groovebox, Multi Effect, MCL, and all other Apps
coexist on a single device.

.. note::
   The bootloader is the **default but not mandatory**. If you only need one
   App -- for example a dedicated MCL or Groovebox unit -- you can remove the
   bootloader entirely and flash a single firmware directly. See
   :ref:`single-app-mode` below.


How It Works
============

On power-up:

1. If an App was previously loaded, it starts automatically.
2. **Hold Page Up** during power-on to open the boot menu.
3. The menu reads the SD card, lists available ``.uf2`` files, and lets you
   launch any of them.
4. **Hold Page Down** during power-on to enter BOOTSEL mode
   (for direct USB flashing via Picotool or drag-and-drop).

If no SD card is inserted or the boot menu file is missing, the device falls
back to BOOTSEL mode automatically.

The bootloader is based on the
`UF2 Loader <https://github.com/ctag-fh-kiel/uf2loader>`_ project (an
adaptation of the PicoCalc bootloader by
`pelrun <https://github.com/pelrun/uf2loader>`_ for the TBD platform).


SD Card Layout
==============

.. code-block:: text

    /
    ├── BOOT2350.uf2          # Boot menu UI (required)
    └── tbd-apps/
        ├── groovebox.uf2     # Groovebox (default)
        ├── multi_fx.uf2      # Multi Effect / Synth
        ├── mcl.uf2           # MegaCommand Live
        ├── dbg_prb.uf2       # Debug Probe
        ├── tusb_msc.uf2      # USB Mass Storage
        ├── ui_test.uf2       # UI test
        ├── game.uf2          # Example game
        └── your_app.uf2      # Your own App

``BOOT2350.uf2``
   The boot menu UI. It runs on the TBD-16's OLED display and lets you browse
   and launch Apps with the encoders and buttons. This file must be in the
   root of the SD card.

``tbd-apps/``
   A folder containing all launchable ``.uf2`` firmware images. Any UF2 file
   placed here will appear in the boot menu.


Switching Apps
==============

1. **Power-cycle** the device (turn it off and back on).
2. **Hold Page Up** during power-on to re-enter the boot menu.

The bootloader remembers the last-launched App and will auto-start it on the
next power cycle unless you hold Page Up.

.. note::
   The bootloader and boot menu read the SD card using the SPI interface.
   If your App switches the SD card to SDIO mode, you need to
   **power-cycle** (not just reset) to re-initialize the SD card for the
   boot menu.


.. _adding-your-own-app:

Adding Your Own App
===================

Any firmware you build with the RP2350 template repository produces a ``.uf2``
file. To make it available in the boot menu:

1. Build your project (``pio run``).
2. Copy the output ``.uf2`` file to the SD card:

   **Option A -- via USB Mass Storage App:**

   a. Boot into the :ref:`USB Mass Storage <usb-mass-storage>` App from the
      menu.
   b. The SD card appears as a USB drive on your computer.
   c. Copy your ``.uf2`` file into the ``tbd-apps/`` folder.
   d. Reboot and select your App from the menu.

   **Option B -- remove the SD card:**

   a. Remove the SD card from the TBD-16.
   b. Insert it into your computer using a card reader.
   c. Copy your ``.uf2`` file into the ``tbd-apps/`` folder.
   d. Re-insert the card and reboot.

3. On next power-up, hold **Page Up** to open the boot menu and select your App.

.. tip::
   If your App uses
   `Pico Binary Information <https://www.raspberrypi.com/documentation/pico-sdk/runtime.html#group_pico_binary_info>`_
   blocks, the boot menu will display its name automatically. Otherwise it
   shows the filename.


.. _single-app-mode:

Single-App Mode (No Bootloader)
===============================

If you want to run a single App permanently -- for example turning the TBD-16
into a dedicated Groovebox or MCL unit -- you can remove the bootloader and
flash your firmware directly to the RP2350.

**Step 1: Erase the flash**

Use `pico-universal-flash-nuke <https://github.com/Gadgetoid/pico-universal-flash-nuke>`_
to completely erase the RP2350's flash memory, including the bootloader
partition:

1. Hold **Page Down** during power-on to enter BOOTSEL mode.
2. Drag-and-drop the ``flash_nuke.uf2`` file onto the USB drive that appears.
3. The RP2350 will erase its entire flash and reboot into BOOTSEL mode again.

**Step 2: Flash your single App**

With the bootloader erased, flash your App's ``.uf2`` directly:

- Drag-and-drop the ``.uf2`` file onto the BOOTSEL USB drive, **or**
- Use ``picotool load your_app.uf2`` from the command line.

The RP2350 will now boot directly into your App on every power-up -- no boot
menu, no SD card required for launching.

**Restoring the bootloader:**

To get the multi-App boot menu back, repeat the BOOTSEL process and flash the
bootloader ``.uf2`` from the
`UF2 Loader releases <https://github.com/ctag-fh-kiel/uf2loader>`_.


Technical Details
=================

- The bootloader uses a **flash partition** on the RP2350. Apps cannot see or
  overwrite the bootloader under normal operation.
- Apps should use the RP2350's ``rom_flash_op`` API (not the legacy
  ``flash_range_erase``/``flash_range_program`` APIs) to access flash, as the
  partition table restricts direct access.
- The bootloader is based on
  `pelrun/uf2loader <https://github.com/pelrun/uf2loader>`_ (PicoCalc),
  adapted for the TBD-16 with SSD1309 display support, TBD button mapping,
  and pin reassignment.
- UF2 files can still be written via BOOTSEL mode or Picotool as normal and
  will automatically be placed in the app partition.


Links
=====

- `UF2 Loader source <https://github.com/ctag-fh-kiel/uf2loader>`_ on GitHub
- `UF2 specification <https://github.com/microsoft/uf2>`_
- `pico-universal-flash-nuke <https://github.com/Gadgetoid/pico-universal-flash-nuke>`_ -- erase RP2350 flash completely
- :doc:`Back to Apps </apps/index>`
