****************************
Initial Device Setup Guide
****************************

Setting up your TBD device for the first time requires updating the firmware on both chips and initializing the SD cards with the software packages they need. This guide walks you through the entire process step-by-step.

What You'll Need
================

* Your TBD device
* 2× USB-C cables (one for ESP32-P4, one for power)
* A computer with Chrome, Edge, or Opera browser
* An SD card reader/writer
* 2× SD cards (one for ESP32-P4, one for RP2350) or pre-installed cards if included

The device has two chips:

* **ESP32-P4** — main processor, uses an SD card for software
* **RP2350** — secondary processor (co-processor)

Each requires a separate firmware update.

Step 1: Remove the SD Cards
============================

The device comes with two SD card slots. Before flashing the firmware, you must remove both SD cards:

1. **Locate the SD card slots** on the front panel of your device:
   
   * **Middle slot** (closer to the center of the PCB) — ESP32-P4 SD card
   * **Edge slot** (closer to the board edge) — RP2350 SD card

2. **Gently push** each card inward until it clicks and pops out
3. **Set them aside** in a safe place — label them so you don't mix them up

You'll reinsert them after updating the firmware.

Step 2: Flash the ESP32-P4 Firmware (First)
============================================

The ESP32-P4 is the main processor and should be flashed **before** the RP2350.

**Physical setup:**

1. Connect the **JTAG USB-C port on the front** of the device to your computer (data connection)
2. Connect one of the **USB-C ports on the back** to your computer or power supply (power)

**Flashing procedure:**

1. Open the `ESP32-P4 Device Flasher <./20_flash.html>`_ page in your browser
2. The page will show a firmware selector — keep the default selection or choose based on your preference
3. Click the **Connect** button
4. Your computer will show a device selection dialog — select your TBD device
5. Click **Flash**
6. Wait for the progress bar to reach 100% (typically 30–40 seconds)
7. Once complete, you'll see a success message — click **Disconnect**

**Note:** Do not remove any cables during flashing.

Step 3: Flash the RP2350 Firmware (Second)
===========================================

Now update the secondary processor.

**Physical setup:**

The RP2350 uses WebUSB (different protocol). Keep your device connected via the same USB cables.

**Entering BOOTSEL mode:**

1. **Locate the BOOTSEL button** on the front panel (next to the JTAG port)
2. **Hold the BOOTSEL button** while pressing the **RESET button** (to the right of BOOTSEL)
3. Release both buttons — the device is now in BOOTSEL mode

**Flashing procedure:**

1. Open the `RP2350 Device Flasher <./25_flash_pico.html>`_ page in your browser
2. Select your firmware (e.g., ``Possan TBD``)
3. Click **Connect**
4. Your browser will request permission to connect — allow it
5. Click **Flash**
6. Wait for the progress bar to complete (typically 10–15 seconds)
7. Once complete, click **Reboot**

The device will restart out of BOOTSEL mode.

Step 4: Prepare the SD Cards
=============================

While the device boots with the new firmware, format your SD cards for use.

**Format for ESP32-P4 (Main SD Card):**

1. Insert the first SD card into an SD card reader on your computer
2. **Erase the card completely** (backup any existing data first)
3. **Format as FAT32** with the volume name ``NO NAME`` (exactly)
   
   * **macOS:** Disk Utility → select the card → Erase → Format: MS-DOS (FAT) → Volume Name: NO NAME
   * **Windows:** File Explorer → right-click card → Format → File System: FAT32 → Volume Label: NO NAME
   * **Linux:** Any partition manager, or terminal: ``mkfs.vfat -n "NO NAME" /dev/sdX``

**Download SD card software package:**

1. Download the SD card init package: `tbd-sd-card.zip <../_static/sdcard_image/tbd-sd-card.zip>`_
2. Extract the ZIP file on your computer
3. Copy the extracted files directly into the root of the **formatted ESP32-P4 SD card**
   
   * You should see files like ``config.json``, ``data/``, etc. in the card root
   * Do not create a subfolder — files go directly in the root

**Format for RP2350 (Secondary SD Card):**

1. Insert the second SD card into the reader
2. Erase and format as FAT32 with volume name ``NO NAME``
3. **Leave it empty** — the RP2350 does not require software files

Step 5: Insert SD Cards and First Boot
=======================================

Now that both SD cards are ready, insert them back into your device.

**Inserting the cards:**

1. **Insert the ESP32-P4 SD card** into the **middle slot** (closer to the center of the PCB)
2. **Insert the RP2350 SD card** into the **edge slot** (closer to the board/PCB edge)
3. Gently push each card until it clicks into place

**First boot process:**

The first boot after inserting the SD cards will take **several minutes**. This is normal — the ESP32-P4 is extracting and initializing the software package.

**What to expect:**

* The device will power on
* The **OLED screen will display the DADA logo** (colorful animation)
* You'll see various initialization messages scroll on the screen
* **Do not turn off the device** — this process must complete

Once initialization is complete:

* The OLED screen will clear and show debug information
* **You should see ``SD OK`` on one of the lines** — this confirms the SD card is properly initialized
* The device is now ready to use

Step 6: Verify the Setup
========================

To verify everything is working:

1. Power on the device if it's off
2. Check that the OLED shows debug info with **``SD OK``** visible
3. Try navigating the menu on the device — controls should respond normally
4. If the RP2350 firmware is working, you should see activity from the secondary processor

**Troubleshooting:**

* **OLED shows error or no "SD OK":** The SD card may not have the files, be corrupted, or not formatted correctly. Reformat and re-extract the files.
* **Device won't boot:** Try removing and reinserting both SD cards, then power cycle.
* **Flashing failed:** Ensure you disconnected properly between steps and re-enter the correct mode (normal mode for ESP32-P4, BOOTSEL for RP2350).

Next Steps
==========

Congratulations! Your device is now set up and ready to use. Visit the `System <./System.html>`_ documentation for operating instructions and the `Sound Library <../sound_library/index.html>`_ to explore the available processors and sounds.

For advanced users, see `The TBD Command Tool <./The%20TBD%20Command%20Tool.html>`_ for command-line access.
