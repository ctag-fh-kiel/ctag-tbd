*************************
Debugging with SWD
*************************

The TBD-16 exposes the RP2350's **SWD** (Serial Wire Debug) and **UART** pins on
the right side of the enclosure, so you can flash, set breakpoints, and read
serial output without opening the case. This page explains how to set
everything up with the Raspberry Pi Debug Probe and PlatformIO in VS Code.


Why Use a Debug Probe?
======================

The default upload method works fine --- hold BOOTSEL, drag a UF2 file. But a
CMSIS-DAP debug probe unlocks a much faster workflow:

-  **One-click flash** --- Upload over SWD without touching BOOTSEL or
   unplugging cables.
-  **Breakpoints & stepping** --- Pause execution, inspect variables, and step
   through code line by line --- right inside VS Code.
-  **PIO serial output** --- Read ``Serial.println()`` debug messages over
   UART without using any of the RP2350's hardware UARTs (which are reserved
   for MIDI).
-  **No reboot cycle** --- Flash and debug without resetting the board.


What You Need
=============

.. list-table::
   :widths: 30 70
   :header-rows: 0

   * - **Raspberry Pi Debug Probe**
     - The `official debug probe <https://www.raspberrypi.com/products/debug-probe/>`_
       from Raspberry Pi. It connects to your computer via USB and provides both
       a CMSIS-DAP SWD interface and a UART bridge in a single device.
   * - **Jumper wires**
     - 4 female-to-female jumper wires to connect the debug probe to the
       TBD-16's debug header.
   * - **USB cable**
     - A USB Micro-B cable to connect the debug probe to your computer (ships
       with the probe).

.. note::
   You can also use a regular **Raspberry Pi Pico** or **Pico 2** board
   flashed with the
   `debugprobe firmware <https://github.com/raspberrypi/debugprobe/releases>`_
   as a cheaper alternative. The dedicated Debug Probe is recommended because
   it provides both SWD and UART in a single USB connection with proper
   connectors.


Wiring
======

The TBD-16 has a **debug header** on the right side of the case with the
following pins:

.. list-table::
   :widths: 20 20 60
   :header-rows: 1

   * - Pin
     - Signal
     - Description
   * - GND
     - Ground
     - Connect to GND on the debug probe
   * - SWCLK
     - SWD Clock
     - Connect to **SC** on the debug probe
   * - SWDAT
     - SWD Data
     - Connect to **SD** on the debug probe
   * - UART TX
     - Serial output
     - Connect to **RX** on the debug probe's UART port

Connect the four wires between the debug header and the debug probe:

.. list-table::
   :widths: 30 10 30
   :header-rows: 1

   * - TBD-16 Debug Header
     - →
     - Debug Probe
   * - GND
     - →
     - GND (SWD cable)
   * - SWCLK
     - →
     - SC (SWD cable)
   * - SWDAT
     - →
     - SD (SWD cable)
   * - UART TX
     - →
     - RX (UART cable)


PlatformIO Configuration
=========================

Open ``platformio.ini`` in your project and **uncomment** the two CMSIS-DAP
lines:

.. code-block:: ini

    [env:pi2350]
    platform = https://github.com/maxgerhardt/platform-raspberrypi.git
    board = generic_rp2350
    framework = arduino
    ; ... other settings ...

    debug_tool = cmsis-dap
    upload_protocol = cmsis-dap

With these lines active, PlatformIO will:

-  **Upload** firmware over SWD instead of UF2 --- no BOOTSEL button needed.
-  **Debug** using the CMSIS-DAP interface --- breakpoints, stepping, and
   variable inspection work out of the box.


Flashing over SWD
==================

Once wired and configured, flashing is the same as before:

Click the **right-arrow icon** (→) in the PlatformIO toolbar, or run:

.. code-block:: bash

    pio run --target upload

PlatformIO will detect the debug probe and flash the firmware over SWD.
The device does **not** need to be in BOOTSEL mode.


Debugging in VS Code
=====================

1. Set a **breakpoint** by clicking in the gutter next to a line of code.
2. Click **Debug** in the PlatformIO sidebar, or press ``F5``.
3. The firmware is compiled, flashed, and halted at the entry point.
4. Step through code with ``F10`` (step over) or ``F11`` (step into).
5. Inspect variables in the **Variables** panel, or hover over them in the
   editor.

.. tip::
   If the debugger does not connect, try deleting the ``.pio`` folder and
   rebuilding the project (``pio run --target clean && pio run``).
   PlatformIO occasionally caches stale debug configurations.


Serial Output via PIO UART
============================

The TBD-16 template firmware includes a **PIO-based serial transmitter** that
uses the SDA pin on the debug header (GPIO 20). This is a software UART
implemented via the RP2350's PIO state machine, so it does **not** consume any
of the hardware UART peripherals (which are used for MIDI).

To enable serial debug output, uncomment these lines in the main sketch:

.. code-block:: cpp

    // In global scope
    SerialPIO transmitter(20, SerialPIO::NOPIN);

    // In setup()
    transmitter.begin(115200);
    transmitter.println("CTAG TBD started");

    // In loop()
    transmitter.println("CTAG TBD Loop");

Then use PlatformIO's **Upload and Monitor** button or run:

.. code-block:: bash

    pio device monitor --baud 115200

The serial output appears on the debug probe's UART port, visible as a second
serial device on your computer.


Troubleshooting
===============

**Debug probe not detected**
   - Make sure the USB cable to the debug probe is connected and the probe's
     LED is on.
   - Verify that ``debug_tool = cmsis-dap`` and ``upload_protocol = cmsis-dap``
     are uncommented in ``platformio.ini``.
   - On macOS, no driver installation is needed. On Windows, the probe should
     appear automatically; if not, check Device Manager for a CMSIS-DAP device.

**Upload succeeds but debug session does not start**
   - Delete the ``.pio`` folder and rebuild: ``pio run --target clean``.
   - Make sure no other application is using the debug probe (only one debug
     session at a time).

**No serial output**
   - Confirm the UART TX wire is connected to the debug probe's RX pin.
   - Ensure the ``SerialPIO`` lines are uncommented in your code.
   - Check that the baud rate matches (115200).

**Intermittent connection issues**
   - Use short jumper wires (< 20 cm). Long wires can cause SWD signal
     integrity issues.
   - Make sure GND is connected --- SWD will not work without a common ground.
