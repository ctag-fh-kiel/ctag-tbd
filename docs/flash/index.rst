Flash
=====

The TBD-16 ships with all 50+ DSP plugins in a single firmware --- you do
**not** need to flash every time you want a different sound. Just pick a plugin
from the hardware interface or the web UI.

The RP2350 front-end also supports **multiple firmware images** on its SD card,
switchable from the boot menu without reflashing (see
:doc:`UF2 Bootloader & Apps </frontend/25_bootloader>`).

**When you do need to flash:**

- A new firmware release is available
- You want to run a custom build
- Something went wrong and you need to recover

:doc:`Flash DSP Firmware <25_flash_dsp>`
   Re-flash the ESP32-P4 audio engine from your browser --- no tools needed.

:doc:`Flash UI Firmware <30_flash_ui>`
   Re-flash the RP2350 front-end from your browser via WebUSB.

:doc:`Device Recovery <50_device_recovery>`
   Step-by-step guide to fully re-initialize your TBD-16 from scratch.

.. toctree::
   :hidden:
   :glob:

   [0-9]*
