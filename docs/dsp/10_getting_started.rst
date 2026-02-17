**********************
DSP --- Getting Started
**********************

This guide walks you through setting up your development environment for DSP
plugin development on the ESP32-P4 --- the TBD-16's audio engine.


Before You Begin
================

DSP development means writing audio processing code --- synthesizers, effects,
and drum machines that run on the ESP32-P4. This is more advanced than
:doc:`frontend development </frontend/10_getting_started>`, which uses Arduino
and doesn't require audio programming knowledge.

**If you're new to the TBD platform**, consider starting with frontend
development first. You can build custom MIDI controllers, sequencers, and
control surfaces for existing DSP plugins without touching audio code.

**If you're ready for DSP**, this guide will get you set up.


1. Install the Toolchain
=========================

The DSP firmware is built with `ESP-IDF <https://docs.espressif.com/projects/esp-idf/>`_,
Espressif's official development framework for ESP32 chips.

1a. Install ESP-IDF
--------------------

The project requires **ESP-IDF v5.5.1** or newer.

**macOS / Linux:**

.. code-block:: bash

    # Clone ESP-IDF
    mkdir -p ~/esp
    cd ~/esp
    git clone --recursive https://github.com/espressif/esp-idf.git
    cd esp-idf
    git checkout v5.5.1
    git submodule update --init --recursive

    # Install toolchain and tools
    ./install.sh

**Windows:**

Download and run the
`ESP-IDF Tools Installer <https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/windows-setup.html>`_.
Select ESP-IDF v5.5.1 during installation.

.. note::
   The ESP-IDF installer downloads the Xtensa and RISC-V cross-compilers,
   CMake, Ninja, Python, and all required tools. This can take several minutes.


1b. Install System Dependencies
--------------------------------

**macOS:**

.. code-block:: bash

    brew install xxhash

The build system uses ``xxh128sum`` for firmware integrity checks.


1c. Install VS Code Extension (Optional)
------------------------------------------

Espressif provides a VS Code extension for ESP-IDF development:

1. Open VS Code.
2. Go to **Extensions** (``Cmd+Shift+X``).
3. Search for **ESP-IDF** and install the Espressif extension.
4. Run the setup wizard (``Cmd+Shift+P`` > "ESP-IDF: Configure ESP-IDF Extension").

This gives you integrated build, flash, and monitor commands. You can also work
entirely from the terminal if you prefer.


2. Get the Source
==================

Clone this repository with submodules:

.. code-block:: bash

    git clone --recursive https://github.com/dadamachines/ctag-tbd.git
    cd ctag-tbd

.. important::
   The ``--recursive`` flag is required. The project depends on submodules
   including Ableton Link, esp-dsp, and Mutable Instruments code.

If you already cloned without ``--recursive``:

.. code-block:: bash

    git submodule update --init --recursive


3. Configure Your Shell
========================

Before building, export the ESP-IDF environment in your terminal:

.. code-block:: bash

    . ~/esp/esp-idf/export.sh

.. tip::
   Add this line to your shell profile (``~/.zshrc`` or ``~/.bashrc``) so it
   runs automatically in every terminal session.


4. Build the Firmware
======================

.. code-block:: bash

    idf.py build

This compiles the entire DSP engine, all plugins, the web server, and WiFi
stack. The first build takes several minutes. Subsequent builds are incremental
and much faster.

The output is at ``build/ctag-tbd.bin``.


5. Flash to Your TBD-16
=========================

Connect your TBD-16 via USB and flash:

.. code-block:: bash

    idf.py flash

To flash and open the serial monitor in one step:

.. code-block:: bash

    idf.py flash monitor

Press ``Ctrl+]`` to exit the monitor.


6. Alternative --- Use the Simulator
=====================================

You can develop and test DSP plugins **without any hardware** using the TBD
Simulator. It runs the full DSP engine on your computer with a local web
interface.

See the :doc:`Simulator Guide <50_simulator>` for setup instructions.

This is ideal for:

-  Developing plugins without a TBD-16 on hand
-  Rapid iteration (no flash/reboot cycle)
-  Automated testing of audio algorithms


7. Troubleshooting
====================

**"Missing xxh128sum"**
    Install xxhash: ``brew install xxhash`` (macOS) or
    ``apt install xxhash`` (Linux).

**Submodule errors**
    Run ``git submodule update --init --recursive`` to fetch all dependencies.

**Build fails on first try**
    Make sure you ran ``. ~/esp/esp-idf/export.sh`` in the current terminal
    session. ESP-IDF tools are not in your PATH by default.


Next Steps
==========

-  :doc:`Plugin Architecture <20_plugin_architecture>` --- How the DSP plugin
   system works.
-  :doc:`Step-by-Step Plugin Guide <30_step_by_step>` --- Create your first
   audio plugin from scratch.
-  :doc:`Simulator <50_simulator>` --- Develop plugins on your computer without
   hardware.
-  :doc:`Web UI API <60_web_api>` --- Control plugins remotely over WiFi.
