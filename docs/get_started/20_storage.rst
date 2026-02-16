*****************
Storage & Samples
*****************

The TBD-16 uses SD cards for storing configuration, the web interface, and audio samples.

SD Card Structure
=================

When you connect the TBD-16 to your computer via USB (in USB-MSC mode), you will see
the following directory structure on the ESP32-P4 SD card:

*   ``/data/`` --- System configuration files.
    *   ``spm-config.jsn``: Main configuration for the Sound Processor Manager. It tracks
        which plugins are available and stores the last state of each channel (active plugins,
        patches, etc.).
    *   ``/sp/``: Folder containing plugin-specific configuration and UI definitions (JSON files).
*   ``/rom/`` --- Audio samples used by the internal samplers and granular engines.
*   ``/www/`` --- The files for the web-based user interface.
*   ``/c6_fw/`` --- (Optional) Folder used for updating the WiFi co-processor firmware.

Working with Samples
====================

The ``/rom/`` directory is organized into banks and categories to help you navigate your
sample library.

Organization
------------

By default, the TBD-16 sample ROM is structured as follows:

*   **Drums**: Kicks, snares, hats, and loops.
*   **Wavetables**: Formats compatible with the wavetable oscillators.
*   **Other**: General purpose samples and field recordings.

Adding Your Own Samples
-----------------------

To add samples:

1.  Boot into USB-MSC mode (see :doc:`Building & Setup </dsp/30_building>`).
2.  Copy your ``.wav`` files into the appropriate subfolders in ``/rom/``.
3.  For best performance, use **48kHz / 32-bit float** or **24-bit** WAV files.

Managing Configuration
======================

The ``spm-config.jsn`` file is updated automatically by the system. If you want to force
a rescan of available plugins (for example, after adding a new plugin definition to
``/data/sp/``), you can delete the ``"availableProcessors"`` block in ``spm-config.jsn``.
The system will recreate it on the next boot.
