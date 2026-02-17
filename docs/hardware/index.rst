Hardware
========

The **TBD-16** is a standalone desktop audio DSP instrument --- a premium devkit
and a complete instrument in one box.

.. list-table::
   :widths: 30 70

   * - **Processors**
     - ESP32-P4 (DSP) · RP2350 (UI/MIDI) · ESP32-C6 (WiFi)
   * - **Interface**
     - 30 buttons · 4 endless pots · 2.4" OLED · 19 RGB LEDs
   * - **Audio**
     - Stereo in · stereo out · headphone out (all TRS 3.5 mm) · 44.1 kHz / 32-bit float
   * - **MIDI**
     - 2× TRS In (Type-A) · 2× TRS Out · USB MIDI
   * - **USB**
     - 3× USB-C (P4 Device · Power/RP2350 · JTAG)
   * - **Plugins**
     - 50+ synths, effects, drum machines in one firmware

:doc:`Full TBD-16 specs → <10_tbd16>`

| `Shop <https://dadamachines.com/shop/>`_ · `dadamachines.com <https://dadamachines.com>`_

----

Platform Options
----------------

The TBD platform is modular. Beyond the TBD-16, dadamachines offers two
additional paths for instrument designers and manufacturers:

.. list-table::
   :header-rows: 1
   :widths: 25 40 35

   * - Product
     - What It Is
     - For Whom
   * - :doc:`TBD Main PCB <20_main_pcb>`
     - Core DSP board with 30-pin FFC for custom UI
     - Instrument designers, product developers
   * - :doc:`Custom Integration <30_custom_integration>`
     - ESP32-P4 + RP2350 + Codec on your own PCB
     - Manufacturers, OEMs, collaborators

All tiers run the same CTAG TBD audio engine with 50+ plugins, WiFi, MIDI,
and Ableton Link.

| `Contact dadamachines <https://dadamachines.com/contact/>`_ · `Shop <https://dadamachines.com/shop/>`_

.. toctree::
   :hidden:
   :glob:

   [0-9]*
