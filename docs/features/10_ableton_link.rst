*************
Ableton Link
*************

The TBD-16 includes built-in support for `Ableton Link <https://www.ableton.com/en/link/>`_,
a technology that keeps devices in time over a local network. When enabled, your
TBD-16 automatically synchronizes tempo, beat, and transport with any other
Link-enabled app or device on the same WiFi network.

This means you can jam with Ableton Live, hardware synths, iOS apps, and other
Link-compatible tools --- all locked to the same tempo and phase without any
MIDI clock cables or manual configuration.


How It Works
============

1. The TBD-16 connects to your WiFi network (see :doc:`WiFi & Firmware Updates </get_started/15_wifi_and_updates>`).
2. Ableton Link automatically discovers other Link sessions on the same network.
3. Tempo, beat position, and start/stop state are shared in real time.

There is **no leader or follower** --- Link is a peer-to-peer protocol. Any participant
can change the tempo and all others follow instantly.


What Gets Synchronized
======================

The Link session provides the following data to the TBD-16:

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
=======================

Plugin developers can access the Link session data through the ``ProcessData`` struct.
When Link is active, plugins receive synchronized tempo and beat information that
can drive rhythmic effects, sequencers, arpeggiators, or any tempo-aware processing.

See the :doc:`Plugin Architecture </dsp/10_plugin_architecture>` documentation for details
on the ``ProcessData`` struct.


Compatible Apps and Devices
===========================

Ableton Link is supported by `hundreds of apps and devices <https://www.ableton.com/en/link/products/>`_,
including:

- **Ableton Live** (macOS, Windows)
- **Ableton Note** (iOS)
- **Reason** (macOS, Windows)
- **Serato DJ** (macOS, Windows)
- **Traktor** (macOS, Windows)
- **iOS apps** --- Korg Gadget, iMaschine, Patterning, and many more
- **Hardware** --- Teenage Engineering OP-Z, Akai Force, and others

Any device or app on the same WiFi network that supports Link will automatically
discover and sync with TBD-16.


Requirements
============

- **WiFi connection** --- Link uses the network to communicate. The TBD-16 must be
  connected to your WiFi (see :doc:`WiFi & Firmware Updates </get_started/15_wifi_and_updates>`).
- All devices must be on the **same local network** (same WiFi / same subnet).
- Link is **enabled by default** in the firmware. It can be disabled at build time
  via ``idf.py menuconfig`` under *CTAG TBD Configuration > Enable Ableton Link Support*.


Technical Details
=================

The Link integration runs on the ESP32-P4 processor and communicates session data
to the RP2350 front-end processor via the SPI bus. This means both the DSP engine
and the UI processor have access to synchronized tempo and transport state.

The implementation uses the official
`Ableton Link SDK <https://github.com/Ableton/link>`_ (included as a git submodule).
Link data is captured in the audio processing loop at interrupt priority for
sample-accurate timing, and a separate thread-safe API is available for
non-real-time queries.
