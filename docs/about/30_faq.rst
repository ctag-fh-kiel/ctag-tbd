***
FAQ
***

General
=======

What is TBD-16?
---------------

TBD-16 is a standalone desktop audio DSP platform by `dadamachines <https://dadamachines.com>`_.
It runs the open-source `CTAG TBD <https://github.com/ctag-fh-kiel/ctag-tbd>`_ audio engine
and includes 50+ synthesizers and effects that you control through a browser-based interface.

What does TBD stand for?
------------------------

**To Be Determined** --- reflecting the platform's open, extensible nature.
The sound engine is not fixed; you choose and combine processors from a growing library,
or create your own.

Do I need a Eurorack system?
----------------------------

No. The TBD-16 is a standalone desktop device with standard MIDI (DIN + USB) and
audio I/O. No Eurorack case or power supply needed.

What processors / chips are inside?
------------------------------------

- **ESP32-P4** --- Main DSP engine and WiFi web server
- **RP2350** --- User interface, MIDI, and hardware I/O
- **ESP32-C6** --- WiFi co-processor

See :doc:`TBD-16 Hardware </hardware/10_tbd16>` for more details.

Can I use TBD-16 in a commercial product?
-----------------------------------------

The software is licensed under GPL 3.0. You may use, modify, and distribute it
under the terms of that license. The dadamachines TBD-16 hardware design itself
is proprietary. See :doc:`Credits & License <10_credits>` for full license details.


Audio & DSP
===========

What sample rate and bit depth does TBD-16 use?
------------------------------------------------

The default configuration runs at **44.1 kHz, 32-bit float** internal processing.

How many plugins can run simultaneously?
----------------------------------------

Two --- one per audio channel. Each channel independently runs a plugin from the
library. Plugins are selected through the web UI.

Can I process external audio?
-----------------------------

Yes. The audio inputs accept line-level signals. Any effect plugin can process
incoming audio in real time.

What is Ableton Link?
---------------------

Ableton Link is a technology for synchronizing tempo across devices on the same
WiFi network. TBD-16 has built-in Link support, allowing it to sync with
Ableton Live, iOS apps, and other Link-enabled tools. See
:doc:`Ableton Link </features/10_ableton_link>` for details.


Connectivity
============

How do I connect to the web interface?
--------------------------------------

Connect the TBD-16 to your WiFi network, then open its IP address in a browser.
See :doc:`WiFi & Firmware Updates </get_started/15_wifi_and_updates>` for setup instructions.

What MIDI connections are supported?
------------------------------------

- 5-pin DIN MIDI In/Out
- USB MIDI (via USB-C)

The RP2350 processor handles MIDI and makes it available to the DSP engine.


Development
===========

Can I create my own plugins?
-----------------------------

Yes. The plugin system is fully documented:

- :doc:`Plugin Architecture </create_plugins/10_prerequisites>` --- How the system works
- :doc:`Step-by-Step Guide </create_plugins/20_step_by_step>` --- Creating your first plugin

Do I need the hardware to develop plugins?
------------------------------------------

No. The :doc:`Simulator </development/05_simulator>` runs the full DSP engine on your
computer (macOS, Windows, Linux). You can develop and test plugins entirely in
software, then flash to hardware when ready.

What language are plugins written in?
-------------------------------------

C++. The plugin base class ``ctagSoundProcessor`` provides the framework.
A code generator (``generator.js``) creates boilerplate from a JSON parameter
definition file.


Troubleshooting
===============

My new plugin doesn't appear in the web UI
-------------------------------------------

The system caches the list of available processors in ``spm-config.jsn``.
Delete the ``"availableProcessors": [ ... ],`` block from that file and
restart the device or simulator. See the
:doc:`Simulator Guide </development/05_simulator>` for details.

I can't connect to the web interface
-------------------------------------

1. Ensure the TBD-16 is powered on and connected to your WiFi network.
2. Check that your computer is on the **same network**.
3. Try accessing the device at its IP address directly.
4. If the device isn't connecting to WiFi, see :doc:`WiFi & Firmware Updates </get_started/15_wifi_and_updates>`
   for configuration instructions.

Firmware flash failed
---------------------

- Make sure the USB cable supports data (not charge-only).
- Try a different USB port.
- Use the browser-based flasher at :doc:`Flash DSP Firmware </get_started/25_flash_dsp>` or
  :doc:`Flash UI Firmware </get_started/30_flash_ui>`.
- If flashing via ``idf.py flash``, ensure ESP-IDF is properly installed
  (see :doc:`Building & Setup </development/01_building>`).

Build errors about missing submodules
--------------------------------------

Run:

.. code-block:: bash

   git submodule update --init --recursive

This fetches all required submodules including the Ableton Link SDK and esp-dsp.

Build error about missing ``xxh128sum``
---------------------------------------

Install xxhash:

.. code-block:: bash

   # macOS
   brew install xxhash

   # Debian/Ubuntu
   sudo apt-get install xxhash
