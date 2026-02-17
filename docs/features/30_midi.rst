****
MIDI
****

The TBD-16 provides full MIDI connectivity for playing synthesizer plugins
chromatically, triggering drum machines, and controlling parameters.


Connections
===========

The TBD-16 supports multiple MIDI paths:

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Connection
     - Details
   * - **TRS MIDI In 1**
     - TRS 3.5 mm Type-A input for keyboards, sequencers, and controllers
       (see `minimidi.world <https://minimidi.world/>`_ for the Type-A standard)
   * - **TRS MIDI In 2 / Clock**
     - Second TRS 3.5 mm Type-A input; doubles as Clock/Reset input for
       Eurorack and modular sync (use a 3.5 mm splitter cable for separate
       Clock and Reset)
   * - **TRS MIDI Out 1**
     - TRS 3.5 mm Type-A output for sending MIDI to external gear
   * - **TRS MIDI Out 2**
     - Second TRS 3.5 mm Type-A output
   * - **USB MIDI**
     - Class-compliant USB MIDI via USB-C #1 (no drivers required)

All connections are class-compliant --- no special drivers are needed on any
operating system.


How MIDI Works on TBD-16
=========================

MIDI is processed by the **RP2350** front-end processor. The RP2350 handles
all hardware I/O and typically runs its own **generators, sequencers, and
arpeggiators** that control the DSP plugins on the ESP32-P4 via the SPI bus.

External MIDI messages (from TRS or USB) are also received by the RP2350 and
forwarded to the P4, where plugins can respond to:

- **Note On / Note Off** --- Play melodic synthesizers, trigger drum hits
- **Control Change (CC)** --- Map MIDI CCs to plugin parameters
- **Pitch Bend** --- Continuous pitch modulation
- **Program Change** --- Switch between presets or plugins
- **Clock** --- Tempo synchronization via MIDI clock

.. note::

   Some DSP plugins (such as **Bjorklund**) have their own built-in sequencers
   and generate rhythms autonomously, independent of the RP2350 firmware.

.. tip::

   For wireless tempo sync without MIDI cables, use
   :doc:`Ableton Link <10_ableton_link>` instead of MIDI clock.


Using MIDI with Plugins
========================

Plugins that support MIDI input respond to incoming messages automatically.
You can assign MIDI CCs to plugin parameters through the hardware interface
or the web interface.

Synthesizer plugins typically respond to:

- Note messages for pitch
- Velocity for dynamics
- CC messages for filter cutoff, resonance, and other parameters

Effect plugins can use MIDI to:

- Tap tempo via note messages
- Control wet/dry mix via CC
- Trigger freeze/hold functions via note on/off
