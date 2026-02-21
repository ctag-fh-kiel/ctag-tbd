:orphan:

*************
Utility Apps
*************

The TBD-16 ships with several utility Apps on the SD card. These are not
musical instruments -- they are tools for development, maintenance, and testing.


.. _debug-probe:

Debug Probe
===========

.. list-table::
   :widths: 25 75
   :header-rows: 0

   * - **SD card file**
     - ``tbd-apps/dbg_prb.uf2``
   * - **What it does**
     - Turns the RP2350 into a
       `CMSIS-DAP <https://arm-software.github.io/CMSIS_5/DAP/html/index.html>`_
       debug probe

The TBD-16's UI board has an **STM32F030R8T6** microcontroller that handles
I2C communication with the buttons and encoders. The Debug Probe App turns
the RP2350 into a debug adapter so you can flash and debug the STM32 without
additional hardware.

- Source: `debugprobe-tbd <https://github.com/ctag-fh-kiel/debugprobe-tbd>`_
  on GitHub
- STM32 firmware: `stm32-tbd-fw <https://github.com/ctag-fh-kiel/stm32-tbd-fw>`_
  on GitHub


.. _usb-mass-storage:

USB Mass Storage
================

.. list-table::
   :widths: 25 75
   :header-rows: 0

   * - **SD card file**
     - ``tbd-apps/tusb_msc.uf2``
   * - **What it does**
     - Exposes the RP2350's SD card as a USB drive on your computer

Boot into this App and the SD card appears as a removable drive. You can:

- Copy new ``.uf2`` Apps to the ``tbd-apps/`` folder
- Back up or restore presets and samples
- Update the boot menu (``BOOT2350.uf2``)

No need to remove the SD card from the device.


.. _ui-test:

UI Test
=======

.. list-table::
   :widths: 25 75
   :header-rows: 0

   * - **SD card file**
     - ``tbd-apps/ui_test.uf2``
   * - **What it does**
     - Hardware test that exercises the OLED display, RGB LEDs,
       potentiometers, buttons, and SD card

Useful for verifying that all hardware components are working correctly
after assembly or if you suspect a hardware issue.


.. _game:

Game
====

.. list-table::
   :widths: 25 75
   :header-rows: 0

   * - **SD card file**
     - ``tbd-apps/game.uf2``
   * - **What it does**
     - A small game that demonstrates the RP2350 can run standalone
       applications beyond audio firmware

A fun proof of concept. It shows that the TBD-16's App system is not
limited to music -- any RP2350 firmware can run on the device.


Links
=====

- :doc:`Bootloader <bootloader>` -- how to switch between Apps
- :doc:`Back to Apps </apps/index>`
