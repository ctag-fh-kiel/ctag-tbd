****
Apps
****

.. image:: /_static/assets/dadamachines-tbd-16_mockup_002.jpg
   :alt: dadamachines TBD-16 Apps
   :width: 100%

The TBD-16 is not a single-purpose instrument. It is a **platform that runs
Apps** -- complete firmware configurations that change what the device does and
how it feels. Switch between a groovebox, a multi-effect processor, a MIDI
sequencer, a USB audio interface, or your own creation. Each App is stored as a
``.uf2`` file on the SD card and can be launched from the boot menu.


What Is an App?
===============

An App is a combination of **RP2350 firmware** (handling the hardware UI, MIDI,
sequencer logic) and **ESP32-P4 behaviour** (running DSP plugins, providing
WiFi, Ableton Link, or USB audio). Together, they define the full user
experience.

.. list-table::
   :header-rows: 1
   :widths: 30 35 35

   * - Layer
     - What It Does
     - Examples
   * - **RP2350 firmware** (.uf2)
     - Controls buttons, encoders, display, LEDs, MIDI routing, sequencer
     - Groovebox UI, multi-effect UI, MCL sequencer, MIDI controller
   * - **ESP32-P4 DSP**
     - Runs audio plugins, manages WiFi, Ableton Link, USB audio
     - 50+ synths/effects, audio interface mode, Link sync

Some Apps require a specific ESP32-P4 firmware; others work with the standard
DSP firmware. The boot menu on the OLED lets you pick which RP2350 App to run.

Think of it like the
`monome norns <https://norns.community/>`_ script ecosystem or
`Critter & Guitari Organelle <https://www.critterandguitari.com/organelle-patches>`_
patches -- except that TBD-16 Apps can also change the hardware UI behaviour and
MIDI routing, not just the sound engine.


Apps Shipping with TBD-16
=========================


Groovebox (default)
-------------------

The flagship App and what powers the TBD-16 out of the box.

.. list-table::
   :widths: 25 75
   :header-rows: 0

   * - **RP2350**
     - ``tbd-pico-seq3`` firmware by
       `Per-Olov Jernberg (Possan) <https://possan.codes/>`_
       (`GitHub <https://github.com/possan>`_)
   * - **ESP32-P4**
     - PicoSeqRack plugin (LGPL 3.0)
   * - **What it does**
     - 16-track drum machine / groovebox with step sequencing, pattern
       management, per-track mute/solo, and full hardware control via the
       TBD-16's buttons, encoders, and display. MIDI in/out, Ableton Link.

.. note::
   The names *tbd-pico-seq3* and *PicoSeqRack* are working titles. A final
   name for the Groovebox App will be announced before the TBD-16 ships.


Multi-Effect / Synth
--------------------

.. list-table::
   :widths: 25 75
   :header-rows: 0

   * - **RP2350**
     - Plugin browser firmware (planned)
   * - **ESP32-P4**
     - Standard DSP firmware with 50+ plugins
   * - **What it does**
     - Browse and load any of the 50+ DSP plugins (synthesizers, effects,
       drum machines, granular engines) using the TBD-16's hardware UI. Two
       independent channels let you run any two plugins simultaneously --
       as a mono or stereo effect chain, a dual synth, or any combination.
       MIDI mapping, preset management, and Ableton Link.


MegaCommand Live (MCL)
----------------------

.. list-table::
   :widths: 25 75
   :header-rows: 0

   * - **RP2350**
     - `MCL <https://github.com/jmamma/MCL>`_ firmware by
       `Justin Mammarella (jmamma) <https://github.com/jmamma>`_
   * - **ESP32-P4**
     - Ableton Link provider (no DSP)
   * - **What it does**
     - A high-performance MIDI sequencer originally built for the Elektron
       MachineDrum. On the TBD-16, MCL uses the hardware UI for hands-on
       sequencing while the ESP32-P4 provides Ableton Link sync. A powerful
       App for anyone who sequences external MIDI gear.


Planned Apps
============


MIDI Controller + Audio Interface
---------------------------------

.. list-table::
   :widths: 25 75
   :header-rows: 0

   * - **RP2350**
     - MIDI controller firmware (planned)
   * - **ESP32-P4**
     - USB audio interface mode (no DSP)
   * - **What it does**
     - Turns the TBD-16 into a companion for an iPhone, iPad, or computer.
       The RP2350 firmware maps the TBD-16's 30 buttons, 4 encoders, and
       display to MIDI CC messages, making it a tactile controller for iOS
       / iPadOS music apps. Meanwhile, the ESP32-P4 operates as a USB
       audio interface, providing stereo audio input and output -- all
       over a single USB-C cable. No DSP runs on the device; it is a
       controller and audio bridge.


Community & Third-Party Apps
----------------------------

The App system is open. Anyone can build and share Apps for the TBD-16.
We expect community-developed Apps to appear as the platform matures --
new sequencers, experimental interfaces, games, visualisers, and things
we have not imagined yet.

If you are working on an App, share it on the
`dadamachines Forum <https://forum.dadamachines.com>`_ or open a pull
request on `GitHub <https://github.com/dadamachines/ctag-tbd>`_.


Utility Apps
============

The TBD-16 also ships with several utility Apps on the SD card:

.. list-table::
   :widths: 22 78
   :header-rows: 1

   * - App
     - Description
   * - **Debug Probe** (``dbg_prb.uf2``)
     - Turns the RP2350 into a
       `CMSIS-DAP debug probe <https://github.com/ctag-fh-kiel/debugprobe-tbd>`_
       for flashing and debugging the **STM32F030R8T6** on the UI board
       (buttons, encoders via I2C). STM32 firmware source:
       `stm32-tbd-fw <https://github.com/ctag-fh-kiel/stm32-tbd-fw>`_.
   * - **USB Mass Storage** (``tusb_msc.uf2``)
     - Exposes the RP2350's SD card as a USB drive on your computer. Copy
       files, update App UF2s, or back up data without removing the card.
   * - **UI Test** (``ui_test.uf2``)
     - Hardware test that exercises the OLED display, RGB LEDs,
       potentiometers, buttons, and SD card.
   * - **Game** (``game.uf2``)
     - A small game demonstrating that the RP2350 can run standalone
       applications beyond audio firmware.


How It Works: The UF2 Bootloader
=================================

The TBD-16's RP2350 runs a custom **UF2 bootloader** that loads Apps from the
SD card. On power-up:

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
adaptation of the PicoCalc bootloader for the TBD platform).


SD Card Layout
--------------

.. code-block:: text

    /
    ├── BOOT2350.uf2          # Boot menu UI (required)
    └── tbd-apps/
        ├── groovebox.uf2     # Groovebox (default)
        ├── mcl.uf2           # MegaCommand Live
        ├── dbg_prb.uf2       # Debug probe
        ├── tusb_msc.uf2      # USB mass storage
        ├── ui_test.uf2       # Hardware test
        ├── game.uf2          # Example game
        └── your_app.uf2      # Your own App

``BOOT2350.uf2``
   The boot menu UI. It runs on the TBD-16's OLED display and lets you browse
   and launch Apps with the encoders and buttons. This file must be in the
   root of the SD card.

``tbd-apps/``
   A folder containing all launchable ``.uf2`` firmware images. Any UF2 file
   placed here will appear in the boot menu.


Adding Your Own App
-------------------

Any firmware you build with the RP2350 template repository produces a ``.uf2``
file. To make it available in the boot menu:

1. Build your project (``pio run``).
2. Copy the output ``.uf2`` file to the SD card:

   **Option A -- via USB Mass Storage App:**

   a. Boot into the **USB Mass Storage** App from the menu.
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


Switching Apps
--------------

1. **Power-cycle** the device (turn it off and back on).
2. **Hold Page Up** during power-on to re-enter the boot menu.

The bootloader remembers the last-launched App and will auto-start it on the
next power cycle unless you hold Page Up.

.. note::
   The bootloader and boot menu read the SD card using the SPI interface.
   If your App switches the SD card to SDIO mode, you need to
   **power-cycle** (not just reset) to re-initialize the SD card for the
   boot menu.


Technical Details
-----------------

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


Inspiration
===========

The TBD-16 App model draws on platforms where the community shapes what the
hardware becomes:

- `norns community <https://norns.community/>`_ -- hundreds of scripts for
  the monome norns, browseable by category and author
- `Organelle patches <https://www.critterandguitari.com/organelle-patches>`_
  (Critter & Guitari) and
  `patchstorage.com/organelle <https://patchstorage.com/platform/organelle/>`_
  -- Pure Data patches shared across the community
- `norns-community on GitHub <https://github.com/monome-community/norns-community>`_
  -- open-source catalog infrastructure

We want the TBD-16 to grow the same way. The
`dadamachines Forum <https://forum.dadamachines.com>`_ is the place to share
your Apps and discover what others have built.
