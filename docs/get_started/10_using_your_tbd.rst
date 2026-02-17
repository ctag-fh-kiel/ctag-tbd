*********************
Using Your TBD-16
*********************

Your TBD-16 ships ready to use. Just plug in power and start making music ---
no computer, no WiFi, no setup required.


1. Power On
===========

- Plug a **USB-C cable into USB-C port #1 or #2** on the back of the TBD-16.
  Any USB-C power source works --- a laptop, a wall adapter, or a USB power bank.
- Connect your audio output (speakers, headphones, mixer) to the audio jacks.
- Optionally connect MIDI gear via the TRS MIDI ports or USB MIDI.

The TBD-16 boots in a few seconds. The OLED display and RGB LEDs come alive,
and the device loads the default **Groovebox / Drum Machine** firmware with
**16 tracks**.

.. tip::

   The TBD-16 works great on the go with a USB power bank. dadamachines offers
   a `power bank mount <https://www.amazon.de/dp/B0BCK5Y852>`_ that attaches
   to the back of the device.


2. Start Making Music
=====================

The hardware interface gives you everything you need:

- **30 buttons** --- Select tracks, trigger pads, control transport and patterns
- **4 endless potentiometers** (with push-switch) --- Dial in parameters with
  360-degree rotation
- **2.4" OLED display** (128 × 64 px) --- Shows track state, menus, parameters
- **19 RGB LEDs** --- Visual feedback for track state, sequencing, and modes

Just start pressing buttons and turning knobs --- the 16-track drum machine is
ready to go out of the box.


3. Firmware Modes
=================

The TBD-16 ships with two main firmware modes on the RP2350:

**Groovebox / Drum Machine** (default)
   A 16-track drum machine and groovebox. The RP2350 runs sequencers,
   arpeggiators, and pattern generators that control the DSP plugins on the
   ESP32-P4. Start making beats immediately.

**MultiEffect**
   Lets you select which DSP plugins run in Slot 0 and Slot 1 of the
   Sound Processor. Control everything from the hardware interface or via
   external MIDI (USB or TRS).

You switch between firmware images using the UF2 bootloader --- see
:doc:`UF2 Bootloader & Apps </frontend/25_bootloader>` for details.

.. note::

   Some DSP plugins also have their own built-in sequencers (for example, the
   **Bjorklund** euclidean rhythm generator). These are self-contained and work
   independently of the RP2350 firmware mode.


4. Connect MIDI (Optional)
===========================

Use the TRS MIDI ports (Type-A) or USB MIDI to connect keyboards, sequencers,
or other MIDI gear. The RP2350 processes incoming MIDI and forwards it to the
DSP engine for chromatic playing, parameter control, or clock sync.

See :doc:`MIDI </hardware/15_midi>` for full details.


5. Web Interface (Optional)
============================

The TBD-16 also has a WiFi-based web interface for **configuration and preset
management**. It is *not* required for making sound --- everything can be
controlled from the hardware.

Use the web interface when you want to:

- Manage and save presets
- Browse the full plugin library
- Configure WiFi, MIDI mapping, and system settings
- Update firmware over the air

See :doc:`WiFi & Ableton Link <15_wifi_and_link>` to set up the
connection.


What's Next
===========

- :doc:`WiFi & Ableton Link <15_wifi_and_link>` --- Connect to your network, access web interface, sync tempo
- :doc:`Storage & Samples <20_storage>` --- Manage SD card content and audio samples
- :doc:`Plugin Architecture </dsp/20_plugin_architecture>` --- Learn how plugins work or create your own
