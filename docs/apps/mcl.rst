:orphan:

*****************************
MegaCommand Live (MCL)
*****************************

**MegaCommand Live** is a high-performance MIDI sequencer originally created by
`Justin Mammarella (jmamma) <https://github.com/jmamma>`_ for the Elektron
MachineDrum. On the TBD-16, it runs natively on the RP2350 while the ESP32-P4
provides Ableton Link sync.


At a Glance
===========

.. list-table::
   :widths: 25 75
   :header-rows: 0

   * - **Status**
     - Shipping
   * - **RP2350 firmware**
     - `MCL <https://github.com/jmamma/MCL>`_ by
       `jmamma <https://github.com/jmamma>`_
   * - **ESP32-P4**
     - Ableton Link provider (no DSP)
   * - **SD card file**
     - ``tbd-apps/mcl.uf2``
   * - **Tags**
     - Sequencer, MIDI, Ableton Link


What It Does
============

- **MIDI sequencing** with tracks for external hardware (MachineDrum,
  Monomachine, Analog Four, or any MIDI instrument)
- **Song mode** for chaining patterns into arrangements
- **Live performance tools** -- real-time transpose, mute groups, parameter
  locks
- **Ableton Link** -- the ESP32-P4 runs Link and provides a shared tempo
  to the RP2350, so MCL locks to your DAW or other Link-enabled devices
- **Full hardware UI** -- the TBD-16's 30 buttons, 4 encoders, and OLED
  display are mapped for hands-on sequencing


How It Works
============

**RP2350** (MCL firmware)
   Runs the complete MegaCommand Live sequencer engine. Drives the OLED,
   reads buttons and encoders, sends and receives MIDI data via USB and
   TRS connectors.

**ESP32-P4** (Ableton Link provider)
   Does not run DSP plugins in this App. Instead, it connects to the local
   WiFi network and runs an Ableton Link session. The shared tempo is relayed
   to MCL on the RP2350 so everything stays in sync.

This is a good example of how the TBD-16's dual-processor architecture works:
the ESP32-P4 does not always run synth engines. Here, it contributes network
connectivity and Link sync while the RP2350 does the heavy sequencing work.


About the MCL Project
=====================

MegaCommand Live started as firmware for the
`MegaCommand MIDI Controller <https://github.com/jmamma/MCL>`_ hardware,
designed to extend the Elektron MachineDrum with pattern management, song
mode, and advanced MIDI features. Justin Mammarella (jmamma) ported it to
the RP2350 for the TBD-16.

The MCL source is available on GitHub under the GPL license.


Links
=====

- `MCL on GitHub <https://github.com/jmamma/MCL>`_
- `jmamma on GitHub <https://github.com/jmamma>`_
- :doc:`Back to Apps </apps/index>`
