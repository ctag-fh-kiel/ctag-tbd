********
TBD-Core
********

.. raw:: html

   <!-- Hero image placeholder — replace src with actual product photo -->
   <div style="text-align:center; margin: 0 0 2em 0;">
     <img src="https://docs.dadamachines.com/images/tbd-core-hero-placeholder.jpg"
          alt="dadamachines TBD-Core"
          style="width:100%; max-width:720px; border-radius:8px;" />
   </div>

The TBD-Core is the core board from the TBD-16, available separately for
instrument designers and product developers who want to build custom hardware
around the TBD platform.

It bridges the gap between the ready-to-use TBD-16 instrument and a fully custom
PCB integration --- giving you a proven, production-grade DSP platform with the
freedom to design your own user interface and enclosure.

.. raw:: html

   <!-- Quick-nav jump links -->
   <div style="display:flex; gap:0.75em; flex-wrap:wrap; margin:1.5em 0 2em;">
     <a href="#technical-specifications" style="padding:0.5em 1.2em; border:1px solid var(--color-background-border, #ccc); border-radius:6px; text-decoration:none; font-weight:500;">Tech Specs</a>
     <a href="#ffc-connector" style="padding:0.5em 1.2em; border:1px solid var(--color-background-border, #ccc); border-radius:6px; text-decoration:none; font-weight:500;">FFC Pinout</a>
     <a href="#assets" style="padding:0.5em 1.2em; border:1px solid var(--color-background-border, #ccc); border-radius:6px; text-decoration:none; font-weight:500;">Assets</a>
     <a href="#getting-started" style="padding:0.5em 1.2em; border:1px solid var(--color-background-border, #ccc); border-radius:6px; text-decoration:none; font-weight:500;">Getting Started</a>
     <a href="https://dadamachines.com/shop/" style="padding:0.5em 1.2em; border:1px solid var(--color-background-border, #ccc); border-radius:6px; text-decoration:none; font-weight:500;">Buy</a>
   </div>


Technical Specifications
========================

The TBD-Core shares the same processors, audio codec, and connectivity as the
TBD-16. Everything below is on the board --- no carrier board required for audio
or MIDI.

.. list-table::
   :widths: 30 70
   :header-rows: 0

   * - **DSP Processor**
     - `ESP32-P4 <https://www.espressif.com/en/products/socs/esp32-p4>`_ --- Dual-core RISC-V @ 400 MHz · 32 MB stacked PSRAM · 16 MB Flash
   * - **UI / MIDI Processor**
     - `RP2350B <https://www.raspberrypi.com/products/rp2350/>`_ --- Dual Arm Cortex-M33 @ 150 MHz · 520 KB SRAM · 48 GPIO · FPU + DSP
   * - **WiFi Co-Processor**
     - ESP32-C6 (managed by the P4)
   * - **Audio Codec**
     - `TLV320AIC3254 <https://www.ti.com/product/en-us/TLV320AIC3254/part-details/TLV320AIC3254IRHBR>`_ --- Stereo ADC/DAC
   * - **Sample Rate**
     - 44.1 kHz
   * - **Bit Depth**
     - 32-bit float (internal processing)
   * - **Audio I/O**
     - Stereo line in · stereo line out · headphone out (all TRS 3.5 mm, assembled on PCB)
   * - **MIDI**
     - 2× TRS In (Type-A) · 2× TRS Out · USB MIDI · USB Host MIDI (assembled on PCB)
   * - **USB Ports**
     - 3× USB-C · 1× USB-A Host
   * - **Wireless**
     - WiFi (ESP32-C6) · Ableton Link
   * - **Storage**
     - 2× micro SD (P4 + RP2350)
   * - **Power**
     - USB-C (5 V)
   * - **UI Expansion**
     - 30-pin FFC connector for custom UI boards
   * - **Dimensions**
     - *Coming soon*

.. note::
   All audio, MIDI, and USB connectors come **assembled on the TBD-Core PCB** ---
   identical to the TBD-16. If you need a different I/O layout or custom
   connectors, see :doc:`Custom Integration <30_custom_integration>`.


What's on the Board
====================

.. list-table::
   :header-rows: 1
   :widths: 20 20 60

   * - Chip
     - Role
     - Key Specs
   * - **ESP32-P4**
     - DSP Engine
     - Dual RISC-V @ 400 MHz · 32 MB stacked PSRAM · 16 MB Flash · AI/vector instructions
   * - **RP2350B**
     - UI / MIDI
     - Dual Arm Cortex-M33 @ 150 MHz · 520 KB SRAM · 48 GPIO · 8× 12-bit ADC · 12× PIO state machines · FPU + DSP instructions
   * - **ESP32-C6**
     - WiFi
     - WiFi 6 (802.11ax) · Bluetooth 5 · managed by the P4

- **Audio codec** (`TLV320AIC3254 <https://www.ti.com/product/en-us/TLV320AIC3254/part-details/TLV320AIC3254IRHBR>`_) --- Stereo ADC/DAC
- **Power management** --- Regulated power for all components
- **SD card slots** --- Two slots (P4 + RP2350)
- **All I/O jacks** --- Audio, MIDI, USB, and headphone connectors
- **30-pin FFC connector** --- Flat flex cable interface for custom UI boards


How It Differs from TBD-16
==========================

.. list-table::
   :header-rows: 1
   :widths: 40 30 30

   * - Feature
     - TBD-16
     - TBD-Core
   * - Enclosure & front panel
     - Included
     - You design your own
   * - Knobs, buttons, display
     - Included (via UI board + FFC)
     - Via FFC to your UI board
   * - Audio, MIDI, USB jacks
     - Included
     - Included (same as TBD-16)
   * - DSP engine & firmware
     - Same
     - Same
   * - Plugin library (50+)
     - Same
     - Same

The TBD-16 includes a separate **UI board** (connected via FFC) with an
**STM32F030R8T6** I/O controller that reads the buttons, 360-degree endless
potentiometers, and other inputs via I2C
(`firmware source <https://github.com/ctag-fh-kiel/stm32-tbd-fw>`_).
With the TBD-Core, you design your own UI board to connect through the same FFC.


FFC Connector
=============

The 30-pin FFC connector breaks out the signals you need for a custom user interface:

- **GPIO** for buttons, encoders, LEDs, displays
- **SPI/I2C** for additional peripherals
- **Analog inputs** for potentiometers, CV jacks
- **Power rails** for your UI board

This lets you design a custom front panel, control surface, or enclosure while
reusing the entire TBD audio/processing platform without modification.

.. note::
   **Full FFC pinout table coming soon.** A detailed pin-by-pin table with signal
   names, directions, voltage levels, and recommended use will be published here.

..
   TODO: Insert FFC pinout table. Format:
   Pin | Signal Name | Direction | Voltage | Description
   1   | ...         | ...       | ...     | ...
   ...


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


Assets
======

.. note::
   Downloadable assets will be published here as they become available.

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Asset
     - Status
   * - Datasheet PDF
     - *Coming soon*
   * - FFC Pinout (table + PDF)
     - *Coming soon*
   * - Simplified 3D Model
     - *Coming soon*
   * - KiCad Reference Design
     - *Coming soon* (open-source core for the TBD platform)

..
   Future assets:
   - Datasheet PDF
   - FFC Pinout PDF
   - 3D Model (simplified, STEP)
   - KiCad reference design (open-source core)


Getting Started
===============

Interested in building with the TBD-Core?
`Contact dadamachines <https://dadamachines.com/contact/>`_ to discuss your project
and availability.

The software workflow is identical to the TBD-16:

- :doc:`DSP --- Getting Started </dsp/10_getting_started>` --- Compile DSP firmware
- :doc:`Simulator </dsp/50_simulator>` --- Develop plugins without hardware
- :doc:`Frontend --- Getting Started </frontend/10_getting_started>` --- Build custom UI / MIDI firmware
- :doc:`Flash & Updates </flash/index>` --- Flash firmware from your browser

----

.. raw:: html

   <div style="display:flex; gap:0.75em; flex-wrap:wrap; margin:1.5em 0;">
     <a href="https://dadamachines.com/shop/" style="padding:0.6em 1.5em; background:var(--color-brand-primary, #6a5acd); color:#fff; border-radius:6px; text-decoration:none; font-weight:600;">Buy TBD-Core</a>
     <a href="https://dadamachines.com/contact/" style="padding:0.6em 1.5em; border:1px solid var(--color-background-border, #ccc); border-radius:6px; text-decoration:none; font-weight:500;">Contact dadamachines</a>
   </div>
