Frontend Development
====================

The RP2350 is the TBD-16's user-facing processor. It handles the display, LEDs,
buttons, encoders, and MIDI --- everything the player touches. Because it runs
Arduino on PlatformIO, you can prototype new interfaces in minutes.

:doc:`Getting Started <10_getting_started>`
   Install PlatformIO, flash the template firmware, and run your first build.

:doc:`RP2350 Firmware <20_rp2350>`
   Deep dive into the SPI protocol, MIDI integration, and control data flow
   between the RP2350 and the DSP engine.

:doc:`UF2 Bootloader & Apps <25_bootloader>`
   The TBD-16 can run multiple RP2350 firmware images from the SD card ---
   switch between your app, the drum machine, MCL, a debug probe, and more.

:doc:`Debugging with SWD <30_debugging>`
   Use a Raspberry Pi Debug Probe for one-click flashing, breakpoints, and
   serial output --- no BOOTSEL button needed.

.. toctree::
   :hidden:
   :glob:

   [0-9]*
