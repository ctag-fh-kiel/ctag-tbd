DSP Development
================

The ESP32-P4 runs the CTAG TBD audio engine --- a real-time plugin host that
calls your code 32 samples at a time at 44.1 kHz. Write a C++ class, register
it, and your plugin appears in the web interface ready to play.

:doc:`Getting Started <10_getting_started>`
   Install ESP-IDF, clone the repo, and build the firmware.

:doc:`Plugin Architecture <20_plugin_architecture>`
   How the engine discovers, loads, and calls plugins.

:doc:`Step-by-Step Plugin <30_step_by_step>`
   Create a plugin from scratch using the code generator.

:doc:`Building & Flashing <40_building>`
   Configure, compile, and flash the full DSP firmware.

:doc:`Simulator <50_simulator>`
   Test plugins on your desktop without hardware.

:doc:`Web UI API <60_web_api>`
   REST endpoints for controlling plugins over WiFi.

:doc:`USB Audio Interface <55_audio_interface>`
   Alternative firmware that turns the TBD-16 into a class-compliant USB sound card.

.. toctree::
   :hidden:
   :glob:

   [0-9]*
