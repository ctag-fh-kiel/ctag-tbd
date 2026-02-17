**********************
WiFi & Firmware Updates
**********************


WiFi
====

The TBD-16 includes a WiFi co-processor (ESP32-C6) that provides network
connectivity for the web interface used for **configuration and preset
management**.

.. note::

   WiFi is not required for making sound. The TBD-16 works fully standalone
   with its hardware interface (buttons, encoders, OLED display).

Initially, the TBD creates its own WiFi network with the SSID **ctag-tbd**
(no password). Join that network, open a browser and navigate to
``http://ctag-tbd.local``. This opens the configuration page where you can
manage presets, browse the plugin library, and adjust system settings.

From the **Edit configuration** page, you can adjust WiFi settings:

-  **WiFi mode:**

   -  **Access point (AP)** --- TBD creates its own network that you can
      join directly. No additional router is required. You can secure the
      network with a password.

   -  **Station (STA)** --- TBD joins an existing WiFi network.

-  **SSID** --- The network name TBD will create or join.

-  **Password** --- The password for the network.

-  **mDNS Name** --- The domain name used to access the TBD (default: ``ctag-tbd``).

**Example: Live setup with multiple TBDs**

1. Set one TBD as **AP** (access point).
2. Set other TBDs as **STA** and join the first TBD's network.
3. Access all TBDs through the primary TBD's network --- no external router needed.


Firmware Update
===============

The TBD-16 has two processors that need separate firmware:

-  **ESP32-P4** --- runs the audio DSP engine. Flashed via the JTAG USB-C port
   on the front panel using **WebSerial** (Chrome/Edge/Opera).
-  **RP2350** --- runs the UI, MIDI, and hardware interface. Flashed via the
   back USB-C port using **WebUSB** or UF2 drag-and-drop.

To update the firmware, use the browser-based flashers:

-  :doc:`DSP Firmware Flasher <25_flash_dsp>` --- Flash the audio DSP firmware.
-  :doc:`UI Firmware Flasher <30_flash_ui>` --- Flash the UI/MIDI firmware.

For a guided full re-flash, see :doc:`Device Recovery <50_device_recovery>`.

.. note::

   Firmware updates do not erase the SD card. Your patches, samples, and
   WiFi settings are preserved on the SD card and survive firmware updates.
