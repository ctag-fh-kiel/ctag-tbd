****************************
Initial Device Setup Guide
****************************

Your TBD device needs two firmware updates and SD card initialization before first use. This guide walks you through the complete process.

**Time required:** ~15–20 minutes  
**What you need:** TBD device, 2× USB-C cables, Chrome/Edge/Opera browser, 2× SD cards, SD card reader

---

Step 1: Remove the SD Cards
============================

The SD card slots are on the **back of the device** (inside, on the PCB). You need to remove both before flashing firmware.

**Locate your SD cards:**

.. raw:: html

    <figure style="text-align: center; margin: 1.5em 0;">
      <img src="../_static/assets/dada-tbd-pcb-backside_001.jpg" 
           alt="TBD PCB back showing SD card locations" 
           style="max-width: 500px; border: 1px solid #ccc; border-radius: 6px;">
      <figcaption style="font-size: 0.9em; color: #666; margin-top: 0.5em;">
        <strong>Red circle:</strong> ESP32-P4 SD card (middle of PCB) | 
        <strong>Other slot:</strong> RP2350 SD card (near edge)
      </figcaption>
    </figure>

**Remove the cards:**

1. Gently push each card inward until it clicks and pops out
2. Set them aside in a safe place and label them to avoid mixing them up



Step 2: Flash the ESP32-P4 Firmware (First)
============================================

The ESP32-P4 (main processor) must be flashed **before** the RP2350.

**Connect your device:**

- Plug the **JTAG USB-C port** (front) into your computer
- Plug a **back USB-C port** into power or another computer port

**Flash the firmware:**

1. Open the `ESP32-P4 Device Flasher <./20_flash.html>`_
2. Select firmware: **CTAG TBD (Development) — 2026-02-11** or another variant
3. Click **Connect** and select your device when prompted
4. Click **Flash** and wait for 100% (about 30–40 seconds)
5. Click **Disconnect** when done

*Firmware file:* `ctag-tbd-2026-02-11.bin <../_static/firmware/p4/ctag-tbd-2026-02-11.bin>`_



Step 3: Flash the RP2350 Firmware (Second)
===========================================

Now update the secondary processor. Keep your device connected via the same USB cables.

**Enter BOOTSEL mode:**

1. Hold the **BOOTSEL button** (front, next to JTAG port)
2. While holding, press the **RESET button** (to the right of BOOTSEL)
3. Release both buttons

**Flash the firmware:**

1. Open the `RP2350 Device Flasher <./25_flash_pico.html>`_
2. Select firmware: **CTAG TBD (Development) — 2026-02-11** (to match P4)
3. Click **Connect** and allow your browser to access the device
4. Click **Flash** and wait for 100% (about 10–15 seconds)
5. Click **Reboot**

*Firmware file:* `ctag-tbd-2026-02-11.uf2 <../_static/firmware/pico/ctag-tbd-2026-02-11.uf2>`_



Step 4: Prepare the SD Cards
=============================

While the device boots, format both SD cards for use.

**Format the ESP32-P4 SD card (middle slot):**

*Insert into your SD card reader:*

- **Erase completely** (backup any existing data first)
- **Format as FAT32** with volume label **NO NAME** (exactly)

  - macOS: Disk Utility → Erase → Format: MS-DOS (FAT) → Name: NO NAME
  - Windows: File Explorer → right-click → Format → FAT32 → Label: NO NAME
  - Linux: ``mkfs.vfat -n "NO NAME" /dev/sdX``

*Copy the SD card software package:*

1. Download: `tbd-sd-card.zip <../_static/sdcard_image/tbd-sd-card.zip>`_
2. Extract the ZIP file
3. Copy **all extracted files** directly to the **root of the card** (not in a subfolder)
4. Eject the card

**Format the RP2350 SD card (edge slot):**

- Erase and format as FAT32 with label **NO NAME**
- **Leave it empty** (no files needed)
- Eject the card



Step 5: Insert SD Cards and Initialize
=======================================

Insert both SD cards back into the device on the PCB back:

1. **Middle slot:** Insert the **ESP32-P4 SD card**
2. **Edge slot:** Insert the **RP2350 SD card**
3. Gently push each until it clicks

**First boot (SD card initialization):**

Power on the device. The initialization will take **5–10 minutes**.

*What to expect:*

- Device powers on
- **OLED shows initialization messages and debug text**
- **Do not turn off the device** — this process must complete

*When initialization is done:*

- OLED shows debug information
- **One line displays ``SD OK``** — confirms SD card is initialized ✓
- Device is ready for firmware update



Step 6: Update to Latest Firmware (Possan)
===========================================

Now that the device is initialized, update both chips to the latest production firmware.

**Update RP2350 first:**

1. Enter **BOOTSEL mode** (hold BOOTSEL + press RESET)
2. Open the `RP2350 Device Flasher <./25_flash_pico.html>`_
3. Select firmware: **Possan TBD (Experimental) — 2026-02-14**
4. Click **Connect** → **Flash** → **Reboot**

*Firmware file:* `possan-tbd-2026-02-14.uf2 <../_static/firmware/pico/possan-tbd-2026-02-14.uf2>`_

**Update ESP32-P4 second:**

1. Open the `ESP32-P4 Device Flasher <./20_flash.html>`_
2. Select firmware: **Possan TBD (Experimental) — 2026-02-14**
3. Click **Connect** → **Flash** → **Disconnect**

*Firmware file:* `possan-tbd-2026-02-14.bin <../_static/firmware/p4/possan-tbd-2026-02-14.bin>`_

Step 7: Verify the Setup
=========================

Power on the device and check:

✓ OLED shows debug information with ``SD OK`` visible  
✓ Device responds to controls  
✓ Both firmware versions are running correctly

**Troubleshooting:**

- **OLED shows no "SD OK":** Reformat the ESP32-P4 card and re-extract the files
- **Device won't boot:** Remove and reinsert both SD cards, then power cycle
- **Flashing failed:** Disconnect between steps and re-enter the correct mode

---

Setup Complete!
===============

Your device is ready to use. See the `System <./10_system.html>`_ documentation for operating instructions and the `Sound Library <../sound_library/index.html>`_ to explore available processors and sounds.
