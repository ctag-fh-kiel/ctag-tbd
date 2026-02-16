*******************
Building & Setup
*******************

This guide outlines the steps required to build the CTAG TBD firmware from source and set up the development environment.

Prerequisites
=============

1. ESP-IDF Framework
--------------------

The project requires **ESP-IDF v5.5.1** or newer.

**Installation:**

.. code-block:: bash

    # Clone ESP-IDF
    mkdir -p ~/esp
    cd ~/esp
    git clone --recursive https://github.com/espressif/esp-idf.git
    cd esp-idf
    git checkout v5.5.1
    git submodule update --init --recursive

    # Install Tools
    ./install.sh

2. System Dependencies (macOS)
------------------------------

The build system requires ``xxhash`` for checksum calculations.

.. code-block:: bash

    brew install xxhash

Project Setup
=============

1. Clone Repository (Recursive)
-------------------------------

This project uses submodules (e.g., ``ableton_link``, ``esp-dsp``). Ensure you clone recursively or update submodules after cloning.

.. code-block:: bash

    git clone --recursive https://github.com/dadamachines/ctag-tbd.git
    # OR if already cloned:
    git submodule update --init --recursive

2. Configure Environment
------------------------

Before building, export the ESP-IDF environment variables in your terminal session:

.. code-block:: bash

    . ~/esp/esp-idf/export.sh

Building
========

To build the firmware:

.. code-block:: bash

    idf.py build

The build output will be located in ``build/ctag-tbd.bin``.

Flashing
========

To flash the firmware to the device:

.. code-block:: bash

    idf.py flash

To flash and monitor the serial output:

.. code-block:: bash

    idf.py flash monitor

Troubleshooting
===============

- **Missing xxh128sum**: If you see an error related to ``XXH128SUM``, install ``xxhash`` (``brew install xxhash`` on macOS).
- **Submodule Errors**: If you encounter errors about missing files in ``components/ableton_link`` or others, run ``git submodule update --init --recursive``.
- **Partition Size Warning**: You may see a warning: "app partitions are too small for binary". The ``ota_0`` partition (5MB) is sufficient for the initial flash, but the ``ota_1`` partition (1MB) is too small for OTA updates.

Simulator Build
===============

The simulator allows you to run the TBD engine on your host machine for development and testing.

Prerequisites (macOS)
---------------------

Install dependencies via Homebrew:

.. code-block:: bash

    brew install cmake boost

Building the Simulator
----------------------

1. Navigate to the simulator directory and create a build folder:

   .. code-block:: bash

       cd simulator
       mkdir -p build && cd build

2. Run CMake and compile:

   .. code-block:: bash

       cmake .. && make

Running the Simulator
---------------------

1. Navigate to the build directory:

   .. code-block:: bash

       cd simulator/build

2. Run the executable:

   .. code-block:: bash

       ./tbd-sim

   *Note: The simulator will run until you press <Enter>.*

**Audio Configuration**:
The simulator requires a valid audio device.

- List available audio devices:

  .. code-block:: bash

      ./tbd-sim --list

- Run with a specific device (e.g., device 2) if the default fails:

  .. code-block:: bash

      ./tbd-sim --device 2

Developing New Plugins
======================

When creating a new plugin (e.g., adding ``mui-MyPlugin.jsn``), you must force the system to detect it. The ``spm-config.jsn`` file caches the list of available processors to speed up boot times.

**Issue**: If your new plugin does not appear in the Web UI, it is likely because ``spm-config.jsn`` contains an outdated cache.

**Solution**:

1. Open ``sdcard_image/data/spm-config.jsn``.
2. Delete the entire ``"availableProcessors": [ ... ],`` block.
3. Save the file.
4. Run the simulator (or reboot the hardware). The system will detect the missing list, scan the directory, and register your new plugin.

For more detailed information, including advanced usage and dependencies for other platforms, refer to the `Simulator README <https://github.com/ctag-fh-kiel/ctag-tbd/blob/p4_main/simulator/readme.md>`_.
