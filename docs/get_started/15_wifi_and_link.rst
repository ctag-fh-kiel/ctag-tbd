*********************
WiFi & Ableton Link
*********************

The TBD-16 uses WiFi for two things: the **web interface** (configuration and
preset management) and **Ableton Link** (wireless tempo sync). Both are powered
by the ESP32-C6 WiFi co-processor.

WiFi is not required for making sound --- the TBD-16 works fully standalone
with its hardware interface (buttons, encoders, OLED display).


WiFi Setup
==========

Out of the box, the TBD creates its own WiFi network:

- **SSID:** ``ctag-tbd`` (no password)
- Open a browser → ``http://ctag-tbd.local``
- This opens the web interface for presets, plugin settings, and system
  configuration

From the **Edit configuration** page you can adjust WiFi settings:

-  **WiFi mode:**

   -  **Access point (AP)** --- TBD creates its own network that you can
      join directly. No additional router is required. You can secure the
      network with a password.

   -  **Station (STA)** --- TBD joins an existing WiFi network.

-  **SSID** --- The network name TBD will create or join.

-  **Password** --- The password for the network.

-  **mDNS Name** --- The domain name used to access the TBD (default: ``ctag-tbd``).

The TBD-16 also supports a **USB network connection** via USB-C #1 (ESP32-P4
High Speed USB). When connected to a computer, the web interface is accessible
over USB without WiFi. WiFi is still needed for Ableton Link and multi-device
setups.

**Example: Live setup with multiple TBDs**

1. Set one TBD as **AP** (access point).
2. Set other TBDs as **STA** and join the first TBD's network.
3. Access all TBDs through the primary TBD's network --- no external router needed.


Ableton Link
=============

The TBD-16 includes built-in support for `Ableton Link <https://www.ableton.com/en/link/>`_,
a technology that keeps devices in time over a local network. When enabled, your
TBD-16 automatically synchronizes tempo, beat, and transport with any other
Link-enabled app or device on the same WiFi network.

Jam with Ableton Live, hardware synths, iOS apps, and other Link-compatible
tools --- all locked to the same tempo and phase without MIDI clock cables or
manual configuration.


How It Works
------------

1. Connect the TBD-16 to your WiFi network (see WiFi Setup above).
2. Ableton Link automatically discovers other Link sessions on the same network.
3. Tempo, beat position, and start/stop state are shared in real time.

There is **no leader or follower** --- Link is a peer-to-peer protocol. Any
participant can change the tempo and all others follow instantly.


What Gets Synchronized
----------------------

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Field
     - Description
   * - **Tempo**
     - Shared BPM across all Link peers
   * - **Beat**
     - Absolute beat position in the session
   * - **Phase**
     - Beat phase (0.0 -- 1.0), useful for syncing rhythmic patterns
   * - **Playing**
     - Start/Stop transport state (shared across peers)
   * - **Peers**
     - Number of other Link-enabled devices on the network


Using Link with Plugins
-----------------------

Plugin developers can access the Link session data through the ``ProcessData``
struct. When Link is active, plugins receive synchronized tempo and beat
information that can drive rhythmic effects, sequencers, arpeggiators, or any
tempo-aware processing.

See :doc:`Plugin Architecture </plugins/architecture>` for details on the
``ProcessData`` struct.


Compatible Apps and Devices
---------------------------

Ableton Link is supported by `hundreds of apps and devices <https://www.ableton.com/en/link/products/>`_,
including:

- **Ableton Live** (macOS, Windows)
- **Ableton Note** (iOS)
- **Reason** (macOS, Windows)
- **Serato DJ** (macOS, Windows)
- **Traktor** (macOS, Windows)
- **iOS apps** --- Korg Gadget, iMaschine, Patterning, and many more
- **Hardware** --- Teenage Engineering OP-Z, Akai Force, and others


Technical Details
-----------------

- Link is **enabled by default** in the firmware. It can be disabled at build
  time via ``idf.py menuconfig`` under
  *CTAG TBD Configuration > Enable Ableton Link Support*.
- The Link integration runs on the ESP32-P4 and communicates session data to the
  RP2350 front-end via SPI, so both DSP and UI processors have access to
  synchronized tempo and transport.
- Implementation uses a
  `fork of the Ableton Link SDK <https://github.com/ctag-fh-kiel/link>`_
  (git submodule under ``components/ableton_link``). Link data is captured in
  the audio loop at interrupt priority for sample-accurate timing.
