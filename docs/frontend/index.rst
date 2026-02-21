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

:doc:`Apps </apps/index>`
   The TBD-16 runs Apps -- each one a complete firmware that changes what the
   device does. Groovebox, Multi Effect/Synth, MCL, utilities, and more.
   See the Apps catalog for screenshots and details.

:doc:`Bootloader </apps/bootloader>`
   How the boot menu loads and switches Apps from the SD card. Technical
   details on the UF2 partition, SD card layout, and BOOTSEL mode.

:doc:`Debugging with SWD <30_debugging>`
   Use a Raspberry Pi Debug Probe for one-click flashing, breakpoints, and
   serial output --- no BOOTSEL button needed.

.. toctree::
   :hidden:
   :glob:

   [0-9]*
