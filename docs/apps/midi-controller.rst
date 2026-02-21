:orphan:

*************************************
MIDI Controller + Audio Interface
*************************************

.. admonition:: Planned
   :class: important

   This App is not yet available. It represents what becomes possible when
   the TBD-16's dual-processor architecture is used as a bridge between
   hardware and software -- rather than as a standalone instrument.

The **MIDI Controller + Audio Interface** App turns the TBD-16 into the
perfect companion for an iPhone, iPad, or laptop. One USB-C cable connects
the device and provides MIDI control, stereo audio input, and stereo audio
output simultaneously.


At a Glance
===========

.. list-table::
   :widths: 25 75
   :header-rows: 0

   * - **Status**
     - Planned
   * - **RP2350 firmware**
     - MIDI controller firmware (planned)
   * - **ESP32-P4**
     - USB audio interface mode (no DSP)
   * - **SD card file**
     - ``tbd-apps/midi_audio.uf2`` (planned)
   * - **Tags**
     - Controller, MIDI, Audio, Planned


What It Will Do
===============

- **30 buttons → MIDI CC** -- every button on the TBD-16 sends configurable
  MIDI control messages
- **4 encoders → MIDI CC** -- continuous control with push-button support
- **OLED display** -- shows the current mapping, parameter names, or
  feedback from the connected app
- **Stereo audio in + out** -- the ESP32-P4 operates as a USB Audio Class
  compliant interface (no drivers needed on iOS/iPadOS/macOS)
- **Single USB-C cable** -- MIDI and audio travel over the same connection


Why This Matters
================

Most hardware MIDI controllers do not include an audio interface. Most USB
audio interfaces do not have programmable buttons. The TBD-16 can be both
at the same time because it has two processors:

- The **RP2350** handles MIDI and the hardware UI (buttons, encoders, display)
- The **ESP32-P4** handles USB audio (ADC/DAC, sample rate conversion, USB
  Audio Class)

The result: connect your TBD-16 to an iPad running GarageBand, AUM, or any
other audio/MIDI app. The TBD-16's hardware becomes a tactile controller for
that app, and the TBD-16's audio I/O becomes the iPad's sound card. All over
one cable.


Use Cases
=========

- **iPad/iPhone music production** -- control synth apps with physical knobs
  while monitoring through the TBD-16's audio output
- **Live performance** -- use the TBD-16 as a MIDI controller + mixer input
  for your laptop DAW
- **Standalone recording** -- route audio through the TBD-16 and back to the
  computer for monitoring and recording


Links
=====

- :doc:`Back to Apps </apps/index>`
