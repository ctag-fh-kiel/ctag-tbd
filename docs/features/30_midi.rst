****
MIDI
****

The TBD-16 provides full MIDI connectivity for playing synthesizer plugins
chromatically, triggering drum machines, and controlling parameters.


Connections
===========

The TBD-16 supports three MIDI paths:

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Connection
     - Details
   * - **DIN MIDI In**
     - Standard 5-pin DIN input for connecting keyboards, sequencers, and controllers
   * - **DIN MIDI Out**
     - Standard 5-pin DIN output for sending MIDI to external gear
   * - **USB MIDI**
     - Class-compliant USB MIDI via one of the back USB-C ports (no drivers required)

All three paths are class-compliant --- no special drivers are needed on any operating system.


How MIDI Works on TBD-16
=========================

MIDI is processed by the **RP2350** front-end processor, which handles all
hardware I/O. The RP2350 receives MIDI messages and forwards them to the
**ESP32-P4** DSP engine via the SPI bus, where plugins can respond to:

- **Note On / Note Off** --- Play melodic synthesizers, trigger drum hits
- **Control Change (CC)** --- Map MIDI CCs to plugin parameters
- **Pitch Bend** --- Continuous pitch modulation
- **Program Change** --- Switch between presets or plugins
- **Clock** --- Tempo synchronization via MIDI clock

.. tip::

   For wireless tempo sync without MIDI cables, use
   :doc:`Ableton Link <10_ableton_link>` instead of MIDI clock.


Using MIDI with Plugins
========================

Plugins that support MIDI input will respond to incoming messages automatically.
The web UI shows which parameters can be MIDI-mapped and allows you to assign
MIDI CCs to any parameter.

Synthesizer plugins typically respond to:

- Note messages for pitch
- Velocity for dynamics
- CC messages for filter cutoff, resonance, and other parameters

Effect plugins can use MIDI to:

- Tap tempo via note messages
- Control wet/dry mix via CC
- Trigger freeze/hold functions via note on/off
