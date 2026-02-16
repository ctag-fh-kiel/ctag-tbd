***********
Development
***********

This section covers the technical aspects of the TBD-16 platform for developers
who want to build firmware, create plugins, or contribute to the project.

Architecture
============

The TBD-16 is a dual-processor system:

-  **ESP32-P4** --- Runs the audio DSP engine (CTAG TBD core), WiFi web server,
   and plugin management. This is the main firmware in this repository.
-  **RP2350** --- Runs the user interface, MIDI processing, hardware I/O, and
   optional sequencer. Communicates with the P4 via SPI.

The two processors communicate through a SPI bus. The RP2350 sends control data
(knob positions, button states, MIDI messages) to the P4, and the P4 sends
status information back.

Topics
------

*  :doc:`Building & Setup <01_building>` --- Compile and flash the ESP32-P4 firmware.
*  :doc:`API Reference <10_api_reference>` --- REST API for remote control via WiFi.
*  :doc:`RP2350 Firmware <20_rp2350>` --- Developing UI/MIDI firmware for the RP2350.
