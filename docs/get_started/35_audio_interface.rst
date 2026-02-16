*********************
USB Audio Interface
*********************

The TBD-16 hardware supports operating as a **USB Audio Interface**, allowing your
computer to send and receive audio directly to and from the device over USB.

.. note::

   USB Audio Interface mode is **not included in the main firmware** shipped with TBD-16.
   The ESP32-P4 hardware is capable of acting as a USB Audio Class device, but this
   feature requires a separate or custom firmware build. It is listed here because
   the hardware supports it and it may be offered as an option in the future.


What This Means
===============

When running USB Audio Interface firmware, the TBD-16 acts as a class-compliant
USB audio device:

- Your computer (macOS, Windows, Linux) sees TBD-16 as a standard sound card
- Audio is streamed over USB with low latency
- You can use your DAW to route audio through the TBD-16's DSP engine
- No additional drivers are required on macOS and Linux (class-compliant)


Potential Use Cases
===================

- **Hardware insert effect** --- Route a track from your DAW through the TBD-16,
  apply one of the 50+ effects, and capture the processed audio back.
- **External synthesizer** --- Use the TBD-16 as a sound source in your DAW,
  triggered via MIDI.
- **Low-latency monitoring** --- Direct monitoring through the TBD-16's audio
  outputs while recording.


Current Status
==============

The ESP32-P4 includes USB peripheral hardware (USB-OTG) capable of implementing
USB Audio Class 1.0 or 2.0. The main TBD firmware currently uses USB for
serial communication and firmware flashing, not audio streaming.

If you are interested in developing or testing USB Audio Interface firmware,
see the :doc:`Building & Setup </development/01_building>` guide for compiling
custom firmware, and check the
`project GitHub <https://github.com/dadamachines/ctag-tbd>`_ for updates on this feature.
