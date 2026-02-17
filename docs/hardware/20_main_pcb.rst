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
- **RP2350** --- MIDI processing, user interface, sequencers, arpeggiators, generators
- **ESP32-C6** --- WiFi co-processor
- **Audio codec** --- High-quality stereo ADC/DAC
- **Power management** --- Regulated power for all components
- **SD card slots** --- Two slots (P4 + RP2350)
- **30-pin FFC connector** --- Flat flex cable interface for custom UI boards

The TBD-16 also includes a separate **UI board** (connected via FFC) with an
**STM32F030R8T6** I/O controller that reads the buttons, 360-degree endless
potentiometers, and other inputs via I2C
(`firmware source <https://github.com/ctag-fh-kiel/stm32-tbd-fw>`_).


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
   * - MIDI connectors
     - Included (4x TRS 3.5 mm)
     - On your carrier board
   * - DSP engine & firmware
     - Same
     - Same
   * - Plugin library
     - Same
     - Same


Getting Started
===============

Interested in building with the TBD Main PCB?
`Contact dadamachines <https://dadamachines.com/contact/>`_ to discuss your project
and availability.

The software workflow is identical to the TBD-16:

- :doc:`DSP --- Getting Started </dsp/10_getting_started>` --- Compile DSP firmware
- :doc:`Simulator </dsp/50_simulator>` --- Develop plugins without hardware
- :doc:`Frontend --- Getting Started </frontend/10_getting_started>` --- Build custom UI / MIDI firmware
