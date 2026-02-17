****************************
Frontend --- Getting Started
****************************

This guide walks you through setting up your development environment for RP2350
frontend firmware --- custom MIDI controllers, sequencers, control surfaces,
and UI screens for the TBD-16.

.. contents:: Steps
   :local:
   :depth: 2


Why Start Here?
===============

The RP2350 is the TBD-16's **frontend processor**. It handles everything the user
touches and sees: the OLED display, buttons, encoders, LEDs, and MIDI
connectivity. The DSP engine on the ESP32-P4 runs independently --- your
frontend firmware talks to it over SPI.

Building a custom frontend is a great first project because:

-  **Arduino-based** --- Use familiar Arduino APIs, libraries, and community resources.
-  **Fast iteration** --- Drag-and-drop a UF2 file to flash. No special tools required.
-  **No audio knowledge needed** --- You control the DSP engine through a clean
   API. No need to write signal processing code.
-  **Immediate results** --- Change what the display shows, how buttons behave,
   or build an entire sequencer.

**Project ideas to get started:**

-  A MIDI controller that maps your knobs and buttons to specific DSP parameters
-  A drum sequencer (Euclidean, Turing Machine, stochastic)
-  A control surface optimized for a single DSP plugin
-  A custom display layout showing only what you need on stage


1. Install the Toolchain
========================

The frontend firmware is built with `PlatformIO <https://platformio.org/>`_,
an open-source build system for embedded development. It handles the Arduino
framework, board packages, and all library dependencies automatically.

1a. Install VS Code
-------------------

Download and install `Visual Studio Code <https://code.visualstudio.com/>`_.

1b. Install PlatformIO Extension
---------------------------------

1. Open VS Code.
2. Go to **Extensions** (``Cmd+Shift+X`` on macOS, ``Ctrl+Shift+X`` on Windows/Linux).
3. Search for **PlatformIO IDE**.
4. Click **Install**.

PlatformIO will download its core tools automatically on first launch. This may
take a minute.

.. note::
   PlatformIO includes everything you need --- the ARM toolchain, board support
   packages, and upload tools. No separate SDK installation required.


2. Get the Source
=================

Clone the RP2350 firmware template repository:

.. code-block:: bash

    git clone https://github.com/dadamachines/rp2350-arduino-tbd-fw.git
    cd rp2350-arduino-tbd-fw

.. note::
   This is a separate repository from the main DSP project. The frontend and
   backend are developed independently --- you do not need the ESP-IDF or the
   DSP repository to build frontend firmware.


3. Open in VS Code
===================

1. In VS Code, click **File > Open Folder** and select the ``rp2350-arduino-tbd-fw`` folder.
2. PlatformIO will detect the ``platformio.ini`` file and configure the project.
3. Wait for PlatformIO to finish downloading platform packages and libraries
   (visible in the bottom status bar).


4. Build the Firmware
=====================

Click the **checkmark icon** (✓) in the PlatformIO toolbar at the bottom of
VS Code, or run:

.. code-block:: bash

    pio run

PlatformIO will:

-  Download the RP2350 Arduino core (earlephilhower)
-  Fetch all library dependencies (Adafruit GFX, NeoPixel, TinyUSB, ArduinoJson, etc.)
-  Compile the firmware

The first build takes longer as dependencies are downloaded. Subsequent builds
are fast.


5. Flash to Your TBD-16
========================

5a. Enter Bootloader Mode
--------------------------

1. Hold the **BOOTSEL** button on the TBD-16's RP2350.
2. While holding BOOTSEL, press and release **RESET**.
3. Release BOOTSEL. The RP2350 will appear as a USB drive on your computer.

5b. Upload
----------

**Option A --- PlatformIO (recommended):**

Click the **right-arrow icon** (→) in the PlatformIO toolbar, or run:

.. code-block:: bash

    pio run --target upload

**Option B --- Manual drag-and-drop:**

Copy the compiled ``.uf2`` file from the build output to the USB drive:

.. code-block:: bash

    cp .pio/build/pi2350/firmware.uf2 /Volumes/RP2350

The device will reboot automatically after flashing.


6. Project Structure
====================

.. code-block:: text

    rp2350-arduino-tbd-fw/
    ├── platformio.ini        # Build configuration and dependencies
    ├── src/
    │   ├── src.ino           # Arduino entry point (stub)
    │   ├── Midi.cpp / .h     # MIDI handling (USB host, UART, device)
    │   ├── Ui.cpp / .h       # UI logic (display, LEDs, buttons, encoders)
    │   ├── SpiAPI.cpp / .h   # SPI API to control the DSP engine
    │   ├── MidiParser.cpp/.h # MIDI message parsing
    │   ├── DadaLogo.h        # Boot logo bitmap
    │   └── Fonts/            # Custom display fonts
    └── examples/
        └── main.cpp          # Main firmware (dual-core setup)

The entry point is ``examples/main.cpp``:

.. code-block:: cpp

    #include "Midi.h"
    #include "Ui.h"

    Midi midi;
    Ui tbd_ui(midi);

    void setup()  { midi.Init(); }
    void setup1() { tbd_ui.Init(); }

    void loop()   { midi.Update(); }
    void loop1()  { tbd_ui.Update(); }

The RP2350 runs **two cores** simultaneously:

-  **Core 0** --- MIDI processing (USB host, UART, device).
-  **Core 1** --- UI updates (display, LEDs, SPI communication with P4).


7. Controlling the DSP Engine
=============================

The ``SpiAPI`` class lets your frontend firmware control the entire DSP engine.
You can load plugins, change parameters, and manage presets --- all from Arduino
code.

**Example --- Load a plugin and set a parameter:**

.. code-block:: cpp

    #include "SpiAPI.h"

    SpiAPI api;

    void setup() {
        api.Init();
        api.WaitSpiAPIReadyForCmd();

        // Load the "DrumRack" plugin on channel 0
        api.SetActivePlugin(0, "DrumRack");

        // Set a parameter value
        api.SetActivePluginParam(0, "volume", 200);
    }

**Key SPI API methods:**

.. list-table::
   :widths: 40 60
   :header-rows: 1

   * - Method
     - Description
   * - ``GetPlugins()``
     - List all available DSP plugins
   * - ``SetActivePlugin(ch, id)``
     - Load a plugin on a channel
   * - ``SetActivePluginParam(ch, name, val)``
     - Set a parameter value
   * - ``GetActivePluginParams(ch)``
     - Get all parameters of the active plugin
   * - ``LoadPreset(ch, id)``
     - Load a preset
   * - ``SavePreset(ch, name, id)``
     - Save a preset
   * - ``Reboot()``
     - Reboot the DSP processor

See the :doc:`Architecture & SPI API <20_rp2350>` page for full details on
the communication protocol.


8. Debugging
============

**Serial Monitor:**

Use PlatformIO's serial monitor (plug icon in toolbar) or:

.. code-block:: bash

    pio device monitor --baud 115200

**SWD Debugging (optional):**

If you have a CMSIS-DAP debug probe (e.g., Raspberry Pi Debug Probe), uncomment
these lines in ``platformio.ini``:

.. code-block:: ini

    debug_tool = cmsis-dap
    upload_protocol = cmsis-dap

Then press ``F5`` in VS Code to start a debug session with breakpoints and
variable inspection.


Next Steps
==========

-  :doc:`Architecture & SPI API <20_rp2350>` --- Deep dive into the RP2350/P4
   communication protocol, MIDI integration, and pin mapping.
-  :doc:`Flash UI Firmware </get_started/30_flash_ui>` --- Flash pre-built firmware
   without a development environment.
-  Ready for audio DSP? See :doc:`DSP --- Getting Started </dsp/10_getting_started>`.
