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

Simulator
=========

The TBD Simulator runs the full DSP engine on your host machine --- no hardware required.
See the dedicated :doc:`Simulator Guide <05_simulator>` for build instructions, usage, and
multi-platform setup.
