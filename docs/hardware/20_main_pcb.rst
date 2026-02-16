**************
TBD Main PCB
**************

The TBD Main PCB is the core board from the TBD-16, available separately for
instrument designers and product developers who want to build custom hardware
around the TBD platform.

It bridges the gap between the ready-to-use TBD-16 instrument and a fully custom
PCB integration --- giving you a proven, production-grade DSP platform with the
freedom to design your own user interface and enclosure.


What You Get
============

The Main PCB includes:

- **ESP32-P4** --- DSP engine, WiFi web server, plugin management
- **RP2350** --- MIDI processing, hardware I/O, sequencer capabilities
- **ESP32-C6** --- WiFi co-processor
- **Audio codec** --- High-quality stereo ADC/DAC
- **Power management** --- Regulated power for all components
- **SD card slots** --- Two slots (P4 + RP2350)
- **30-pin FFC connector** --- Flat flex cable interface for custom UI boards


The FFC Connector
=================

The 30-pin FFC connector breaks out the signals you need for a custom user interface:

- **GPIO** for buttons, encoders, LEDs, displays
- **SPI/I2C** for additional peripherals
- **Analog inputs** for potentiometers, CV jacks
- **Power rails** for your UI board

This lets you design a custom front panel, control surface, or enclosure while
reusing the entire TBD audio/processing platform without modification.


Use Cases
=========

- **Custom instruments** --- Design a guitar pedal, desktop synth, or performance
  controller with your own panel layout and controls
- **Product prototyping** --- Validate a product concept quickly using the proven
  TBD platform before committing to a custom PCB
- **Educational platforms** --- Build teaching tools with custom interfaces tailored
  to specific learning objectives
- **Art installations** --- Embed DSP processing in interactive or generative
  sound installations with custom enclosures


How It Differs from TBD-16
==========================

.. list-table::
   :header-rows: 1
   :widths: 40 30 30

   * - Feature
     - TBD-16
     - Main PCB
   * - Enclosure & front panel
     - Included
     - You design your own
   * - Knobs, buttons, display
     - Included
     - Via FFC to your UI board
   * - Audio I/O jacks
     - Included
     - On your carrier board
   * - MIDI DIN connectors
     - Included
     - On your carrier board
   * - DSP engine & firmware
     - Same
     - Same
   * - Plugin library
     - Same
     - Same


Getting Started
===============

If you're interested in building with the TBD Main PCB, contact
`dadamachines <https://dadamachines.com>`_ to discuss your project.

For development, the software workflow is identical to TBD-16:

- :doc:`Building & Setup </dsp/30_building>` --- Compile firmware
- :doc:`Simulator </dsp/40_simulator>` --- Develop plugins without hardware
- :doc:`RP2350 Firmware </frontend/10_rp2350>` --- Customize the front-end processor
