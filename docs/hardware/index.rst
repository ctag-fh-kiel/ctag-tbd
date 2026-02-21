Hardware
========

.. raw:: html

   <img src="../_static/assets/dadamachines-tbd-16_mockup_002.jpg" alt="dadamachines TBD-16" style="max-width: 100%; width: 100%; border-radius: 6px;">

The **TBD-16** is a compact desktop audio DSP instrument powered by an
`ESP32-P4 <https://www.espressif.com/en/products/socs/esp32-p4>`_ (400 MHz dual RISC-V)
and an `RP2350B <https://www.raspberrypi.com/products/rp2350/>`_ (150 MHz dual Cortex-M33).
Plug in power and start making music --- no computer or Eurorack case required.

.. list-table::
   :widths: 30 70

   * - **Audio**
     - 44.1 kHz · 32-bit float · stereo in/out · headphone out
   * - **MIDI**
     - 4× TRS (Type-A) · USB MIDI · USB Host MIDI · Ableton Link
   * - **Interface**
     - 30 buttons · 4 endless pots · 2.4" OLED · 19 RGB LEDs
   * - **Plugins**
     - 50+ synths, effects, drum machines

:doc:`Full TBD-16 specs and connectivity → <10_tbd16>`

.. raw:: html

   <div style="display:flex; gap:0.75em; flex-wrap:wrap; margin:1em 0 2.5em;">
     <a href="https://dadamachines.com/shop/" style="padding:0.6em 1.5em; background:var(--color-brand-primary, #6a5acd); color:#fff; border-radius:6px; text-decoration:none; font-weight:600;">Buy TBD-16</a>
     <a href="https://dadamachines.com" style="padding:0.6em 1.5em; border:1px solid var(--color-background-border, #ccc); border-radius:6px; text-decoration:none; font-weight:500;">dadamachines.com</a>
   </div>

----

The TBD Platform
----------------

The TBD-16 is the flagship product of the **TBD platform** --- a modular hardware
architecture built around the same ESP32-P4 DSP engine. Whether you want a ready-made
instrument, a core board for a custom enclosure, or a fully custom PCB, all tiers
run the same CTAG TBD audio engine with 50+ plugins, WiFi, MIDI, and Ableton Link.

.. list-table::
   :header-rows: 1
   :widths: 25 40 35

   * - Product
     - What It Is
     - For Whom
   * - :doc:`TBD-16 <10_tbd16>`
     - Complete desktop instrument, ready to play
     - Musicians, developers, anyone who wants to start immediately
   * - :doc:`TBD-Core <20_tbd_core>`
     - Core DSP board with all I/O assembled + 30-pin FFC for custom UI
     - Instrument designers, product developers
   * - :doc:`Custom Integration <30_custom_integration>`
     - ESP32-P4 + RP2350B + codec fully integrated on your own PCB
     - Manufacturers, OEMs, collaborators


Which Tier Is Right for You?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 30 23 24 23

   * - Requirement
     - TBD-16
     - TBD-Core
     - Custom Integration
   * - Ready to use out of the box
     - **Yes**
     - ---
     - ---
   * - Custom UI / enclosure
     - ---
     - **Yes** (via FFC)
     - **Yes** (full control)
   * - All I/O jacks on board
     - **Yes**
     - **Yes**
     - You choose
   * - Custom connector layout
     - ---
     - ---
     - **Yes**
   * - Custom codec / channels
     - ---
     - ---
     - **Yes**
   * - Optimized BOM at scale
     - ---
     - ---
     - **Yes**
   * - Same DSP engine & plugins
     - **Yes**
     - **Yes**
     - **Yes**


Proven Lineage
--------------

The TBD audio engine has been shipping in hardware for years across multiple form factors:

- **TBD-16** --- Current-generation desktop instrument by dadamachines
- **CTAG TBD Eurorack** --- The original open-source module
  (`upstream repo <https://github.com/ctag-fh-kiel/ctag-tbd>`_),
  commercially sold by
  `Instruments of Things <https://instrumentsofthings.com/products/tbd>`_
- **AE Modular TBD** --- Compact adaptation by
  `tangible waves <https://www.tangiblewaves.com/store/p149/TBD.html>`_
  (`wiki <https://wiki.aemodular.com/#/modules/tbd>`_)

There is no Eurorack module based on the new ESP32-P4 + RP2350 platform planned yet,
but dadamachines is open to partnering with manufacturers or developers who want to
build one. See :doc:`Custom Integration <30_custom_integration>`.

| `Contact dadamachines <https://dadamachines.com/contact/>`_ · `Shop <https://dadamachines.com/shop/>`_

.. toctree::
   :hidden:
   :glob:

   [0-9]*
