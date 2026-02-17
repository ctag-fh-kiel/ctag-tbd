*****************
Storage & Samples
*****************

The TBD-16 uses two SD cards for storing system data and audio samples.
It ships with pre-loaded factory content --- drums, wavetables, and loops
--- so you can start making music right away.


SD Cards
========

The TBD-16 has two micro-SD card slots, one for each processor:

.. list-table::
   :header-rows: 1
   :widths: 20 30 50

   * - Slot
     - Processor
     - Contents
   * - Middle slot
     - ESP32-P4
     - System config, web interface, audio samples (``/tbdsamples/``),
       backup (``/dbup/``), version file
   * - Edge slot
     - RP2350
     - Frontend firmware (``.uf2`` apps), RP2350 config


Factory Samples
===============

The P4 SD card ships with a ``/tbdsamples/`` folder organized by category:

- **drums/** --- Kicks, snares, hi-hats, claps, percussion, loops
- **wavetables/** --- Wavetable banks for the wavetable oscillator plugins
- **other/** --- Miscellaneous samples and textures

Samples are stored as **44.1 kHz, 16-bit mono WAV** files. At boot, the
system loads the active sample bank and wavetable bank into PSRAM for
real-time playback by DSP plugins.


Sample Banks
============

Samples are organized into **banks** --- named collections of WAV files.
The TBD-16 ships with a default sample bank (factory drums) and a default
wavetable bank.

Banks are defined by JSON files inside ``/tbdsamples/``:

- ``sample_rom.jsn`` --- Master index listing all available banks and the
  currently active bank
- ``def_smp.jsn`` --- Default sample bank (drums, percussion)
- ``def_wt.jsn`` --- Default wavetable bank
- Additional ``.jsn`` files for extra banks (e.g. ``a4_dub.jsn``)

You can switch the active bank through the web interface or the hardware UI.


Adding Your Own Samples
=======================

.. note::

   We're building an integrated web-based sample manager that will handle
   all conversion and bank building directly on the device --- no scripts,
   no file conversion on your computer. This is still a work in progress.

In the meantime, here's how to get your own samples onto the TBD-16:

1. **Prepare your WAV files** --- Convert them to **44.1 kHz, mono, 16-bit PCM**.
   You can use any audio editor (Audacity, Ableton, etc.) or the Python converter
   script included in the repository (``sample_rom/wav_info_parser.py``).

2. **Access the SD card** --- Either remove the P4 SD card and insert it in your
   computer, or boot into USB-MSC mode to access it over USB.

3. **Copy files** --- Place your ``.wav`` files into a subfolder of ``/tbdsamples/``
   (e.g. ``/tbdsamples/my_samples/``).

4. **Create a bank file** --- Create a ``.jsn`` file in ``/tbdsamples/`` listing
   your samples (see the existing ``def_smp.jsn`` as a template). Each entry needs:

   - ``filename`` --- Stem name without extension (max 32 chars)
   - ``path`` --- Subfolder path relative to ``/tbdsamples/``
   - ``nsamples`` --- Number of sample frames in the file

5. **Register the bank** --- Add your bank file to the ``smp_banks`` array in
   ``sample_rom.jsn``.

6. **Reboot** --- The TBD-16 will load the new bank data on the next start.

.. tip::

   Keep file names short (32 characters max) and avoid special characters.
   Sample data is loaded into PSRAM at boot, so total bank size is limited
   by available memory.


System Configuration
====================

The P4 SD card also contains a ``/data/`` folder with system configuration
files. These are managed automatically by the firmware:

- ``spm-config.jsn`` --- Sound Processor Manager state (loaded plugins, patches)
- ``/data/sp/`` --- Plugin parameter presets and UI definitions

A backup of the ``/data/`` folder is stored in ``/dbup/`` and can be used
for recovery.

.. warning::

   Editing configuration files manually is not recommended unless you know
   what you're doing. For normal use, the system manages these automatically.


Recovery
========

If your SD card data becomes corrupted, see
:doc:`Device Recovery <50_device_recovery>` for a complete guide to
re-initializing your TBD-16 from scratch, including fresh SD card images.
