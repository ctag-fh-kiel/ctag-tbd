*********
Simulator
*********

The TBD Simulator lets you run the full DSP engine on your computer --- no hardware required.
It uses 99% the same code as the device firmware, so you can develop, test, and debug plugins
on your laptop before flashing them to real hardware.

The simulator opens a local web server that serves the same web UI as the hardware module.
You interact with it through your browser, just like you would with a connected TBD-16.

.. note::

   The simulator makes the TBD platform accessible to **everyone**, even without owning the hardware.
   This is a great way to explore the plugin library, develop your own plugins, or teach DSP concepts.


What the Simulator Does
=======================

- Runs the CTAG TBD audio engine natively on your host machine
- Serves the web UI at ``http://localhost:8080``
- Processes real-time audio through your computer's sound card
- Supports CV, Pot, and Trigger simulation via ``http://localhost:8080/ctrl``
- Accepts ``.wav`` file input for offline audio processing
- Uses custom sample ROM files exported from hardware


Prerequisites
=============

macOS
-----

.. code-block:: bash

   brew install cmake boost

Debian / Ubuntu
---------------

.. code-block:: bash

   sudo apt-get install libboost-filesystem-dev libboost-thread-dev libboost-program-options-dev libasound2-dev

Arch Linux
----------

.. code-block:: bash

   sudo pacman -S boost

Windows (MSYS2, 64-bit)
------------------------

Install `MSYS2 <https://www.msys2.org>`_ and launch the **MinGW 64-bit** shell (not the default MSYS shell):

.. code-block:: bash

   pacman -Syu
   # Restart the shell, then:
   pacman -Su git mingw-w64-x86_64-make mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-libtool mingw-w64-x86_64-jq mingw-w64-x86_64-boost


Building
========

1. Navigate to the simulator directory and create a build folder:

   .. code-block:: bash

      cd simulator
      mkdir -p build && cd build

2. Run CMake and compile:

   .. code-block:: bash

      cmake .. && make

   On Windows (MSYS2), use:

   .. code-block:: bash

      cmake -G "MinGW Makefiles" ..
      mingw32-make


Running
=======

From the build directory:

.. code-block:: bash

   cd simulator/build
   ./tbd-sim

The simulator runs until you press **Enter**. Open ``http://localhost:8080`` in your browser
to access the web UI.

Audio Device Configuration
--------------------------

The simulator requires a full-duplex sound card at 44100 Hz / 32-bit float.

- **List available devices:**

  .. code-block:: bash

     ./tbd-sim --list

- **Use a specific device** (e.g., device 2):

  .. code-block:: bash

     ./tbd-sim --device 2

- **Output only** (if no duplex device is available):

  .. code-block:: bash

     ./tbd-sim --output

Command-Line Options
--------------------

.. code-block:: text

   -h [ --help ]      Show help message
   -s [ --srom ]      Sample ROM file (default: ../../sample_rom/sample-rom.tbd)
   -l [ --list ]      List sound cards
   -d [ --device ]    Sound card device ID (default: 0)
   -o [ --output ]    Output only (no input)
   -w [ --wav ]       Read audio from WAV file (stereo float32, loops indefinitely)


Using WAV Input
===============

Instead of live audio, you can feed a stereo WAV file (32-bit float) into the simulator:

.. code-block:: bash

   ./tbd-sim --wav path/to/input.wav

The file loops indefinitely, making it useful for testing effects with consistent input material.


CV and Trigger Simulation
=========================

While the simulator is running, open ``http://localhost:8080/ctrl`` in your browser.
This page provides virtual knobs and trigger buttons that mirror the hardware's
CV inputs and trigger inputs, allowing you to test parameter modulation without physical controls.


Developing Plugins with the Simulator
======================================

The simulator uses the same plugin code and directory structure as the hardware firmware.
This means you can:

1. Create your plugin in ``components/ctagSoundProcessor/`` as described in the
   :doc:`Plugin Tutorial </dsp/30_step_by_step>`.
2. Build and run the simulator to test immediately --- no flashing required.
3. Once stable, build the ESP-IDF firmware and flash to hardware.

Registering New Plugins
-----------------------

If a new plugin does not appear in the web UI, the cached processor list needs to be refreshed:

1. Open ``sdcard_image/data/spm-config.jsn``.
2. Delete the entire ``"availableProcessors": [ ... ],`` block.
3. Save the file and restart the simulator.

The system will scan the directory and register the new plugin automatically.


Cloud Builds
============

GitHub Actions can build simulator binaries for Windows and macOS automatically.
Ensure GitHub Actions are enabled for your fork, then customize the workflows
in ``.github/workflows/`` to suit your needs.
