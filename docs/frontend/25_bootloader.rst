****************************
UF2 Bootloader & Apps
****************************

The TBD-16's RP2350 runs a custom **UF2 bootloader** that can load multiple
firmware images from the SD card. Instead of flashing a single firmware to the
chip, you store ``.uf2`` files on the SD card and select which one to run from
an on-screen menu. The device ships with several apps pre-installed, and you can
add your own.

This system is based on the
`UF2 Loader <https://github.com/ctag-fh-kiel/uf2loader>`_ project, an
adaptation of the PicoCalc bootloader for the TBD platform.


How It Works
============

The bootloader lives in a protected flash partition on the RP2350. On power-up:

1. If a user app was previously loaded, it starts immediately.
2. **Hold the Page Up button** during power-on to open the boot menu.
3. The menu reads the SD card, lists available ``.uf2`` files, and lets you
   launch any of them.
4. **Hold the Page Down button** during power-on to enter BOOTSEL mode
   (for direct USB flashing via Picotool or drag-and-drop).

If no SD card is inserted or the boot menu file is missing, the device falls
back to BOOTSEL mode automatically.


SD Card Layout
==============

The bootloader expects two things on the RP2350's SD card:

.. code-block:: text

    /
    ├── BOOT2350.uf2          # Boot menu UI (required)
    └── tbd-apps/
        ├── game.uf2          # Example game app
        ├── dbg_prb.uf2       # Debug probe for STM32 flashing
        ├── tusb_msc.uf2      # USB mass storage (mount SD card on PC)
        ├── ui_test.uf2       # Hardware test (OLED, LEDs, pots, buttons)
        └── your_app.uf2      # Your own firmware

``BOOT2350.uf2``
   The boot menu UI. It runs on the TBD-16's OLED display and lets you browse
   and launch apps with the encoders and buttons. This file must be in the
   root of the SD card.

``tbd-apps/``
   A folder containing all launchable ``.uf2`` firmware images. Any UF2 file
   placed here will appear in the boot menu.


Pre-Installed Apps
==================

The TBD-16 ships with the following apps on the SD card:

.. list-table::
   :widths: 22 78
   :header-rows: 1

   * - App
     - Description
   * - **Drum Machine / Groovebox**
     - The default firmware. MIDI sequencer, UI, and control surface for
       the DSP engine. This is what runs when you power on normally.
   * - **Debug Probe** (``dbg_prb.uf2``)
     - Turns the RP2350 into a
       `CMSIS-DAP debug probe <https://github.com/ctag-fh-kiel/debugprobe-tbd>`_,
       allowing you to flash and debug the **STM32F030R8T6** microcontroller
       on the UI board (which handles buttons, 360-degree endless
       potentiometers, and other input hardware via I2C). The STM32 firmware
       source is at
       `stm32-tbd-fw <https://github.com/ctag-fh-kiel/stm32-tbd-fw>`_.
       Based on the
       `Raspberry Pi debugprobe <https://github.com/raspberrypi/debugprobe>`_
       firmware.
   * - **USB Mass Storage** (``tusb_msc.uf2``)
     - Exposes the RP2350's SD card as a USB drive on your computer. Useful
       for copying files, updating app UF2s, or backing up data without
       removing the SD card from the device.
   * - **UI Test** (``ui_test.uf2``)
     - A hardware test firmware that exercises the OLED display, RGB LEDs,
       potentiometers, buttons, and SD card. Useful for verifying that all
       hardware is working correctly.
   * - **Game** (``game.uf2``)
     - An example game demonstrating that the RP2350 can run standalone
       applications beyond audio firmware.
   * - **MegaCommand Live (MCL)**
     - `MCL <https://github.com/jmamma/MCL>`_ is a high-performance MIDI
       sequencer originally built for the Elektron MachineDrum. The TBD-16
       is a supported platform, bringing MCL's powerful sequencing
       capabilities to the TBD hardware. Ships as a separate app on the
       SD card.


Adding Your Own App
===================

Any firmware you build with the RP2350 template repository produces a ``.uf2``
file. To make it available in the boot menu:

1. Build your project (``pio run``).
2. Copy the output file to the SD card:

   **Option A --- via USB Mass Storage app:**

   a. Boot into the **USB Mass Storage** app (``tusb_msc.uf2``) from the menu.
   b. The SD card appears as a USB drive on your computer.
   c. Copy your ``.uf2`` file into the ``tbd-apps/`` folder.
   d. Reboot and select your app from the menu.

   **Option B --- remove the SD card:**

   a. Remove the SD card from the TBD-16.
   b. Insert it into your computer using a card reader.
   c. Copy your ``.uf2`` file into the ``tbd-apps/`` folder.
   d. Re-insert the card and reboot.

3. On next power-up, hold **Page Up** to open the boot menu and select your app.

.. tip::
   If your app uses
   `Pico Binary Information <https://www.raspberrypi.com/documentation/pico-sdk/runtime.html#group_pico_binary_info>`_
   blocks, the boot menu will display its name automatically. Otherwise it
   shows the filename.


Returning to the Boot Menu
==========================

If an app is running and you want to switch to a different one:

1. **Power-cycle** the device (turn it off and back on).
2. **Hold Page Up** during power-on to re-enter the boot menu.

The bootloader remembers the last-launched app and will auto-start it on the
next power cycle unless you hold Page Up.


.. note::
   The bootloader and boot menu read the SD card using the SPI interface.
   If your app switches the SD card to SDIO mode, you will need to
   **power-cycle** (not just reset) to re-initialize the SD card for the
   boot menu.


Technical Details
=================

-  The bootloader uses a **flash partition** on the RP2350. User apps cannot
   see or overwrite the bootloader under normal operation.
-  Apps should use the RP2350's ``rom_flash_op`` API (not the legacy
   ``flash_range_erase``/``flash_range_program`` APIs) to access flash, as the
   partition table restricts direct access.
-  The bootloader is based on
   `pelrun/uf2loader <https://github.com/pelrun/uf2loader>`_ (PicoCalc),
   adapted for the TBD-16 with SSD1309 display support, TBD button mapping,
   and pin reassignment.
-  UF2 files can still be written via BOOTSEL mode or Picotool as normal and
   will automatically be placed in the app partition.
