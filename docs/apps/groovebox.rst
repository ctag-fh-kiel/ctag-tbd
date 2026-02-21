:orphan:

*********
Groovebox
*********

The **Groovebox** is the default App and what powers the TBD-16 out of the box.
It turns the device into a 16-track drum machine / groovebox with deep
sequencing, pattern management, and hands-on control.


At a Glance
===========

.. list-table::
   :widths: 25 75
   :header-rows: 0

   * - **Status**
     - Shipping (default App)
   * - **RP2350 firmware**
     - ``tbd-pico-seq3`` by
       `Per-Olov Jernberg (Possan) <https://possan.codes/>`_
       (`GitHub <https://github.com/possan>`_)
   * - **ESP32-P4**
     - PicoSeqRack plugin (LGPL 3.0)
   * - **SD card file**
     - ``tbd-apps/groovebox.uf2``
   * - **Tags**
     - Sequencer, Synth, MIDI, Ableton Link


What It Does
============

- **16 tracks** -- each track triggers a DSP plugin sound
- **Step sequencing** with variable step lengths and swing
- **Pattern management** -- store, recall, and chain patterns
- **Per-track mute/solo** from the hardware buttons
- **MIDI in/out** (USB and TRS Type-A) for syncing or controlling external gear
- **Ableton Link** for wireless tempo sync with DAWs, apps, and other Link
  devices on the same network


How It Works
============

The Groovebox App is a collaboration between two processors:

**RP2350** (``tbd-pico-seq3``)
   Runs the sequencer, drives the OLED display, reads the 30 buttons and
   4 encoders, manages patterns and presets, and handles all MIDI routing.
   Written by `Possan <https://possan.codes/>`_.

**ESP32-P4** (PicoSeqRack plugin)
   Runs the audio engine. Each of the 16 tracks corresponds to a voice in the
   PicoSeqRack DSP plugin. The RP2350 sends note-on/off and parameter changes
   over the SPI bus; the ESP32-P4 renders audio in real time.

The two processors communicate via SPI at high speed. From the player's
perspective they behave as one unified instrument.


Links
=====

- `Possan's website <https://possan.codes/>`_
- `Possan on GitHub <https://github.com/possan>`_
- :doc:`All 50+ DSP plugins </plugins/index>` available on the ESP32-P4
- :doc:`Back to Apps </apps/index>`
