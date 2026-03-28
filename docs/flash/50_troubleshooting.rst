:orphan:

****************************
Troubleshooting
****************************

Common issues when using the flash pages. This applies to both the
:doc:`Stable Channel <10_stable_channel>` and
:doc:`Beta Channel <20_staging_channel>`.

Browser Compatibility
---------------------

**"Your browser does not support WebSerial"**

  Use **Chrome, Edge, or Opera** on desktop. Safari and Firefox do not support WebSerial.

**"Your browser does not support the File System Access API"**

  Use **Chrome or Edge** on desktop. Firefox and Safari do not support this API.
  The File System Access API is required for Path B (Full SD Card Deploy) Step 2.

Serial Connection
-----------------

**Serial port request fails**

  - Make sure the **front JTAG port** cable is connected to your computer
  - Unplug the USB cable and replug it
  - Try a different USB cable
  - Close any terminals or serial monitors that may hold the port open (only one program can use a serial port at a time)
  - Restart your browser

**Linux: serial port not accessible**

  On Linux, serial ports are typically owned by the ``dialout`` (or ``uucp``) group.
  If the browser cannot open the serial port, add your user to the group and log out/in::

    sudo usermod -aG dialout $USER

SD Card (Path B)
----------------

**SD card drive doesn't appear after Step 1**

  The software reset sent via WebSerial can silently fail — the device may not
  actually reboot even though the flash page says it did.

  Try these steps **in order**:

  1. **Wait at least 25–30 seconds.** The SD card needs time to initialize
     (the device retries mounting up to 5 times with increasing delays).
  2. **Power-cycle**: unplug **both** USB cables, wait 3 seconds, replug
     **back Port #1** only. Wait 25–30 seconds.
  3. If the drive still doesn't appear, click **🔄 Retry software reset**
     on the flash page (appears after the countdown finishes).

  Also check:

  - Make sure a USB-C cable is connected from **back Port #1** to your computer.
    Port #1 is the back USB-C port closest to the center of the device.
  - Check your file manager for a new drive (typically named ``NO NAME``).
  - On **macOS**: open Finder → Go → Computer, or check ``/Volumes/``.
  - On **Windows**: open File Explorer → This PC.
  - On **Linux**: check ``/media/$USER/`` or run ``lsblk``.
  - If the drive still doesn't appear after all three steps above, go back to
    Step 1 in the flash page and repeat the process.

**SD card drive appears but disappears quickly / is unresponsive**

  Some older or low-quality SD cards struggle with the high-speed UHS-II
  initialization. The device will retry mounting, but this can take 30+ seconds.
  Wait at least 30 seconds before concluding the drive is not working.

  If the problem persists, the SD card may need to be replaced.

**"Permission denied" when writing files**

  This can mean two things:

  - **Browser permission**: Chrome asked for permission to write to the selected
    directory. Click **Allow**. If you accidentally denied, close the prompt and
    click "Select SD Card Drive" again.
  - **Drive still initializing**: If the drive just appeared, the filesystem may
    not be fully ready. Wait 5–10 more seconds, then retry.

**Step 3 connection fails (device is in MSC mode)**

  When the device is in USB Mass Storage (MSC) mode, the serial/JTAG port on the
  **front** may not respond. This is normal — the SD card drive must be ejected
  before the serial port becomes available again.

  1. **Eject the SD card drive first:**

     - macOS: right-click the drive → Eject
     - Windows: System tray → "Safely Remove Hardware"
     - Linux: ``umount /media/$USER/NO\ NAME`` or right-click → Unmount

  2. Try clicking **Connect** via the **front JTAG port**.
  3. If the serial port still doesn't appear: briefly **unplug and replug the
     front USB cable**, then try again.

**Browser crashed or tab closed during SD card step**

  If the browser crashes or you close the tab while the device is in MSC mode:

  - **Don't panic** — the device is fine and will stay in MSC mode.
  - You can still write files to the SD card drive manually (drag and drop).
  - To switch back to normal firmware: reload the flash page, click the
    **"Skip step 1"** link, and proceed from Step 3 onward. This writes the
    OTA data to select the normal firmware without re-flashing tusb_msc.bin.
  - Alternatively, **power-cycle** the device (unplug both cables, wait 3 s,
    replug) to force a normal boot.

**Device stuck in MSC mode (won't boot normal firmware)**

  If the device keeps booting into MSC mode instead of the normal firmware:

  1. Open the flash page and use the **"Skip step 1"** link.
  2. Connect via the front JTAG port and proceed — the flash page will write
     OTA data to select the normal firmware partition.
  3. If that doesn't work: power-cycle by unplugging all cables, wait 3 seconds,
     replug only **back Port #1**, and try again.

RP2350 Pico (Step 5 / Path A Step 2)
-------------------------------------

**"Picoboot not loaded"**

  Your browser may not support **WebUSB**, or the picoflash module failed to load.
  Use Chrome, Edge, or Opera. Hard-refresh the page (⌘-Shift-R / Ctrl-Shift-R)
  and check the browser console for errors.

**RP2350 doesn't appear in device picker**

  Make sure the RP2350 is in **BOOTSEL mode**: hold the BOOTSEL button, press RESET,
  then release BOOTSEL. The device should appear as "RP2350 Boot" in the picker.

**Linux: WebUSB device not found**

  Chrome on Linux needs udev rules to access USB devices. Create
  ``/etc/udev/rules.d/99-pico.rules`` with::

    SUBSYSTEM=="usb", ATTR{idVendor}=="2e8a", MODE="0666"

  Then reload rules::

    sudo udevadm control --reload-rules && sudo udevadm trigger

**Windows: device not recognised**

  If Chrome does not show the RP2350 in the WebUSB device picker, you may need to
  install the **WinUSB** driver using `Zadig <https://zadig.akeo.ie/>`_. Select the
  RP2350 Boot device and install WinUSB.

After Flashing
--------------

**Device not reachable at http://192.168.4.1**

  - **Remove all USB cables**, wait 3 seconds, then reconnect via **back Port #1** only
  - Wait **25–30 seconds** for the device to boot and start the WiFi access point
  - Connect your computer to the ``TBD-16`` WiFi network
  - If still not reachable: check that the SD card has the correct files (try Path B)
  - Try a hard refresh in your browser (⌘-Shift-R / Ctrl-Shift-R)

**WebUI looks broken or shows old version**

  After a firmware update, do a hard refresh in your browser (⌘-Shift-R / Ctrl-Shift-R).
  If that doesn't help, update the WebUI from the device's settings page, or use Path B
  to write a fresh SD card image.

.. include:: /_includes/footer-links.rst
