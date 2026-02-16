######################
dadamachines TBD-16
######################

The first standalone desktop audio DSP platform based on `CTAG TBD <https://github.com/ctag-fh-kiel/ctag-tbd>`_,
with standard MIDI connectivity — designed to bring open-source audio processing beyond Eurorack.

**TBD-16** combines 50+ high-quality generators and effects in a modular, extensible architecture.
It is built for musicians, educators, and audio researchers who want hands-on DSP without proprietary lock-in.


What This Fork Does
===================

This repository is a fork of `ctag-fh-kiel/ctag-tbd <https://github.com/ctag-fh-kiel/ctag-tbd>`_ (branch ``p4_main``),
adapted for the **dadamachines TBD-16** hardware. Our focus:

- **UI/UX** — Redesigned web interface with musician-friendly interaction patterns
- **Documentation** — Clear guides, example workflows, and UX guidelines for plugin developers
- **Desktop Hardware** — Standalone form factor with standard MIDI, no Eurorack required

The DSP engine, plugin system, and core firmware are developed upstream by
`Robert Manzke / CTAG <https://www.creative-technologies.de/>`_.


For Users
=========

Your TBD-16 ships ready to use. Start with :doc:`Using Your TBD-16 <get_started/10_using_your_tbd>` ---
connect, open the web interface, and start making sound.

**Key Features:**

-  :doc:`Ableton Link <features/10_ableton_link>` --- Sync tempo wirelessly with Ableton Live,
   iOS apps, and other Link-enabled devices on the same network.
-  :doc:`MIDI <features/30_midi>` --- Full MIDI support via DIN and USB for playing and controlling plugins.
-  :doc:`USB Audio Interface <features/20_audio_interface>` --- Hardware support for
   class-compliant USB audio (custom firmware required).

For Developers
==============

-  **Create Plugins** --- :doc:`Plugin Architecture <create_plugins/10_prerequisites>` explains the
   DSP plugin system, and the :doc:`Step-by-Step Guide <create_plugins/20_step_by_step>` walks you
   through creating your first plugin.
-  **Simulator** --- :doc:`Run the full DSP engine on your computer <development/05_simulator>`
   without any hardware. Develop, test, and iterate on plugins entirely in software.
-  **RP2350 Firmware** --- :doc:`RP2350 Development <development/20_rp2350>` covers building custom
   UI, MIDI, and sequencer firmware for the front-end processor.
-  **Build from Source** --- :doc:`Building & Setup <development/01_building>` for compiling the
   ESP32-P4 firmware.

For Manufacturers & Designers
==============================

Beyond the TBD-16 instrument, dadamachines offers hardware for custom products:

-  :doc:`TBD Main PCB <hardware/20_main_pcb>` --- Core board with 30-pin FFC connector for
   custom UI boards and enclosures.
-  :doc:`Custom Integration <hardware/30_custom_integration>` --- Full ESP32-P4 + RP2350 + Codec
   integration on your PCB, with expert hardware and industrial design support.

Contributing
============

Contributions are welcome. Please open an issue or pull request on
`GitHub <https://github.com/dadamachines/ctag-tbd>`_.


.. toctree::
    :hidden:
    :caption: Get Started
    :maxdepth: 4
    :glob:

    get_started/*


.. toctree::
    :hidden:
    :caption: Features
    :maxdepth: 2
    :glob:

    features/*


.. toctree::
    :hidden:
    :caption: Hardware
    :maxdepth: 2
    :glob:

    hardware/*


.. toctree::
    :hidden:
    :caption: Plugins
    :maxdepth: 2
    :glob:

    sound_library/*


.. toctree::
    :hidden:
    :caption: Create Plugins
    :maxdepth: 2
    :glob:

    create_plugins/*


.. toctree::
    :hidden:
    :caption: Development
    :maxdepth: 2
    :glob:

    development/*


.. toctree::
    :hidden:
    :caption: About
    :maxdepth: 2
    :glob:

    about/*


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
