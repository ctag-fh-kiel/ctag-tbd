***
FAQ
***

General
=======

.. dropdown:: What is TBD-16?

   TBD-16 is a standalone desktop audio DSP instrument by
   `dadamachines <https://dadamachines.com>`_.
   It runs the open-source `CTAG TBD <https://github.com/ctag-fh-kiel/ctag-tbd>`_
   audio engine and includes 50+ synthesizers and effects. Just plug in USB-C
   power and start making music using the built-in hardware interface (buttons,
   encoders, OLED display, RGB LEDs) --- no computer required.

.. dropdown:: What does TBD stand for?

   **To Be Determined** --- reflecting the platform's open, extensible nature.
   The sound engine is not fixed; you choose and combine processors from a
   growing library, or create your own.

.. dropdown:: What is the relationship between CTAG TBD and dadamachines?

   `CTAG TBD <https://github.com/ctag-fh-kiel/ctag-tbd>`_ is the open-source
   audio engine developed at the Creative Technologies AG (CTAG) at Kiel
   University of Applied Sciences.
   `dadamachines <https://dadamachines.com>`_ builds the TBD-16 hardware product
   around that engine and maintains this documentation.

.. dropdown:: Do I need a Eurorack system?

   No. The TBD-16 is a fully standalone desktop device with TRS MIDI
   (3.5 mm Type-A), USB MIDI, audio I/O, and a full hardware interface.
   No Eurorack case, no computer, no power supply needed --- just USB-C
   power. It even works with a USB power bank for portable use.

.. dropdown:: What processors / chips are inside?

   - **ESP32-P4** --- Main DSP engine and WiFi web server
   - **RP2350** --- User interface, MIDI, and hardware I/O
   - **ESP32-C6** --- WiFi co-processor
   - **STM32F030R8T6** --- UI board I/O controller (buttons, encoders, on the
     separate UI board connected via FFC)

   See :doc:`TBD-16 Hardware <hardware/10_tbd16>` for more details.

.. dropdown:: Can I use TBD-16 in a commercial product?

   Yes. You can buy TBD-16 units and integrate them into your commercial
   product just like any off-the-shelf component.

   The core DSP engine is licensed under **GPL 3.0** (upstream). The
   dadamachines additions (web UI, tools, documentation) are licensed under
   **LGPL 3.0**, which is more permissive --- individual developers can
   contribute freely, while companies must share back any modifications
   they distribute.

   If you want to keep modifications proprietary, dadamachines offers
   commercial licensing. The TBD-16 hardware design itself is proprietary.

   See :doc:`Open-Source Licenses <about/10_credits>` for full details, or
   :doc:`Custom Integration </hardware/30_custom_integration>` if you want
   to build on the TBD platform directly.


Audio & Sound
=============

.. dropdown:: What sample rate and bit depth does TBD-16 use?

   The default configuration runs at **44.1 kHz, 32-bit float** internal
   processing.

.. dropdown:: How many plugins can run simultaneously?

   Two --- one per audio channel (Slot 0 and Slot 1). Each channel independently
   runs a plugin from the library. In **Groovebox** mode, plugins are assigned
   per track. In **MultiEffect** mode, you select plugins for each slot using
   the hardware interface or the web interface.

.. dropdown:: Can I process external audio?

   Yes. The audio inputs accept line-level signals. Any effect plugin can
   process incoming audio in real time.

.. dropdown:: What is Ableton Link?

   Ableton Link is a technology for synchronizing tempo across devices on the
   same WiFi network. TBD-16 has built-in Link support, allowing it to sync
   with Ableton Live, iOS apps, and other Link-enabled tools. See
   :doc:`Ableton Link <get_started/15_wifi_and_link>` for details.


Connectivity
============

.. dropdown:: How do I connect to the web interface?

   Connect the TBD-16 to your WiFi network, then open its IP address in a
   browser. See :doc:`WiFi & Ableton Link <get_started/15_wifi_and_link>`
   for setup instructions.

.. dropdown:: What MIDI connections are supported?

   - 2x TRS 3.5 mm MIDI In Type-A (2nd input doubles as Clock/Reset)
   - 2x TRS 3.5 mm MIDI Out Type-A
   - USB MIDI (via USB-C)

   All connections follow the `TRS Type-A standard <https://minimidi.world/>`_.
   The RP2350 processor handles MIDI and makes it available to the DSP engine.
   See the `MIDI section <hardware/10_tbd16.html#midi>`_ of the TBD-16 page for details.

.. dropdown:: Can the TBD-16 run multiple RP2350 firmware images?

   Yes. The UF2 bootloader stores multiple ``.uf2`` apps on the RP2350's SD
   card and lets you switch between them from a boot menu. See
   :doc:`UF2 Bootloader & Apps <frontend/25_bootloader>` for details.


Troubleshooting
===============

.. dropdown:: I can't connect to the web interface
   :open:

   1. Ensure the TBD-16 is powered on and connected to your WiFi network.
   2. Check that your computer is on the **same network**.
   3. Try accessing the device at its IP address directly.
   4. If the device isn't connecting to WiFi, see
      :doc:`WiFi & Ableton Link <get_started/15_wifi_and_link>`.

.. dropdown:: Firmware flash failed

   - Make sure the USB cable supports data (not charge-only).
   - Try a different USB port.
   - Use the browser-based flasher at
     :doc:`Flash DSP Firmware </flash/25_flash_dsp>` or
     :doc:`Flash UI Firmware </flash/30_flash_ui>`.
   - For full device recovery, see
     :doc:`Device Recovery </flash/50_device_recovery>`.

.. dropdown:: No sound after changing plugins

   - Check that the plugin is loaded on the correct channel.
   - Some plugins require an external audio input or a MIDI trigger to produce
     sound.
   - In Groovebox mode, make sure the track is unmuted and the sequencer is
     running.
   - Verify that your audio cable is connected to the correct output jack.


Development Questions
=====================

If you're developing custom firmware or plugins, the relevant troubleshooting
is in each development section:

- **DSP / plugin development** --- :doc:`DSP Getting Started <dsp/10_getting_started>`
  (Troubleshooting section) and :doc:`Building & Setup <dsp/40_building>`
- **Frontend / RP2350 development** --- :doc:`Frontend Getting Started <frontend/10_getting_started>`
  and :doc:`Debugging with SWD <frontend/30_debugging>` (Troubleshooting section)
