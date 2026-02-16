*****************
Hardware Versions
*****************

.. note::

   This documentation focuses on the **dadamachines TBD-16** standalone desktop hardware.
   For information on other CTAG TBD hardware variants (Eurorack MK1/MK2, Tangible Waves AEM,
   Straempler), see the upstream `CTAG TBD repository <https://github.com/ctag-fh-kiel/ctag-tbd>`_.


dadamachines TBD-16
===================

The TBD-16 is the first standalone desktop version of the TBD platform.

Processors
----------

-  **ESP32-P4** --- Audio DSP engine, WiFi web server, plugin management.
-  **RP2350** --- User interface, MIDI, hardware I/O, optional sequencer.
-  **ESP32-C6** --- WiFi co-processor (managed by the P4).

Connectivity
------------

-  **MIDI** --- Standard MIDI In/Out via DIN or TRS (no Eurorack required).
-  **Audio** --- Stereo output, headphone output.
-  **USB** --- JTAG (front, for P4 flashing), 2x USB-C (back, for power/RP2350/USB-MSC).
-  **WiFi** --- Web-based control interface via the C6 co-processor.
-  **SD Cards** --- Two slots: one for P4 (config, samples, web UI), one for RP2350.
