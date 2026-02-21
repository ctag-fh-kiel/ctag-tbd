:orphan:

*********************
Multi Effect / Synth
*********************

The **Multi Effect / Synth** App lets you browse, load, and play any of the
50+ DSP plugins using the TBD-16's hardware interface -- no computer or web
UI required.


At a Glance
===========

.. list-table::
   :widths: 25 75
   :header-rows: 0

   * - **Status**
     - Shipping
   * - **RP2350 firmware**
     - Plugin browser firmware
   * - **ESP32-P4**
     - Standard DSP firmware with 50+ plugins
   * - **SD card file**
     - ``tbd-apps/multi_fx.uf2``
   * - **Tags**
     - Synth, FX, MIDI, Ableton Link


What It Does
============

- **Browse plugins** on the OLED with encoders -- scroll, select, load
- **Two independent channels** -- run any two plugins at the same time
- **Mono or stereo** routing per channel
- **Preset management** -- save and recall parameter snapshots
- **MIDI mapping** -- assign knobs and buttons to plugin parameters via MIDI CC
- **Ableton Link** for tempo sync


Dual-Channel Architecture
=========================

The ESP32-P4 DSP firmware always has **two plugin slots** (Slot 0 and Slot 1).
The Multi Effect / Synth App gives you direct hardware control over both:

.. list-table::
   :header-rows: 1
   :widths: 20 40 40

   * -
     - Slot 0
     - Slot 1
   * - **Use case**
     - Main synth or input effect
     - Second synth, effect chain, or parallel processor
   * - **Routing**
     - Mono or stereo
     - Mono or stereo
   * - **Control**
     - Encoders + buttons + MIDI
     - Encoders + buttons + MIDI

This means you can run a synthesizer in Slot 0 and a reverb in Slot 1, or two
completely independent effects, or two synths layered together.

See the :doc:`full list of 50+ plugins </plugins/index>` available for each
slot.


How It Works
============

**RP2350** (plugin browser firmware)
   Displays the plugin list on the OLED. Reads encoder input to scroll through
   plugins and adjust parameters. Sends load commands and parameter updates to
   the ESP32-P4 via SPI.

**ESP32-P4** (standard DSP firmware)
   Runs the same DSP engine used by all Apps. Receives plugin load and parameter
   messages from the RP2350. Processes audio in real time with sub-millisecond
   latency.


Why This Exists
===============

The :doc:`Groovebox <groovebox>` is great when you want a complete instrument.
But sometimes you just want to plug in a guitar and dial up two effects, or play
a single synth with full hands-on control. The Multi Effect / Synth App is
that stripped-down, direct-access mode.


Links
=====

- :doc:`50+ DSP Plugins </plugins/index>`
- :doc:`Back to Apps </apps/index>`
