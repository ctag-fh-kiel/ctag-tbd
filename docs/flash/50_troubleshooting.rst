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

**SD card doesn't mount after Step 1**

  The most common cause is that the device didn't actually reboot after flashing.
  The software reset via WebSerial can be unreliable — you may need a manual reset.

  1. **Press the RESET button** on the back of the device (between USB-C Port #1 and MIDI OUT 2)
  2. Wait 15 seconds for the SD card drive to appear
  3. If still nothing: **unplug both USB cables**, wait 3 seconds, replug them

  Also check:

  - Make sure a USB-C cable is connected from **back Port #1** to your computer
  - Port #1 is the back USB-C port closest to the center of the device
  - Check your file manager for a new drive (typically named ``NO NAME``)
  - If still no drive: go back to Step 1 and try again

**"Permission denied" when writing files**

  When Chrome asks for permission to write to the selected directory, click **Allow**.
  If you accidentally denied, close the prompt and click "Select SD Card Drive" again.

**Step 3 connection fails (device in MSC mode)**

  When the device is in USB Mass Storage mode, the serial port may not be available.
  Make sure you **eject the drive first** (macOS: right-click → Eject; Windows:
  "Safely Remove Hardware"; Linux: unmount), then try connecting via the
  **front JTAG port**. If the serial port still doesn't appear, briefly unplug
  and replug the front USB cable.

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
  - Wait 15–30 seconds for the device to boot and start the WiFi access point
  - Connect your computer to the ``TBD-16`` WiFi network
  - If still not reachable: check that the SD card has the correct files (try Path B)

**WebUI looks broken or shows old version**

  After a firmware update, do a hard refresh in your browser (⌘-Shift-R / Ctrl-Shift-R).
  If that doesn't help, update the WebUI from the device's settings page, or use Path B
  to write a fresh SD card image.

.. include:: /_includes/footer-links.rst
