***********
Development
***********

This section covers the technical aspects of the TBD-16 platform, including build processes, 
hardware integration, and system architecture.

Overview
========

The TBD-16 is based on the **ESP32-P4** and **RP2350** microcontrollers. 
The core audio engine (DSP) runs on the ESP32-P4, while the UI and hardware 
interfacing are handled by the RP2350.

Topics
------

* :doc:`Building & Setup <01_building>` — How to compile and flash the firmware.
* :doc:`API Reference <10_api_reference>` — REST API endpoints for remote control.
* **Architecture** — Understanding the communication between the ESP32 and RP2350.
* **Plugin System** — How to extend the platform with new audio processors.
